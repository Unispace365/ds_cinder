#include "web.h"

#include <cinder/ImageIo.h>
#include <ds/app/app.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>
#include <ds/util/string_util.h>
#include "paulhoux-Cinder-Awesomium/include/CinderAwesomium.h"
#include "private/web_service.h"

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
		});

	}
	void			doNothing() { }
};
Init				INIT;
}

namespace ds {
namespace ui {

Web::Web( ds::ui::SpriteEngine &engine, float width, float height )
	: Sprite(engine, width, height)
	, mService(engine.getService<ds::web::Service>("web"))
	, mWebViewPtr(nullptr)
	, mLoadingAngle(0.0f)
	, mActive(false)
	, mTransitionTime(0.35f)
{
	// Should be unnecessary, but really want to make sure that static gets initialized
	INIT.doNothing();

	setTransparent(false);
	setColor(1.0f, 1.0f, 1.0f);
	setUseShaderTextuer(true);
	hide();
	setOpacity(0.0f);
	setProcessTouchCallback([this](ds::ui::Sprite *, const ds::ui::TouchInfo &info)
	{
		handleTouch(info);
	});

	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;

	// create a webview
	Awesomium::WebCore*	webcore = mService.getWebCore();
	if (webcore) {
		mWebViewPtr = webcore->CreateWebView(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
	}
	//mWebViewPtr->LoadURL( Awesomium::WebURL( Awesomium::WSLit( "http://libcinder.org" ) ) );
	//mWebViewPtr->Focus();

	// load and create a "loading" icon
	try {
		mLoadingTexture = ci::gl::Texture(ci::loadImage(ds::Environment::getAppFolder("data/images", "loading.png")));
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

Web::~Web() {
	if (mWebViewPtr) {
		mWebViewPtr->Destroy();
	}
}

void Web::onSizeChanged() {
	if (mWebViewPtr) {
		const int			w = static_cast<int>(getWidth()),
							h = static_cast<int>(getHeight());
		std::cout << "Web set size w=" << w << ", h=" << h << std::endl;
		if (w < 1 || h < 1) return;
		mWebViewPtr->Resize(w, h);
	}
}

void Web::updateServer( const ds::UpdateParams &updateParams )
{
  Sprite::updateServer(updateParams);

  // create or update our OpenGL Texture from the webview
  if (mWebViewPtr && !mWebViewPtr->IsLoading() && ph::awesomium::isDirty( mWebViewPtr )) {
    try {
      // set texture filter to NEAREST if you don't intend to transform (scale, rotate) it
      ci::gl::Texture::Format fmt; 
      fmt.setMagFilter( GL_NEAREST );

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

void Web::drawLocalClient()
{
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

void Web::handleTouch( const ds::ui::TouchInfo &touchInfo )
{
  if (touchInfo.mFingerIndex != 0)
    return;

  ci::Vec2f pos = globalToLocal(touchInfo.mCurrentGlobalPoint).xy();

  if (ds::ui::TouchInfo::Added == touchInfo.mPhase) {
    ci::app::MouseEvent event(ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
    sendMouseDownEvent(event);
  } else if (ds::ui::TouchInfo::Moved == touchInfo.mPhase) {
    ci::app::MouseEvent event(0, static_cast<int>(pos.x), static_cast<int>(pos.y), ci::app::MouseEvent::LEFT_DOWN, 0, 1);
    sendMouseDragEvent(event);
  } else if (ds::ui::TouchInfo::Removed == touchInfo.mPhase) {
    ci::app::MouseEvent event(ci::app::MouseEvent::LEFT_DOWN, static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 0, 0);
    sendMouseUpEvent(event);
  }
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
		}
	} catch( const std::exception &e ) {
		DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
	}
}

void Web::sendKeyDownEvent( const ci::app::KeyEvent &event )
{
}

void Web::sendKeyUpEvent( const ci::app::KeyEvent &event )
{
}

void Web::sendMouseDownEvent( const ci::app::MouseEvent &event )
{
	// send mouse events to Awesomium
	if (mWebViewPtr) {
		ci::app::MouseEvent eventMove(0, event.getX(), event.getY(), 0, 0, 0);
		ph::awesomium::handleMouseMove( mWebViewPtr, eventMove );
		ph::awesomium::handleMouseDown( mWebViewPtr, event );
	}
}

void Web::sendMouseDragEvent( const ci::app::MouseEvent &event )
{
	// send mouse events to Awesomium
	if (mWebViewPtr) {
		ph::awesomium::handleMouseDrag( mWebViewPtr, event );
	}
}

void Web::sendMouseUpEvent( const ci::app::MouseEvent &event )
{
	// send mouse events to Awesomium
	if (mWebViewPtr) {
		ph::awesomium::handleMouseUp( mWebViewPtr, event );
	}
}

void Web::activate()
{
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

void Web::deactivate()
{
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

bool Web::isActive() const
{
  return mActive;
}

void Web::setTransitionTime( const float transitionTime )
{
  mTransitionTime = transitionTime;
}

} // namespace ui
} // namespace ds
