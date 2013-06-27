#include "ds/app/engine.h"

#ifdef AWESOMIUM
#pragma comment(lib, "awesomium.lib")
#endif

#include <GL/glu.h>
#include "ds/app/app.h"
#include "ds/app/engine_service.h"
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/config/settings.h"
#include "Poco/Path.h"
#include "cinder/Thread.h"
#include "CinderAwesomium.h"
#include "Awesomium/WebSession.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

using namespace ci;
using namespace ci::app;

const char  ds::CMD_SERVER_SEND_WORLD = 1;
const char  ds::CMD_CLIENT_REQUEST_WORLD = 2;

namespace {

const int NUMBER_OF_NETWORK_THREADS = 2;

}

const ds::BitMask	ds::ENGINE_LOG = ds::Logger::newModule("engine");

namespace ds {

const int Engine::NumberOfNetworkThreads = 2;

/**
 * \class ds::Engine
 */
Engine::Engine(ds::App& app, const ds::cfg::Settings &settings, const std::vector<int>* roots)
  : mTweenline(app.timeline())
  , mIdleTime(300.0f)
  , mIdling(true)
  , mTouchManager(*this)
  , mMinTouchDistance(10.0f)
  , mMinTapDistance(30.0f)
  , mSwipeQueueSize(4)
  , mDoubleTapTime(0.1f)
  , mSettings(settings)
  , mCameraPerspNearPlane(1.0f)
  , mCameraPerspFarPlane(1000.0f)
  , mSystemMultitouchEnabled(false)
  , mApplyFxAA(false)
  , mWebCorePtr(nullptr)
  , mWebSessionPtr(nullptr)
{
	// Construct the root sprites
	if (roots) {
		sprite_id_t				id = EMPTY_SPRITE_ID-1;
		for (auto it=roots->begin(), end=roots->end(); it != end; ++it) {
			ui::Sprite*		s = new ui::Sprite(*this, id, (*it) == Engine::CAMERA_PERSP);
			if (!s) throw std::runtime_error("Engine can't create root sprite");
			mRoots.push_back(s);
			--id;
		}
	}
	if (mRoots.empty()) {
		ui::Sprite*		s = new ui::Sprite(*this, EMPTY_SPRITE_ID - 1);
		if (!s) throw std::runtime_error("Engine can't create root sprite");
		mRoots.push_back(s);
	}

  const std::string     DEBUG_FILE("debug.xml");
  mDebugSettings.readFrom(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), DEBUG_FILE), false);
  mDebugSettings.readFrom(ds::Environment::getLocalSettingsPath(DEBUG_FILE), true);
  ds::Logger::setup(mDebugSettings);
  mScreenRect = settings.getRect("local_rect", 0, Rectf(0.0f, 640.0f, 0.0f, 400.0f));
  mWorldSize = settings.getSize("world_dimensions", 0, Vec2f(640.0f, 400.0f));
  mFrameRate = settings.getFloat("frame_rate", 0, 60.0f);
  mTouchManager.setTouchColor(settings.getColor("touch_color", 0, ci::Color(1.0f, 1.0f, 1.0f)));
  mDrawTouches = settings.getBool("touch_overlay:debug", 0, false);
  mIdleTime = settings.getFloat("idle_time", 0, 300.0f);
  mMinTapDistance = settings.getFloat("tap_threshold", 0, 30.0f);
  mApplyFxAA = settings.getBool("FxAA", 0, false);
  mFxAASpanMax = settings.getFloat("FxAA:SpanMax", 0, 2.0);
  mFxAAReduceMul = settings.getFloat("FxAA:ReduceMul", 0, 8.0);
  mFxAAReduceMin = settings.getFloat("FxAA:ReduceMin", 0, 128.0);

  mCameraPosition = ci::Vec3f(0.0f, 0.0f, 100.0f);
  
  mCameraZClipping = settings.getSize("camera:z_clip", 0, ci::Vec2f(1.0f, 1000.0f));
  mCameraFOV = settings.getFloat("camera:fov", 0, 60.0f);

	mScreenToWorld.setScreenSize(mScreenRect.getWidth(), mScreenRect.getHeight());

  const bool scaleWorldToFit = mDebugSettings.getBool("scale_world_to_fit", 0, false);

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		ds::ui::Sprite&			s = *(*it);
		if (s.getPerspective()) {
			s.setSize(mScreenRect.getWidth(), mScreenRect.getHeight());
			s.setDrawSorted(true);
		} else {
			s.setSize(mScreenRect.getWidth(), mScreenRect.getHeight());
			if (scaleWorldToFit) {
				s.setScale(getWidth()/getWorldWidth(), getHeight()/getWorldHeight());
			}
		}
	}

  // SETUP RESOURCES
  std::string resourceLocation = settings.getText("resource_location", 0, "");
  if (resourceLocation.empty()) {
    // This is valid, though unusual
    std::cout << "Engine() has no resource_location setting, is that intentional?" << std::endl;
  } else {
    resourceLocation = Poco::Path::expand(resourceLocation);
    Resource::Id::setupPaths(resourceLocation, settings.getText("resource_db", 0), settings.getText("project_path", 0));
    //std::cout << "db location: " << resourceLocation << std::endl;
    //std::cout << "resource location: " << settings.getText("resource_db", 0) << std::endl;
    //std::cout << "project path location: " << settings.getText("project_path", 0) << std::endl;
    //std::fstream file;
    //file.open("locations.txt", std::fstream::out);
    //if (file.is_open()) {
    //  //std::stringstream ss;

    //  file << "db location: " << resourceLocation << std::endl;
    //  file << "resource location: " << settings.getText("resource_db", 0) << std::endl;
    //  file << "project path location: " << settings.getText("project_path", 0) << std::endl;
 
    //  file.close();
    //}
  }
}

