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



#include "include/cef_app.h"

#include "simple_app.h"
#include "simple_handler.h"

namespace {
// Statically initialize the world class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::web::Service*		w = new ds::web::Service(e);
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
	, mService(engine.getService<ds::web::Service>("cef_web"))
	, mDragScrolling(false)
	, mDragScrollMinFingers(2)
	, mClickDown(false)
	, mPageScrollCount(0)
	, mDocumentReadyFn(nullptr)
	, mHasError(false)
	, mAllowClicks(true)
{
	// Should be unnecessary, but really want to make sure that static gets initialized
	INIT.doNothing();

	mBlobType = BLOB_TYPE;
	mLayoutFixedAspect = true;

	setTransparent(false);
	setColor(1.0f, 1.0f, 1.0f);
	setUseShaderTextuer(true);

	// GN: Someone decided this should be invisible by default.
	//		I'm deciding that that's ree-dic-u-lous
	//hide();
	//setOpacity(0.0f);

	setProcessTouchCallback([this](ds::ui::Sprite *, const ds::ui::TouchInfo &info) {
		handleTouch(info);
	});


	mService.createBrowser("https://google.com");

	
}

Web::~Web() {
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

void Web::drawLocalClient() {
	if (mWebTexture) {
		//ci::gl::color(ci::Color::white());

		if(getPerspective()){
			ci::gl::draw(mWebTexture, ci::Rectf(0.0f, static_cast<float>(mWebTexture.getHeight()), static_cast<float>(mWebTexture.getWidth()), 0.0f));
		} else {
			ci::gl::draw(mWebTexture);
		}
	}
}

void Web::handleTouch(const ds::ui::TouchInfo& touchInfo) {
	if (touchInfo.mFingerIndex != 0)
		return;

	ci::Vec2f pos = globalToLocal(touchInfo.mCurrentGlobalPoint).xy();

	if (ds::ui::TouchInfo::Added == touchInfo.mPhase) {
		ci::app::MouseEvent event(mEngine.getWindow(), ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
		sendMouseDownEvent(event);
		if(mDragScrolling){
			mClickDown = true;
		}
	} else if (ds::ui::TouchInfo::Moved == touchInfo.mPhase) {
		if(mDragScrolling && touchInfo.mNumberFingers >= mDragScrollMinFingers){
			/*
			if(mWebViewPtr){
				if(mClickDown){
					ci::app::MouseEvent uevent(mEngine.getWindow(), ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
					sendMouseUpEvent(uevent);
					mClickDown = false;
				}
				float yDelta = touchInfo.mCurrentGlobalPoint.y- mPreviousTouchPos.y;
				ci::app::MouseEvent event(mEngine.getWindow(), 0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, yDelta, 1);
				ds::web::handleMouseWheel( mWebViewPtr, event, 1 );
			}
			*/
		} else {
			ci::app::MouseEvent event(mEngine.getWindow(), 0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
			sendMouseDragEvent(event);
		}
	} else if (ds::ui::TouchInfo::Removed == touchInfo.mPhase) {
		mClickDown = false;
		ci::app::MouseEvent event(mEngine.getWindow(), ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
		sendMouseUpEvent(event);
	}

	mPreviousTouchPos = touchInfo.mCurrentGlobalPoint;
}

void Web::loadUrl(const std::wstring &url) {
	try {
		loadUrl(ds::utf8_from_wstr(url));
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

void Web::loadUrl(const std::string &url) {
	try {
		/*
		if (mWebViewPtr) {
			DS_LOG_INFO("Web::loadUrl() on " << url);
			mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(url.c_str())));
			mWebViewPtr->Focus();
			onUrlSet(url);
		}
		*/
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

std::string Web::getUrl() {
	/*
	try {
		if (!mWebViewPtr) return "";
		Awesomium::WebURL		wurl = mWebViewPtr->url();
		Awesomium::WebString	webstr = wurl.spec();
		auto					len = webstr.ToUTF8(nullptr, 0);
		if (len < 1) return "";
		std::string				str(len+2, 0);
		webstr.ToUTF8(const_cast<char*>(str.c_str()), len);
		return str;
	} catch (std::exception const&) {
	}
	*/
	return "";
}

void Web::setUrl(const std::string& url) {
	try {
		setUrlOrThrow(url);
	} catch (std::exception const&) {
	}
}

namespace {
bool validateUrl(const std::string& url) {
	try {
		std::string ext = boost::filesystem::path(url).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if(ext == ".pdf") return false;
	}
	catch(std::exception const&) {
	}
	return true;
}

std::string cleanupUrl(const std::string& url) {
	std::string output(url);
	try {
		std::regex pattern("((http[s]?):\\/\\/)?(.*)");
		std::smatch matches;
		if(std::regex_match(url, matches, pattern))
		{
			int count = matches.size();
			std::string protocol = matches.str(2);
			std::string theRemainder = matches.str(3);
			if(protocol == "")
			{
				output = "http://";
				output.append(theRemainder);
			}
		}
	}
	catch(std::exception const&) {
	}
	return output;
}
}

void Web::setUrlOrThrow(const std::string& url) {
	DS_LOG_INFO("Web::setUrlOrThrow() on " << url);
	// Some simple validation, because clients have a tendency to put PDFs where
	// web pages should go.
	if (!validateUrl(url)) throw std::runtime_error("URL is not the correct format (" + url + ").");

	std::string cleanUrl = cleanupUrl(url);

	/*
	try {
		if (mWebViewPtr) {
			mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(cleanUrl.c_str())));
			mWebViewPtr->Focus();
			activate();
			onUrlSet(cleanUrl);
		}
	} catch (std::exception const&) {
		throw std::runtime_error("Web service is not available.");
	}
	*/
}

void Web::sendKeyDownEvent(const ci::app::KeyEvent &event) {
	/*
	if(mWebViewPtr){
		ds::web::handleKeyDown(mWebViewPtr, event);
	}
	*/
}

void Web::sendKeyUpEvent(const ci::app::KeyEvent &event){
	/*
	if(mWebViewPtr){
		ds::web::handleKeyUp(mWebViewPtr, event);
	}
	*/
}

void Web::sendMouseDownEvent(const ci::app::MouseEvent& e) {
	/*
	if (!mWebViewPtr || !mAllowClicks) return;

	ci::app::MouseEvent eventMove(mEngine.getWindow(), 0, e.getX(), e.getY(), 0, 0, 0);
	ds::web::handleMouseMove( mWebViewPtr, eventMove );
	ds::web::handleMouseDown( mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kAdded);
	*/
}

void Web::sendMouseDragEvent(const ci::app::MouseEvent& e) {
	/*
	if(!mWebViewPtr || !mAllowClicks) return;

	ds::web::handleMouseDrag(mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kMoved);
	*/
}

void Web::sendMouseUpEvent(const ci::app::MouseEvent& e) {
	/*
	if(!mWebViewPtr || !mAllowClicks) return;

	ds::web::handleMouseUp( mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kRemoved);
	*/
}

void Web::sendMouseClick(const ci::Vec3f& globalClickPoint){
	/*
	ci::Vec2f pos = globalToLocal(globalClickPoint).xy();

	ci::app::MouseEvent event(mEngine.getWindow(), ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
	sendMouseDownEvent(event);

	ci::app::MouseEvent eventD(mEngine.getWindow(), 0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
	sendMouseDragEvent(eventD);

	ci::app::MouseEvent eventU(mEngine.getWindow(), ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
	sendMouseUpEvent(eventU);
	*/
}


void Web::setZoom(const double v) {
//	if (!mWebViewPtr) return;
//	mWebViewPtr->SetZoom(static_cast<int>(v * 100.0));
}

double Web::getZoom() const {
//	if (!mWebViewPtr) return 1.0;
//	return static_cast<double>(mWebViewPtr->GetZoom()) / 100.0;
	return 1.0;
}

void Web::goBack() {
	/*
	if (mWebViewPtr) {
		mWebViewPtr->Focus();
		mWebViewPtr->GoBack();
	}
	*/
}

void Web::goForward() {
	/*
	if (mWebViewPtr) {
		mWebViewPtr->GoForward();
	}
	*/
}

void Web::reload() {
	/*
	if (mWebViewPtr) {
		mWebViewPtr->Reload(true);
	}
	*/
}

void Web::stop() {
	/*
	if (mWebViewPtr) {
		mWebViewPtr->Stop();
	}
	*/
}

bool Web::canGoBack() {
	/*
	if (!mWebViewPtr) return false;
	return mWebViewPtr->CanGoBack();
	*/
	return true;
}

bool Web::canGoForward() {
	/*
	if (!mWebViewPtr) return false;
	return mWebViewPtr->CanGoForward();
	*/
	return true;
}

void Web::setAddressChangedFn(const std::function<void(const std::string& new_address)>& fn) {
//	if (mWebViewListener) mWebViewListener->setAddressChangedFn(fn);
}

void Web::setDocumentReadyFn(const std::function<void(void)>& fn) {
	mDocumentReadyFn = fn;
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
	/*
	if (!mWebViewPtr) return ci::Vec2f(0.0f, 0.0f);
	return get_document_size(*mWebViewPtr);
	*/
	return ci::Vec2f(1024.0f, 768.0f);
}

ci::Vec2f Web::getDocumentScroll() {
	/*
	if (!mWebViewPtr) return ci::Vec2f(0.0f, 0.0f);
	return get_document_scroll(*mWebViewPtr);
	*/
	return ci::Vec2f::zero();
}

void Web::executeJavascript(const std::string& theScript){
	/*

	Awesomium::WebString		object_ws(Awesomium::WebString::CreateFromUTF8(theScript.c_str(), theScript.size()));
	Awesomium::JSValue			object = mWebViewPtr->ExecuteJavascriptWithResult(object_ws, Awesomium::WebString());
	std::cout << "Object return: " << ds::web::str_from_webstr(object.ToString()) << std::endl;
	*/
}


void Web::onSizeChanged() {
	/*
	if (mWebViewPtr) {
		const int			w = static_cast<int>(getWidth()),
							h = static_cast<int>(getHeight());
		if (w < 1 || h < 1) return;
		mWebViewPtr->Resize(w, h);
	}
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

bool Web::isLoading() {
	/*
	if(mWebViewPtr && mWebViewPtr->IsLoading()){
		return true;
	}
	*/
	return false;
}

bool Web::webViewDirty(){
	/*
	if(!mWebViewPtr){
		return false;
	}

	Awesomium::BitmapSurface* surface = (Awesomium::BitmapSurface*) mWebViewPtr->surface();
	if(!surface) return false; 

	return surface->is_dirty();
	*/
	return false;
}

void Web::update(const ds::UpdateParams &p) {

	// create or update our OpenGL Texture from the webview
}

void Web::onUrlSet(const std::string &url) {
	mUrl = url;
	markAsDirty(URL_DIRTY);
}

void Web::onDocumentReady() {
	/*
	if (!mWebViewPtr || !mJsMethodHandler) return;

	mJsMethodHandler->setDomIsReady(*mWebViewPtr);
	if (mDocumentReadyFn) mDocumentReadyFn();
	*/
}

void Web::setAllowClicks(const bool doAllowClicks){
	mAllowClicks = doAllowClicks;
}

void Web::setWebTransparent(const bool isTransparent){
	/*
	if(mWebViewPtr){
		mWebViewPtr->SetTransparent(isTransparent);
	}
	*/
}

} // namespace ui
} // namespace ds
