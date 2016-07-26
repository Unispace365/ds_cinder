#include "web.h"

#include <algorithm>
#include <regex>
#include <cinder/ImageIo.h>
#include <boost/filesystem.hpp>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_reader.h>
#include <ds/app/environment.h>
#include <ds/data/data_buffer.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>
#include <ds/util/string_util.h>
#include "private/web_service.h"
#include "private/web_callbacks.h"


#include "include/cef_app.h"

namespace {
// Statically initialize the world class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::web::WebCefService*		w = new ds::web::WebCefService(e);
			if (!w) throw std::runtime_error("Can't create ds::web::Service");
			e.addService("cef_web", *w);

			e.installSprite([](ds::BlobRegistry& r){ds::ui::Web::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Web::installAsClient(r);});
		});
	}
	void					doNothing() { }
};
Init						INIT;

char						BLOB_TYPE			= 0;
const ds::ui::DirtyState&	URL_DIRTY			= ds::ui::INTERNAL_A_DIRTY;
const ds::ui::DirtyState&	PDF_PAGEMODE_DIRTY	= ds::ui::INTERNAL_B_DIRTY;
const char					URL_ATT				= 80;
const char					PDF_PAGEMODE_ATT	= 81;
}

namespace ds {
namespace ui {

/**
 * \class ds::ui::sprite::Web static
 */
void Web::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Web::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Web>(r);});
}

/**
 * \class ds::ui::sprite::Web
 */
Web::Web( ds::ui::SpriteEngine &engine, float width, float height )
	: Sprite(engine, width, height)
	, mService(engine.getService<ds::web::WebCefService>("cef_web"))
	, mDragScrolling(false)
	, mDragScrollMinFingers(2)
	, mClickDown(false)
	, mPageScrollCount(0)
	, mDocumentReadyFn(nullptr)
	, mHasError(false)
	, mAllowClicks(true)
	, mBrowserId(-1)
	, mBuffer(nullptr)
	, mHasBuffer(false)
	, mBrowserSize(0, 0)
	, mUrl("")
	, mIsLoading(false)
	, mCanBack(false)
	, mCanForward(false)
	, mZoom(1.0)
	, mTransparentBackground(false)
{
	// Should be unnecessary, but really want to make sure that static gets initialized
	INIT.doNothing();

	mBlobType = BLOB_TYPE;
	mLayoutFixedAspect = true;

	setTransparent(false);
	setUseShaderTexture(true);
	setSize(width, height);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);

	setProcessTouchCallback([this](ds::ui::Sprite *, const ds::ui::TouchInfo &info) {
		handleTouch(info);
	});

	createBrowser();
}

void Web::createBrowser(){
	clearBrowser();

	mService.createBrowser("", this, [this](int browserId){
		{
			// This callback comes back from the CEF UI thread
			std::lock_guard<std::mutex> lock(mMutex);
			mBrowserId = browserId;
		}

		initializeBrowser();
	}, mTransparentBackground);
}

void Web::clearBrowser(){
	if(mBrowserId < 0){
		mService.cancelCreation(this);
	} else {
		// This clears the callbacks too
		mService.closeBrowser(mBrowserId);
	}

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mBrowserId = -1;
	}
}

Web::~Web() {

	clearBrowser();

	{
		std::lock_guard<std::mutex> lock(mMutex);
		if(mBuffer){
			delete mBuffer;
			mBuffer = nullptr;
		}
	}
}

void Web::setWebTransparent(const bool isTransparent){
	if(isTransparent == mTransparentBackground) return;
	mTransparentBackground = isTransparent;

	createBrowser();
}

void Web::initializeBrowser(){

	// Now that we know about the browser, set it to the correct size
	if(!mBuffer){
		onSizeChanged();
	} else {
		mService.requestBrowserResize(mBrowserId, mBrowserSize);
	}

	loadUrl(mUrl);
	if(mZoom != 1.0){
		setZoom(mZoom);
	}

	ds::web::WebCefCallbacks wcc;
	wcc.mTitleChangeCallback = [this](const std::wstring& newTitle){
		// This callback comes back from the CEF UI thread
		std::lock_guard<std::mutex> lock(mMutex);

		mTitle = newTitle;
		if(mTitleChangedCallback){
			mTitleChangedCallback(newTitle);
		}
	};

	wcc.mLoadChangeCallback = [this](const bool isLoading, const bool canBack, const bool canForwards, const std::string& newUrl){
		// This callback comes back from the CEF UI thread
		std::lock_guard<std::mutex> lock(mMutex);

		mIsLoading = isLoading;
		mCanBack = canBack;
		mCanForward = canForwards;
		mUrl = newUrl;

		// zoom seems to need to be set for every page
		if(mZoom != 1.0 && getZoom() != mZoom){
			setZoom(mZoom);
		}

		if(mLoadingUpdatedCallback){
			mLoadingUpdatedCallback(isLoading);
		}

		if(!mIsLoading && mDocumentReadyFn){
			mDocumentReadyFn();
			//	mJsMethodHandler->setDomIsReady(*mWebViewPtr);
		}
	};

	wcc.mPaintCallback = [this](const void * buffer, const int bufferWidth, const int bufferHeight){

		// This callback comes back from the CEF UI thread
		std::lock_guard<std::mutex> lock(mMutex);

		// verify the buffer exists and is the correct size
		// TODO: Add ability to redraw only the changed rectangles (which is what comes from CEF)
		// Would be much more performant, especially for large browsers with small ui changes (like blinking cursors)
		if(mBuffer && bufferWidth == mBrowserSize.x && bufferHeight == mBrowserSize.y){
			mHasBuffer = true;
			memcpy(mBuffer, buffer, bufferWidth * bufferHeight * 4);
		}
	};

	wcc.mErrorCallback = [this](const std::string& theError){
		// This callback comes back from the CEF UI thread
		std::lock_guard<std::mutex> lock(mMutex);

		setErrorMessage(theError);
	};

	wcc.mFullscreenCallback = [this](const bool isFullscreen){
		// This callback comes back from the CEF UI thread
		std::lock_guard<std::mutex> lock(mMutex);

		if(mFullscreenCallback){
			mFullscreenCallback(isFullscreen);
		}
	};

	mService.addWebCallbacks(mBrowserId, wcc);
}