Engine::~Engine()
{
	if (!mServices.empty()) {
		for (auto it=mServices.begin(), end=mServices.end(); it!=end; ++it) {
			delete it->second;
		}
		mServices.clear();
	}

	mTuio.disconnect();

#ifdef AWESOMIUM
  if (mWebCorePtr) {
    Awesomium::WebCore::Shutdown();
  }
#endif
}

void Engine::addService(const std::string& str, ds::EngineService& service)
{
	if (mServices.empty()) {
		mServices[str] = &service;
	} else {
		auto found = mServices.find(str);
		if (found != mServices.end()) {
			delete found->second;
			found->second = nullptr;
		}
		mServices[str] = &service;
	}
}

ds::EngineService& Engine::getService(const std::string& str)
{
	ds::EngineService*	s = mServices[str];
	if (!s) throw std::runtime_error("Service does not exist");
	return *s;
}

int Engine::getRootCount() const
{
	return mRoots.size();
}

ui::Sprite& Engine::getRootSprite(const size_t index)
{
	if (index < 0 || index >= mRoots.size()) throw std::runtime_error("Engine::getRootSprite() on invalid index");
  return *(mRoots[index]);
}

void Engine::updateClient()
{
  float curr = static_cast<float>(getElapsedSeconds());
  float dt = curr - mLastTime;
  mLastTime = curr;

  if (!mIdling && (curr - mLastTouchTime) >= mIdleTime ) {
    mIdling = true;
  }

  if (mWebCorePtr)
    mWebCorePtr->Update();

  mUpdateParams.setDeltaTime(dt);
  mUpdateParams.setElapsedTime(curr);

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->updateClient(mUpdateParams);
	}
}


std::mutex myMutex;

void Engine::updateServer()
{
  float curr = static_cast<float>(getElapsedSeconds());
  //////////////////////////////////////////////////////////////////////////
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    if (!mMouseBeginEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mMouseBeginEvents.begin(), it2 = mMouseBeginEvents.end(); it != it2; ++it) {
    	  mTouchManager.mouseTouchBegin(it->first, it->second);
      }
      mMouseBeginEvents.clear();
    }

    if (!mMouseMovedEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mMouseMovedEvents.begin(), it2 = mMouseMovedEvents.end(); it != it2; ++it) {
        mTouchManager.mouseTouchMoved(it->first, it->second);
      }
      mMouseMovedEvents.clear();
    }
  
    if (!mMouseEndEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mMouseEndEvents.begin(), it2 = mMouseEndEvents.end(); it != it2; ++it) {
        mTouchManager.mouseTouchEnded(it->first, it->second);
      }
      mMouseEndEvents.clear();
    }
    //////////////////////////////////////////////////////////////////////////
    if (!mTouchBeginEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mTouchBeginEvents.begin(), it2 = mTouchBeginEvents.end(); it != it2; ++it) {
        mTouchManager.touchesBegin(*it);
      }
      mTouchBeginEvents.clear();
    }

    if (!mTouchMovedEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mTouchMovedEvents.begin(), it2 = mTouchMovedEvents.end(); it != it2; ++it) {
        mTouchManager.touchesMoved(*it);
      }
      mTouchMovedEvents.clear();
    }

    if (!mTouchEndEvents.empty()) {
      mLastTouchTime = curr;
      mIdling = false;

      for (auto it = mTouchEndEvents.begin(), it2 = mTouchEndEvents.end(); it != it2; ++it) {
        mTouchManager.touchesEnded(*it);
      }
      mTouchEndEvents.clear();
    }
  } // unlock myMutex
  //////////////////////////////////////////////////////////////////////////
  float dt = curr - mLastTime;
  mLastTime = curr;

  if (!mIdling && (curr - mLastTouchTime) >= mIdleTime ) {
    mIdling = true;
  }

  if (mWebCorePtr)
    mWebCorePtr->Update();

  mUpdateParams.setDeltaTime(dt);
  mUpdateParams.setElapsedTime(curr);

  mAutoUpdate.update(mUpdateParams);

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->updateServer(mUpdateParams);
	}
}

