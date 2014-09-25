#include "web.h"

#include <algorithm>
#include <cinder/ImageIo.h>
#include <boost/filesystem.hpp>
#include <ds/app/app.h>
#include <ds/app/blob_reader.h>
#include <ds/app/environment.h>
#include <ds/data/data_buffer.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>
#include <ds/util/string_util.h>
#include "paulhoux-Cinder-Awesomium/include/CinderAwesomium.h"
#include "private/js_method_handler.h"
#include "private/script_translator.h"
#include "private/web_service.h"
#include "private/web_view_listener.h"

namespace {
// Statically initialize the world class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::web::Service*		w = new ds::web::Service(e);
			if (!w) throw std::runtime_error("Can't create ds::web::Service");
			e.addService("web", *w);

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
namespace {
const ds::web::ScriptTree	EMPTY_SCRIPT_TREE;

// Utility to get x,y data as a result of running some javascript. It's assumed that
// the javascript is returning an array value with two named keys that match prop_x and prop_y.
ci::Vec2f get_javascript_xy(Awesomium::WebView& view, const std::string& prop_x, const std::string& prop_y, const std::string& javascript) {
	const std::string		utf8_width(prop_x);
	const std::string		utf8_height(prop_y);
	Awesomium::WebString	width_key(Awesomium::WebString::CreateFromUTF8(utf8_width.c_str(), utf8_width.size()));
	Awesomium::WebString	height_key(Awesomium::WebString::CreateFromUTF8(utf8_height.c_str(), utf8_height.size()));
	Awesomium::WebString	str(Awesomium::WebString::CreateFromUTF8(javascript.c_str(), javascript.size()));
	Awesomium::JSValue		jsv = view.ExecuteJavascriptWithResult(str, Awesomium::WebString());
	const size_t			MAX_BUF = 256;
	char					buf[MAX_BUF];
	ci::Vec2f				ans(0.0f, 0.0f);
	if (jsv.IsObject()) {
		const Awesomium::JSObject&	jobj(jsv.ToObject());
		if (jobj.HasProperty(width_key) && jobj.HasProperty(height_key)) {
			Awesomium::JSValue		w = jobj.GetProperty(width_key),
									h = jobj.GetProperty(height_key);

			Awesomium::WebString	wweb(w.ToString());
			int						size = wweb.ToUTF8(buf, MAX_BUF);
			const std::string		wstr(buf, size);
			Awesomium::WebString	hweb(h.ToString());
			size = hweb.ToUTF8(buf, MAX_BUF);
			const std::string		hstr(buf, size);

			string_to_value(wstr, ans.x);
			string_to_value(hstr, ans.y);
		}
#if 0
		Awesomium::JSArray			names(jobj.GetPropertyNames());
		for (unsigned int k=0; k<names.size(); ++k) {
			Awesomium::JSValue&		jname(names.At(k));
			Awesomium::WebString	wname(jname.ToString());
			int						size = wname.ToUTF8(buf, 4000);
			const std::string		jeez(buf, size);
			std::cout << "\t" << k << "=" << jeez << std::endl;
		}
#endif
	}
	return ans;
}

// Get the current document scroll position
ci::Vec2f get_document_scroll(Awesomium::WebView& view) {
	const std::string		prop_x("x");
	const std::string		prop_y("y");
	const std::string		javascript("(function() { \
		var doc = document.documentElement; \
		var left = (window.pageXOffset || doc.scrollLeft) - (doc.clientLeft || 0); \
		var top = (window.pageYOffset || doc.scrollTop)  - (doc.clientTop || 0); \
		return {x:left, y:top}; }) ();");
	return get_javascript_xy(view, prop_x, prop_y, javascript);
}

ci::Vec2f get_document_size(Awesomium::WebView& view) {
	const std::string		prop_x("width");
	const std::string		prop_y("height");
	const std::string		javascript("(function() { var result = {height:$(document).height(), width:$(document).width()}; return result; }) ();");
	return get_javascript_xy(view, prop_x, prop_y, javascript);
}

} // anonymous namespace

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
	, mService(engine.getService<ds::web::Service>("web"))
	, mWebViewPtr(nullptr)
	, mLoadingAngle(0.0f)
	, mActive(false)
	, mTransitionTime(0.35f)
	, mDrawWhileLoading(false)
	, mDragScrolling(false)
	, mDragScrollMinFingers(2)
	, mClickDown(false)
	, mPageScrollCount(0)
	, mDocumentReadyFn(nullptr)
{
	// Should be unnecessary, but really want to make sure that static gets initialized
	INIT.doNothing();

	mBlobType = BLOB_TYPE;

	setTransparent(false);
	setColor(1.0f, 1.0f, 1.0f);
	setUseShaderTextuer(true);
	hide();
	setOpacity(0.0f);
	setProcessTouchCallback([this](ds::ui::Sprite *, const ds::ui::TouchInfo &info) {
		handleTouch(info);
	});

	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;

	// create a webview
	Awesomium::WebCore*	webcore = mService.getWebCore();
	if (webcore) {
		mWebViewPtr = webcore->CreateWebView(static_cast<int>(getWidth()), static_cast<int>(getHeight()), mService.getWebSession());
		if (mWebViewPtr) {
			mWebViewListener = std::move(std::unique_ptr<ds::web::WebViewListener>(new ds::web::WebViewListener));
			if (mWebViewListener) mWebViewPtr->set_view_listener(mWebViewListener.get());
			mWebLoadListener = std::move(std::unique_ptr<ds::web::WebLoadListener>(new ds::web::WebLoadListener));
			if (mWebLoadListener) {
				mWebViewPtr->set_load_listener(mWebLoadListener.get());
				mWebLoadListener->setOnDocumentReady([this](const std::string& url) { onDocumentReady(); });
			}
			mJsMethodHandler = std::move(std::unique_ptr<ds::web::JsMethodHandler>(new ds::web::JsMethodHandler));
			if (mJsMethodHandler) mWebViewPtr->set_js_method_handler(mJsMethodHandler.get());
		}
	}
	//mWebViewPtr->LoadURL( Awesomium::WebURL( Awesomium::WSLit( "http://libcinder.org" ) ) );
	//mWebViewPtr->Focus();

	// load and create a "loading" icon
	try {
		mLoadingTexture = ci::gl::Texture(ci::loadImage(ds::Environment::getAppFolder("data/images", "loading.png")));
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception loading loading image for websprite: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

Web::~Web() {
	if (mWebViewPtr) {
		mWebViewPtr->set_view_listener(nullptr);
		mWebViewPtr->Destroy();
	}
}

void Web::updateServer( const ds::UpdateParams &updateParams ) {
	Sprite::updateServer(updateParams);

	mPageScrollCount = 0;

	// create or update our OpenGL Texture from the webview
	if (mWebViewPtr
		&& (mDrawWhileLoading || !mWebViewPtr->IsLoading())
		&& ph::awesomium::isDirty( mWebViewPtr )) {
		try {
			// set texture filter to NEAREST if you don't intend to transform (scale, rotate) it
			ci::gl::Texture::Format fmt;
		//	fmt.setMagFilter( GL_NEAREST );
			fmt.setMagFilter( GL_LINEAR );

			// get the texture using a handy conversion function
			mWebTexture = ph::awesomium::toTexture( mWebViewPtr, fmt );
		} catch( const std::exception &e ) {
			DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
		}
	}

	mLoadingAngle += updateParams.getDeltaTime() * 60.0f * 5.0f;
	if (mLoadingAngle >= 360.0f)
		mLoadingAngle = mLoadingAngle - 360.0f;
}

void Web::drawLocalClient() {
  if (mWebTexture) {
    //ci::gl::color(ci::Color::white());
    ci::gl::draw(mWebTexture);
  }

  // show spinner while loading
  if (mLoadingTexture && mWebViewPtr && mWebViewPtr->IsLoading()) {
    ci::gl::pushModelView();

    ci::gl::translate(0.5f * ci::Vec2f(getWidth(), getHeight()));
    ci::gl::scale(0.5f, 0.5f );
    ci::gl::rotate(mLoadingAngle);
    ci::gl::translate(-0.5f * ci::Vec2f(mLoadingTexture.getSize()));

    //ci::gl::color(ci::Color::white());
    //ci::gl::enableAlphaBlending();
    ci::gl::draw(mLoadingTexture);
    //ci::gl::disableAlphaBlending();

    ci::gl::popModelView();
  }
}

void Web::handleTouch(const ds::ui::TouchInfo& touchInfo) {
	if (touchInfo.mFingerIndex != 0)
		return;

	ci::Vec2f pos = globalToLocal(touchInfo.mCurrentGlobalPoint).xy();

	if (ds::ui::TouchInfo::Added == touchInfo.mPhase) {
		ci::app::MouseEvent event(ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
		sendMouseDownEvent(event);
		if(mDragScrolling){
			mClickDown = true;
		}
	} else if (ds::ui::TouchInfo::Moved == touchInfo.mPhase) {
		if(mDragScrolling && touchInfo.mNumberFingers >= mDragScrollMinFingers){
			if(mWebViewPtr){
				if(mClickDown){
					ci::app::MouseEvent uevent(ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
					sendMouseUpEvent(uevent);
					mClickDown = false;
				}
				float yDelta = touchInfo.mCurrentGlobalPoint.y- mPreviousTouchPos.y;
				ci::app::MouseEvent event(0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, yDelta, 1);
				ph::awesomium::handleMouseWheel( mWebViewPtr, event, 1 );
			}
		} else {
			ci::app::MouseEvent event(0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
			sendMouseDragEvent(event);
		}
	} else if (ds::ui::TouchInfo::Removed == touchInfo.mPhase) {
		mClickDown = false;
		ci::app::MouseEvent event(ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
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
		if (mWebViewPtr) {
			DS_LOG_INFO("Web::loadUrl() on " << url);
			mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(url.c_str())));
			mWebViewPtr->Focus();
			onUrlSet(url);
		}
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

std::string Web::getUrl() {
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
	return "";
}

void Web::setUrl(const std::string& url) {
	try {
		setUrlOrThrow(url);
	} catch (std::exception const&) {
	}
}

namespace {
bool			validate_url(const std::string& url) {
	try {
		std::string ext = boost::filesystem3::path(url).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext == ".pdf") return false;
	} catch (std::exception const&) {
	}
	return true;
}
}

void Web::setUrlOrThrow(const std::string& url) {
	DS_LOG_INFO("Web::setUrlOrThrow() on " << url);
	// Some simple validation, because clients have a tendency to put PDFs where
	// web pages should go.
	if (!validate_url(url)) throw std::runtime_error("URL is not the correct format (" + url + ").");

	try {
		if (mWebViewPtr) {
			mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(url.c_str())));
			mWebViewPtr->Focus();
			activate();
			onUrlSet(url);
			return;
		}
	} catch (std::exception const&) {
	}
	throw std::runtime_error("Web service is not available.");
}

void Web::sendKeyDownEvent( const ci::app::KeyEvent &event ) {
	// untested!
	if(mWebViewPtr){
		ph::awesomium::handleKeyDown(mWebViewPtr, event);
	}
}

void Web::sendKeyUpEvent( const ci::app::KeyEvent &event ){
	// untested!
	if(mWebViewPtr){
		ph::awesomium::handleKeyUp(mWebViewPtr, event);
	}
}

void Web::sendMouseDownEvent(const ci::app::MouseEvent& e) {
	if (!mWebViewPtr) return;

	ci::app::MouseEvent eventMove(0, e.getX(), e.getY(), 0, 0, 0);
	ph::awesomium::handleMouseMove( mWebViewPtr, eventMove );
	ph::awesomium::handleMouseDown( mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kAdded);
}

void Web::sendMouseDragEvent(const ci::app::MouseEvent& e) {
	if (!mWebViewPtr) return;

	ph::awesomium::handleMouseDrag(mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kMoved);
}

void Web::sendMouseUpEvent(const ci::app::MouseEvent& e) {
	if (!mWebViewPtr) return;

	ph::awesomium::handleMouseUp( mWebViewPtr, e);
	sendTouchEvent(e.getX(), e.getY(), ds::web::TouchEvent::kRemoved);
}

void Web::setTouchListener(const std::function<void(const ds::web::TouchEvent&)>& fn) {
	mTouchListener = fn;
}

void Web::handleListenerTouchEvent(const ds::web::TouchEvent& te) {
	if (!mWebViewPtr) return;

//	const ci::Vec2f			doc_size(getDocumentSize());
	const ci::Vec2f			doc_scroll(getDocumentScroll());
//	const ci::Vec2i			pos(static_cast<int>((te.mUnitPosition.x*doc_size.x)-doc_scroll.x),
//								static_cast<int>((te.mUnitPosition.y*doc_size.y)-doc_scroll.y));
	const ci::Vec2i			pos(static_cast<int>((te.mPosition.x)-doc_scroll.x),
								static_cast<int>((te.mPosition.y)-doc_scroll.y));
	if (te.mPhase == te.kAdded) {
		sendMouseDownEvent(ci::app::MouseEvent(1, pos.x, pos.y, 0, 0, 0));
	} else if (te.mPhase == te.kMoved) {
		sendMouseDragEvent(ci::app::MouseEvent(1, pos.x, pos.y, 0, 0, 0));
	} else if (te.mPhase == te.kRemoved) {
		sendMouseUpEvent(ci::app::MouseEvent(1, pos.x, pos.y, 0, 0, 0));
	}
}

void Web::activate() {
	if (mActive) {
		return;
	}

	animStop();
	show();
	enable(true);
	mActive = true;
	if (mWebViewPtr) {
		mWebViewPtr->ResumeRendering();
	}
	mEngine.getTweenline().apply(*this, ANIM_OPACITY(), 1.0f, mTransitionTime, ci::EaseOutQuad());
}

void Web::deactivate() {
	if (!mActive) {
		return;
	}

	animStop();
	mEngine.getTweenline().apply(*this, ANIM_OPACITY(), 0.0f, mTransitionTime, ci::EaseOutQuad(), [this]()
	{
		hide();
		enable(false);
		mActive = false;
		if (mWebViewPtr) mWebViewPtr->PauseRendering();
		this->loadUrl(ds::Environment::getAppFolder("data", "index.html"));
	});
}

bool Web::isActive() const {
	return mActive;
}

void Web::setTransitionTime( const float transitionTime ) {
	mTransitionTime = transitionTime;
}

void Web::setZoom(const double v) {
	if (!mWebViewPtr) return;
	mWebViewPtr->SetZoom(static_cast<int>(v * 100.0));
}

double Web::getZoom() const {
	if (!mWebViewPtr) return 1.0;
	return static_cast<double>(mWebViewPtr->GetZoom()) / 100.0;
}

void Web::goBack() {
	if (mWebViewPtr) {
		mWebViewPtr->GoBack();
	}
}

void Web::goForward() {
	if (mWebViewPtr) {
		mWebViewPtr->GoForward();
	}
}

void Web::reload() {
	if (mWebViewPtr) {
		mWebViewPtr->Reload(true);
	}
}

bool Web::canGoBack() {
	if (!mWebViewPtr) return false;
	return mWebViewPtr->CanGoBack();
}

bool Web::canGoForward() {
	if (!mWebViewPtr) return false;
	return mWebViewPtr->CanGoForward();
}

void Web::setAddressChangedFn(const std::function<void(const std::string& new_address)>& fn) {
	if (mWebViewListener) mWebViewListener->setAddressChangedFn(fn);
}

void Web::setDocumentReadyFn(const std::function<void(void)>& fn) {
	mDocumentReadyFn = fn;
}

ci::Vec2f Web::getDocumentSize() {
	if (!mWebViewPtr) return ci::Vec2f(0.0f, 0.0f);
	return get_document_size(*mWebViewPtr);
}

ci::Vec2f Web::getDocumentScroll() {
	if (!mWebViewPtr) return ci::Vec2f(0.0f, 0.0f);
	return get_document_scroll(*mWebViewPtr);
}

ds::web::ScriptTree Web::runJavaScript(	const std::string& object_utf8, const std::string& function_utf8,
										const ds::web::ScriptTree& args) {
	if (!mWebViewPtr) return ds::web::ScriptTree();

	Awesomium::WebString		object_ws(Awesomium::WebString::CreateFromUTF8(object_utf8.c_str(), object_utf8.size()));
	Awesomium::JSValue			object = mWebViewPtr->ExecuteJavascriptWithResult(object_ws, Awesomium::WebString());
	if (object.IsObject()) {
		Awesomium::WebString	function_ws(Awesomium::WebString::CreateFromUTF8(function_utf8.c_str(), function_utf8.size()));
		Awesomium::JSArray		args(jsarray_from_tree(args));
		Awesomium::JSValue		ans = object.ToObject().Invoke(function_ws, args);
		return ds::web::tree_from_jsvalue(ans);
	}
	return ds::web::ScriptTree();
}

void Web::registerJavaScriptMethod(	const std::string& class_name, const std::string& method_name,
									const std::function<void(const ds::web::ScriptTree&)>& fn) {
	if (!mWebViewPtr || !mJsMethodHandler) return;

	mJsMethodHandler->registerMethod(*mWebViewPtr, class_name, method_name, fn);
}

void Web::onSizeChanged() {
	if (mWebViewPtr) {
		const int			w = static_cast<int>(getWidth()),
							h = static_cast<int>(getHeight());
		if (w < 1 || h < 1) return;
		mWebViewPtr->Resize(w, h);
	}
}

void Web::writeAttributesTo(ds::DataBuffer &buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(URL_DIRTY)) {
		buf.add(URL_ATT);
		buf.add(mUrl);
	}
}

void Web::readAttributeFrom(const char attributeId, ds::DataBuffer &buf) {
	if (attributeId == URL_ATT) {
		setUrl(buf.read<std::string>());
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

bool Web::isLoading() {
	if(mWebViewPtr && mWebViewPtr->IsLoading()){
		return true;
	}
	return false;
}

void Web::onUrlSet(const std::string &url) {
	mUrl = url;
	markAsDirty(URL_DIRTY);
}

void Web::onDocumentReady() {
	if (!mWebViewPtr || !mJsMethodHandler) return;

	mJsMethodHandler->setDomIsReady(*mWebViewPtr);
	if (mDocumentReadyFn) mDocumentReadyFn();
}

void Web::sendTouchEvent(const int x, const int y, const ds::web::TouchEvent::Phase& phase) {
	if (!mWebViewPtr || !mTouchListener) return;

	if (phase == ds::web::TouchEvent::kAdded) {
		mPageSizeCache = getDocumentSize();
	}
	if (mPageScrollCount <= 0) {
		mPageScrollCache = getDocumentScroll();
	}
	++mPageScrollCount;

	ds::web::TouchEvent		te;
	te.mPhase = phase;
	te.mPosition.x = (mPageScrollCache.x + static_cast<float>(x));
	te.mPosition.y = (mPageScrollCache.y + static_cast<float>(y));
	te.mPosition.x = (static_cast<float>(x));
	te.mPosition.y = (static_cast<float>(y));
	te.mUnitPosition.x = te.mPosition.x / mPageSizeCache.x;
	te.mUnitPosition.y = te.mPosition.y / mPageSizeCache.y;

	mTouchListener(te);
}

} // namespace ui
} // namespace ds