void Web::updateClient(const ds::UpdateParams &p) {
	Sprite::updateClient(p);

	update(p);
}

void Web::updateServer(const ds::UpdateParams &p) {
	Sprite::updateServer(p);

	mPageScrollCount = 0;

	update(p);
}

void Web::update(const ds::UpdateParams &p) {

	// Anything that modifies mBuffer needs to be locked
	std::lock_guard<std::mutex> lock(mMutex);

	if(mBuffer && mHasBuffer){
		ci::gl::Texture::Format fmt;
		fmt.setMinFilter(GL_LINEAR);
		fmt.setMagFilter(GL_LINEAR);
		mWebTexture = ci::gl::Texture(mBuffer, GL_BGRA, mBrowserSize.x, mBrowserSize.y, fmt);
		mHasBuffer = false;
	}
}

void Web::onSizeChanged() {
	{
		// Anything that modifies mBuffer needs to be locked
		std::lock_guard<std::mutex> lock(mMutex);

		const int theWid = static_cast<int>(getWidth());
		const int theHid = static_cast<int>(getHeight());
		const ci::Vec2i newBrowserSize(theWid, theHid);
		if(newBrowserSize == mBrowserSize && mBuffer){
			return;
		}

		mBrowserSize = newBrowserSize;

		if(mBuffer){
			delete mBuffer;
			mBuffer = nullptr;
		}
		const int bufferSize = theWid * theHid * 4;
		mBuffer = new unsigned char[bufferSize];

		mHasBuffer = false;
	}

	if(mBrowserId > -1){
		mService.requestBrowserResize(mBrowserId, mBrowserSize);
	}
}

void Web::drawLocalClient() {
	if (mWebTexture) {
		if(getPerspective()){
			ci::gl::draw(mWebTexture, ci::Rectf(0.0f, static_cast<float>(mWebTexture.getHeight()), static_cast<float>(mWebTexture.getWidth()), 0.0f));
		} else {
			ci::gl::draw(mWebTexture);
		}
	}
}

std::string Web::getUrl() {
	return mUrl;
}

void Web::loadUrl(const std::wstring &url) {
	loadUrl(ds::utf8_from_wstr(url));
}

void Web::loadUrl(const std::string &url) {
	mUrl = url;
	markAsDirty(URL_DIRTY);
	if(mBrowserId > -1 && !mUrl.empty()){
		mService.loadUrl(mBrowserId, mUrl);
	}
}

void Web::setUrl(const std::string& url) {
	loadUrl(url);
}

void Web::setUrlOrThrow(const std::string& url) {
	loadUrl(url);
}

void Web::sendKeyDownEvent(const ci::app::KeyEvent &event) {
	mService.sendKeyEvent(mBrowserId, 0, event.getNativeKeyCode(), event.getChar(), event.isShiftDown(), event.isControlDown(), event.isAltDown());
}

void Web::sendKeyUpEvent(const ci::app::KeyEvent &event){
	mService.sendKeyEvent(mBrowserId, 2, event.getNativeKeyCode(), event.getChar(), event.isShiftDown(), event.isControlDown(), event.isAltDown());
}

void Web::sendMouseDownEvent(const ci::app::MouseEvent& e) {
	if(!mAllowClicks) return;

	ci::Vec2f pos = globalToLocal(ci::Vec3f((float)e.getX(), (float)e.getY(), 0.0f)).xy();
	mService.sendMouseClick(mBrowserId, e.getX(), e.getY(), 0, 0, 1);
}

void Web::sendMouseDragEvent(const ci::app::MouseEvent& e) {
	if(!mAllowClicks) return;

	ci::Vec2f pos = globalToLocal(ci::Vec3f((float)e.getX(), (float)e.getY(), 0.0f)).xy();
	mService.sendMouseClick(mBrowserId, e.getX(), e.getY(), 0, 1, 1);
}