void Engine::setCamera(const bool perspective)
{
	if(!perspective){
		gl::setViewport(Area((int)mScreenRect.getX1(), (int)mScreenRect.getY2(), (int)mScreenRect.getX2(), (int)mScreenRect.getY1()));
		mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1, 1);
		//gl::setMatrices(mCamera);
	} else {

    mCameraPersp.setEyePoint( Vec3f(0.0f, 0.0f, 100.0f) );
    mCameraPersp.setCenterOfInterestPoint( Vec3f(0.0f, 0.0f, 0.0f) );
    mCameraPersp.setPerspective( 60.0f, getWindowAspectRatio(), mCameraPerspNearPlane, mCameraPerspFarPlane );
		//mCameraPersp.setPerspective(mCameraFOV, getWindowAspectRatio(), mCameraZClipping.x, mCameraZClipping.y );
		//mCameraPersp.lookAt( mCameraPosition, mCameraTarget, Vec3f(0.0f, 1.0f, 0.0f) );
	}

	setCameraForDraw(perspective);
}

void Engine::setPerspectiveCameraPlanes(const float nearPlane, const float farPlane)
{
  mCameraPerspNearPlane = nearPlane;
  mCameraPerspFarPlane = farPlane;
}

void Engine::setCameraForDraw(const bool perspective){
	if(!perspective){
		//mCamera.setOrtho(mFbo.getBounds().getX1(), mFbo.getBounds().getX2(), mFbo.getBounds().getY2(), mFbo.getBounds().getY1(), -1.0f, 1.0f);
    gl::setMatrices(mCamera);
    gl::disableDepthRead();
    gl::disableDepthWrite();
	} else {
    gl::setMatrices(mCameraPersp);
		mScreenToWorld.update();
    // enable the depth buffer (after all, we are doing 3D)
    //gl::enableDepthRead();
    //gl::enableDepthWrite();
		//gl::translate(-getWorldWidth()/2.0f, -getWorldHeight()/2.0f, 0.0f);
	}
}

void Engine::clearAllSprites()
{
	for (auto it=mRoots.begin(), end=mRoots.end(); it != end; ++it) {
		(*it)->clearChildren();
	}
}

void Engine::drawClient()
{
  glAlphaFunc ( GL_GREATER, 0.001f ) ;
  glEnable ( GL_ALPHA_TEST ) ;

  if (mApplyFxAA) {
    {
      gl::SaveFramebufferBinding bindingSaver;

      // bind the framebuffer - now everything we draw will go there
      mFbo.bindFramebuffer();

      gl::enableAlphaBlending();
      //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
      gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );

			for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
				ds::ui::Sprite*			s = (*it);
				setCameraForDraw(s->getPerspective());
				s->drawClient(ci::gl::getModelView(), mDrawParams);
			}

      if (mDrawTouches)
        mTouchManager.drawTouches();

      mFbo.unbindFramebuffer();
    }

    gl::enableAlphaBlending();
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    setCamera();
    gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
 //   gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
    Rectf screen(0.0f, getHeight(), getWidth(), 0.0f);

    static ci::gl::GlslProg shader;
    if (!shader) {
      std::string location = ds::Environment::getAppFolder("data/shaders");
      std::string name = "fxaa";
      try {
        shader = ci::gl::GlslProg(ci::loadFile((location+"/"+name+".vert").c_str()), ci::loadFile((location+"/"+name+".frag").c_str()));
      } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
      }
    }

    if (shader) {
      shader.bind();
      mFbo.bindTexture();
      shader.uniform("tex0", 0);
      shader.uniform("texcoordOffset", ci::Vec2f(1.0f / getWidth(), 1.0f / getHeight()));
      shader.uniform("FXAA_SPAN_MAX", mFxAASpanMax);
      shader.uniform("FXAA_REDUCE_MUL", 1.0f / mFxAAReduceMul);
      shader.uniform("FXAA_REDUCE_MIN", 1.0f / mFxAAReduceMin);

      //gl::draw( mFbo.getTexture(0), screen );
      gl::drawSolidRect(screen);

      mFbo.unbindTexture();
      shader.unbind();
    } else {
      gl::draw( mFbo.getTexture(0), screen );
    }
  } else {	  
    gl::enableAlphaBlending();
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );

		for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
			ds::ui::Sprite*			s = (*it);
			setCameraForDraw(s->getPerspective());
			if (s->getPerspective()) glClear(GL_DEPTH_BUFFER_BIT);
			s->drawClient(ci::gl::getModelView(), mDrawParams);
		}

    if (mDrawTouches)
      mTouchManager.drawTouches();
  }

  glAlphaFunc ( GL_ALWAYS, 0.001f ) ;
}

