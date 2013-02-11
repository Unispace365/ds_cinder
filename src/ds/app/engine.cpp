#include "ds/app/engine.h"

#include <GL/glu.h>
#include "ds/app/app.h"
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/config/settings.h"
#include "Poco/Path.h"
#include "cinder/Thread.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

using namespace ci;
using namespace ci::app;

namespace {

const int NUMBER_OF_NETWORK_THREADS = 2;

}

const ds::BitMask	ds::ENGINE_LOG = ds::Logger::newModule("engine");

namespace ds {

const int Engine::NumberOfNetworkThreads = 2;

/**
 * \class ds::Engine
 */
Engine::Engine(ds::App& app, const ds::cfg::Settings &settings)
  : mTweenline(app.timeline())
  , mRootSprite(*this, ROOT_SPRITE_ID)
  , mIdleTime(300.0f)
  , mIdling(true)
  , mTouchManager(*this)
  , mMinTouchDistance(10.0f)
  , mMinTapDistance(30.0f)
  , mSwipeQueueSize(4)
  , mDoubleTapTime(0.1f)
  , mSettings(settings)
  , mSystemMultitouchEnabled(false)
{
  const std::string     DEBUG_FILE("debug.xml");
  mDebugSettings.readFrom(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), DEBUG_FILE), false);
  mDebugSettings.readFrom(ds::Environment::getLocalSettingsPath(DEBUG_FILE), true);
  ds::Logger::setup(mDebugSettings);
  mScreenRect = settings.getRect("local_rect", 0, Rectf(0.0f, 640.0f, 0.0f, 400.0f));
  mWorldSize = settings.getSize("world_dimensions", 0, Vec2f(640.0f, 400.0f));
  mTouchManager.setTouchColor(settings.getColor("touch_color", 0, ci::Color(1.0f, 1.0f, 1.0f)));
  mDrawTouches = settings.getBool("touch_overlay:debug", 0, false);
  mIdleTime = settings.getFloat("idle_time", 0, 300.0f);
  mMinTapDistance = settings.getFloat("tap_threshold", 0, 30.0f);

  bool scaleWorldToFit = mDebugSettings.getBool("scale_world_to_fit", 0, false);

  mRootSprite.setSize(mScreenRect.getWidth(), mScreenRect.getHeight());

  if (scaleWorldToFit) {
    mRootSprite.setScale(getWidth()/getWorldWidth(), getHeight()/getWorldHeight());
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
  mTuio.disconnect();
}

ui::Sprite &Engine::getRootSprite()
{
  return mRootSprite;
}

void Engine::updateClient()
{
  float curr = static_cast<float>(getElapsedSeconds());
  float dt = curr - mLastTime;
  mLastTime = curr;

  if (!mIdling && (curr - mLastTouchTime) >= mIdleTime ) {
    mIdling = true;
  }

  mUpdateParams.setDeltaTime(dt);
  mUpdateParams.setElapsedTime(curr);

  mRootSprite.updateClient(mUpdateParams);
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

  mUpdateParams.setDeltaTime(dt);
  mUpdateParams.setElapsedTime(curr);

  mAutoUpdate.update();

  mRootSprite.updateServer(mUpdateParams);
}


void Engine::drawClient()
{

  {
    //gl::SaveFramebufferBinding bindingSaver;

    // bind the framebuffer - now everything we draw will go there
    //mFbo.bindFramebuffer();

    //mCamera.setOrtho(mFbo.getBounds().getX1(), mFbo.getBounds().getX2(), mFbo.getBounds().getY2(), mFbo.getBounds().getY1(), -1.0f, 1.0f);
    gl::setMatrices(mCamera);

    gl::enableAlphaBlending();
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    gl::clear( Color( 0.0f, 0.0f, 0.0f ) );

    mRootSprite.drawClient(Matrix44f::identity(), mDrawParams);

    if (mDrawTouches)
      mTouchManager.drawTouches();
  }

  //gl::enableAlphaBlending();
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  //gl::setMatrices(mCamera);
  //gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
  //gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
  //Rectf screen(0.0f, getHeight(), getWidth(), 0.0f);
  //gl::draw( mFbo.getTexture(0), screen );

}

void Engine::drawServer()
{
  gl::enableAlphaBlending();
  gl::clear( Color( 0.0f, 0.0f, 0.0f ) );

  gl::setMatrices(mCamera);

  mRootSprite.drawServer(Matrix44f::identity(), mDrawParams);

  if (mDrawTouches)
    mTouchManager.drawTouches();
}

void Engine::setup(ds::App&)
{
  //mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  //gl::setMatrices(mCamera);
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

  settings.setFrameRate(mSettings.getFloat("frame_rate", 0, 60.0f));

  if (mSettings.getText("screen:mode", 0, "") == "full")
    settings.setFullScreen(true);
  else if (mSettings.getText("screen:mode", 0, "") == "borderless")
    settings.setBorderless(true);
  settings.setAlwaysOnTop(mSettings.getBool("screen:always_on_top", 0, false));

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

static bool illegal_sprite_id(const ds::sprite_id_t id)
{
  return id == EMPTY_SPRITE_ID || id == ROOT_SPRITE_ID;
}

ds::sprite_id_t Engine::nextSpriteId()
{
  static ds::sprite_id_t              ID = 0;
  ++ID;
  while (illegal_sprite_id(ID)) ++ID;
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

void Engine::setCamera()
{
  gl::setViewport(Area((int)mScreenRect.getX1(), (int)mScreenRect.getY2(), (int)mScreenRect.getX2(), (int)mScreenRect.getY1()));
  mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1000.0f, 1000.0f);
  gl::setMatrices(mCamera);
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
}

} // namespace ds