void Web::sendMouseUpEvent(const ci::app::MouseEvent& e) {
	if(!mAllowClicks) return;

	mService.sendMouseClick(mBrowserId, e.getX(), e.getY(), 0, 2, 1);
}

void Web::sendMouseClick(const ci::Vec3f& globalClickPoint){
	if(!mAllowClicks) return;

	ci::Vec2f pos = globalToLocal(globalClickPoint).xy();
	int xPos = (int)roundf(pos.x);
	int yPos = (int)roundf(pos.y);

	mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 0, 1);
	mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 1, 1);
	mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 2, 1);
}

void Web::handleTouch(const ds::ui::TouchInfo& touchInfo) {
	if(touchInfo.mFingerIndex != 0)
		return;

	ci::Vec2f pos = globalToLocal(touchInfo.mCurrentGlobalPoint).xy();
	int xPos = (int)roundf(pos.x);
	int yPos = (int)roundf(pos.y);

	if(ds::ui::TouchInfo::Added == touchInfo.mPhase) {
		if(mAllowClicks) mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 0, 1);
		if(mDragScrolling){
			mClickDown = true;
		}
		
	} else if(ds::ui::TouchInfo::Moved == touchInfo.mPhase) {


		if(mDragScrolling && touchInfo.mNumberFingers >= mDragScrollMinFingers){
			
			if(mClickDown){
				if(mAllowClicks) mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 1, 0);
				if(mAllowClicks) mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 2, 0);
				mClickDown = false;
			}

			float yDelta = touchInfo.mCurrentGlobalPoint.y - mPreviousTouchPos.y;
			mService.sendMouseWheelEvent(mBrowserId, xPos, yPos, 0, (int)roundf(yDelta));			
			
		} else {
			if(mAllowClicks) mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 1, 1);
		}
	} else if(ds::ui::TouchInfo::Removed == touchInfo.mPhase) {
		if(mAllowClicks) mService.sendMouseClick(mBrowserId, xPos, yPos, 0, 2, 1);
	}

	mPreviousTouchPos = touchInfo.mCurrentGlobalPoint;
}

void Web::setZoom(const double percent) {
	mZoom = percent;
	mService.setZoomLevel(mBrowserId, (percent - 1.0) / .25);
}

double Web::getZoom() const {
	if(mBrowserId < 0) return mZoom;
	return (mService.getZoomLevel(mBrowserId)*.25 + 1.0);
}

void Web::goBack() {
	mService.goBackwards(mBrowserId);
}

void Web::goForward() {
	mService.goForwards(mBrowserId);
}

void Web::reload(const bool ignoreCache) {
	mService.reload(mBrowserId, ignoreCache);
}

void Web::stop() {
	mService.stopLoading(mBrowserId);
}

bool Web::canGoBack() {
	return mCanBack;
}

bool Web::canGoForward() {
	return mCanForward;
}

bool Web::isLoading() {
	return mIsLoading;
}

void Web::setTitleChangedFn(const std::function<void(const std::wstring& newTitle)>& func){
	mTitleChangedCallback = func;
}

void Web::setAddressChangedFn(const std::function<void(const std::string& new_address)>& fn) {
	mAddressChangedCallback = fn;
}

void Web::setDocumentReadyFn(const std::function<void(void)>& fn) {
	mDocumentReadyFn = fn;
}

void Web::setErrorCallback(std::function<void(const std::string&)> func){
	mErrorCallback = func;
}

void Web::setFullscreenChangedCallback(std::function<void(const bool)> func){
	mFullscreenCallback = func;
}

void Web::setErrorMessage(const std::string &message){
	mHasError = true;
	mErrorMessage = message;

	if(mErrorCallback){
		mErrorCallback(mErrorMessage);
	}
}

void Web::clearError(){
	mHasError = false;
}

ci::Vec2f Web::getDocumentSize() {
	// TODO?
	return ci::Vec2f(getWidth(), getHeight());
}

ci::Vec2f Web::getDocumentScroll() {
	/* TODO
	if (!mWebViewPtr) return ci::Vec2f(0.0f, 0.0f);
	return get_document_scroll(*mWebViewPtr);
	*/
	return ci::Vec2f::zero();
}

void Web::executeJavascript(const std::string& theScript){
	/* TODO
	Awesomium::WebString		object_ws(Awesomium::WebString::CreateFromUTF8(theScript.c_str(), theScript.size()));
	Awesomium::JSValue			object = mWebViewPtr->ExecuteJavascriptWithResult(object_ws, Awesomium::WebString());
	std::cout << "Object return: " << ds::web::str_from_webstr(object.ToString()) << std::endl;
	*/
}

void Web::writeAttributesTo(ds::DataBuffer &buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if (mDirty.has(URL_DIRTY)) {
		buf.add(URL_ATT);
		buf.add(mUrl);
	}
}

void Web::readAttributeFrom(const char attributeId, ds::DataBuffer &buf) {
	if (attributeId == URL_ATT) {
		setUrl(buf.read<std::string>());
	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

void Web::setAllowClicks(const bool doAllowClicks){
	mAllowClicks = doAllowClicks;
}

} // namespace ui
} // namespace ds