void Engine::drawServer()
{
  glAlphaFunc ( GL_GREATER, 0.001f ) ;
  glEnable ( GL_ALPHA_TEST ) ;

  gl::enableAlphaBlending();
  gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		ds::ui::Sprite*			s = (*it);
		const bool					persp = s->getPerspective();
		setCameraForDraw(persp);
		if (persp) {
			s->drawClient(ci::gl::getModelView(), mDrawParams);
		} else {
			s->drawServer(ci::gl::getModelView(), mDrawParams);
		}
	}

  if (mDrawTouches)
    mTouchManager.drawTouches();

  glAlphaFunc ( GL_ALWAYS, 0.001f ) ;
}

void Engine::setup(ds::App&)
{
  //mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  //gl::setMatrices(mCamera);
  setCamera(true);
  setCamera();
  //gl::disable(GL_CULL_FACE);
  //////////////////////////////////////////////////////////////////////////

  gl::Fbo::Format format;
  format.setColorInternalFormat(GL_RGBA32F);
  mFbo = gl::Fbo((int)getWidth(), (int)getHeight(), format);
  //////////////////////////////////////////////////////////////////////////

  float curr = static_cast<float>(getElapsedSeconds());
  mLastTime = curr;
  mLastTouchTime = 0;
  
  mUpdateParams.setDeltaTime(0.0f);
  mUpdateParams.setElapsedTime(curr);
}

void Engine::prepareSettings( ci::app::AppBasic::Settings &settings )
{
  settings.setWindowSize( static_cast<int>(getWidth()),
                          static_cast<int>(getHeight()));
  settings.setResizable(false);

  if (mSettings.getBool("enable_system_multitouch", 0, false)) {
    mSystemMultitouchEnabled = true;
    settings.enableMultiTouch();
  }

  mHideMouse = mSettings.getBool("hide_mouse", 0, false);
  mTuioPort = mSettings.getInt("tuio_port", 0, 3333);

  settings.setFrameRate(mFrameRate);

  if (mSettings.getText("screen:mode", 0, "") == "full")
    settings.setFullScreen(true);
  else if (mSettings.getText("screen:mode", 0, "") == "borderless")
    settings.setBorderless(true);
  settings.setAlwaysOnTop(mSettings.getBool("screen:always_on_top", 0, false));

  if (mSettings.getBool("web:use", 0, false)) {
    //add falg to engine.xml to check here.
    initializeWeb();
  }

  const std::string     nope = "ds:IllegalTitle";
  const std::string     title = mSettings.getText("screen:title", 0, nope);
  if (title != nope) settings.setTitle(title);
}

bool Engine::isIdling() const
{
  if (mIdling)
    return true;
  return false;
}

void Engine::startIdling()
{
  if (mIdling)
    return;

  mIdling = true;
}

ds::sprite_id_t Engine::nextSpriteId()
{
  static ds::sprite_id_t              ID = 0;
  ++ID;
	// Skip negative values.
	if (ID <= EMPTY_SPRITE_ID) ID = EMPTY_SPRITE_ID + 1;
  return ID;
}

void Engine::registerSprite(ds::ui::Sprite& s)
{
  if (s.getId() == ds::EMPTY_SPRITE_ID) {
    DS_LOG_WARNING_M("Engine::registerSprite() on empty sprite ID", ds::ENGINE_LOG);
    assert(false);
    return;
  }
  mSprites[s.getId()] = &s;
}

void Engine::unregisterSprite(ds::ui::Sprite& s)
{
  if (mSprites.empty()) return;
  if (s.getId() == ds::EMPTY_SPRITE_ID) {
    DS_LOG_WARNING_M("Engine::unregisterSprite() on empty sprite ID", ds::ENGINE_LOG);
    assert(false);
    return;
  }
  auto it = mSprites.find(s.getId());
  if (it != mSprites.end()) mSprites.erase(it);
}

ds::ui::Sprite* Engine::findSprite(const ds::sprite_id_t id)
{
  if (mSprites.empty()) return nullptr;
  auto it = mSprites.find(id);
  if (it == mSprites.end()) return nullptr;
  return it->second;
}

void Engine::touchesBegin( TouchEvent event )
{
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mTouchBeginEvents.push_back(event);
  }
  //mTouchManager.touchesBegin(event);
}

void Engine::touchesMoved( TouchEvent event )
{
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mTouchMovedEvents.push_back(event);
  }
  //mTouchManager.touchesMoved(event);
}

void Engine::touchesEnded( TouchEvent event )
{
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mTouchEndEvents.push_back(event);
  }
  //mTouchManager.touchesEnded(event);
}

tuio::Client &Engine::getTuioClient()
{
  return mTuio;
}

void Engine::mouseTouchBegin( MouseEvent event, int id )
{
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mMouseBeginEvents.push_back(MousePair(event, id));
  }
  //mTouchManager.mouseTouchBegin(event, id);
}

void Engine::mouseTouchMoved( MouseEvent event, int id )
{
  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mMouseMovedEvents.push_back(MousePair(event, id));
  }
  //mTouchManager.mouseTouchMoved(event, id);
}

void Engine::mouseTouchEnded( MouseEvent event, int id )
{
  mLastTouchTime = static_cast<float>(getElapsedSeconds());
  mIdling = false;

  {
    boost::lock_guard<boost::mutex> lock(myMutex);
    mMouseEndEvents.push_back(MousePair(event, id));
  }
  //mTouchManager.mouseTouchEnded(event, id);
}

ds::ResourceList& Engine::getResources()
{
  return mResources;
}

const ds::FontList& Engine::getFonts() const
{
  return mFonts;
}

ds::FontList& Engine::editFonts()
{
  return mFonts;
}

float Engine::getMinTouchDistance() const
{
  return mMinTouchDistance;
}

float Engine::getMinTapDistance() const
{
  return mMinTapDistance;
}

unsigned Engine::getSwipeQueueSize() const
{
  return mSwipeQueueSize;
}

float Engine::getDoubleTapTime() const
{
  return mDoubleTapTime;
}

ci::Rectf Engine::getScreenRect() const
{
  return mScreenRect;
}

float Engine::getWidth() const
{
  return mScreenRect.getWidth();
}

float Engine::getHeight() const
{
  return mScreenRect.getHeight();
}

float Engine::getWorldWidth() const
{
  return mWorldSize.x;
}

float Engine::getWorldHeight() const
{
  return mWorldSize.y;
}

void Engine::stopServices()
{
}

bool Engine::systemMultitouchEnabled() const
{
  return mSystemMultitouchEnabled;
}

bool Engine::hideMouse() const
{
  return mHideMouse;
}

void Engine::clearFingers( const std::vector<int> &fingers )
{
	mTouchManager.clearFingers(fingers);
}

void Engine::resetIdleTimeOut()
{
  float curr = static_cast<float>(getElapsedSeconds());
  mLastTime = curr;
  mLastTouchTime = curr;
  mIdling = false;
}

void Engine::setPerspectiveCameraPosition( const ci::Vec3f &pos )
{
  mCameraPersp.setEyePoint(pos);
}

ci::Vec3f Engine::getPerspectiveCameraPosition() const
{
  return mCameraPersp.getEyePoint();
}

void Engine::setPerspectiveCameraTarget( const ci::Vec3f &tar )
{
  mCameraPersp.setCenterOfInterestPoint(tar);
}

ci::Vec3f Engine::getPerspectiveCameraTarget() const
{
  return mCameraPersp.getCenterOfInterestPoint();
}

void Engine::initializeWeb()
{
  #ifdef AWESOMIUM
  Awesomium::WebConfig cnf;
  cnf.log_level = Awesomium::kLogLevel_Verbose;

  // initialize the Awesomium web engine
  mWebCorePtr = Awesomium::WebCore::Initialize( cnf );
  #endif
}

Awesomium::WebCore *Engine::getWebCore() const
{
  return mWebCorePtr;
}

Awesomium::WebSession *Engine::getWebSession() const
{
  return mWebSessionPtr;
}

} // namespace ds
