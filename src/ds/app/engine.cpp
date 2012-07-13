#include "ds/app/engine.h"

#include "ds/debug/debug_defines.h"
#include "cinder/app/App.h"
#include <GL/glu.h>
#include "ds/app/environment.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/config/settings.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

using namespace ci;
using namespace ci::app;

namespace ds {

/**
 * \class ds::Engine
 */
Engine::Engine(const ds::cfg::Settings &settings)
  : mIdleTime(300.0f)
  , mIdling(true)
  , mTouchManager(*this)
  , mMinTouchDistance(10.0f)
  , mMinTapDistance(10.0f)
  , mSwipeQueueSize(4)
  , mDoubleTapTime(0.1f)
{
  const std::string     DEBUG_FILE("debug.xml");
  mDebugSettings.readFrom(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), DEBUG_FILE), false);
  mDebugSettings.readFrom(ds::Environment::getLocalSettingsPath(DEBUG_FILE), true);
  ds::Logger::setup(mDebugSettings);
  mScreenRect = settings.getRect("local_rect", 0, Rectf(0.0f, 640.0f, 0.0f, 400.0f));
  mWorldSize = settings.getSize("world_dimensions", 0, Vec2f(640.0f, 400.0f));
}

Engine::~Engine()
{
  mTuio.disconnect();
}

ui::Sprite &Engine::getRootSprite()
{
  return *mRootSprite;
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

  if (mRootSprite) {
    mRootSprite->updateClient(mUpdateParams);
  }
}

void Engine::updateServer()
{
}

void Engine::drawClient()
{
  gl::enableAlphaBlending();
  gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

  gl::setMatrices(mCamera);

  if (mRootSprite) {
    mRootSprite->drawClient(Matrix44f::identity(), mDrawParams);
  }

  mTouchManager.drawTouches();
}

void Engine::drawServer()
{
  gl::enableAlphaBlending();
  gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

  gl::setMatrices(mCamera);

  if (mRootSprite) {
    mRootSprite->drawServer(Matrix44f::identity(), mDrawParams);
  }

//  mTouchManager.drawTouches();
}

void Engine::setup()
{
  mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  gl::setMatrices(mCamera);
  //gl::disable(GL_CULL_FACE);
  //////////////////////////////////////////////////////////////////////////

  mRootSprite = std::move(std::unique_ptr<ui::Sprite>(new ui::Sprite(*this)));
  float curr = static_cast<float>(getElapsedSeconds());
  mLastTime = curr;
  mLastTouchTime = 0;
}

void Engine::loadCinderSettings( ci::app::App::Settings *settings )
{
  if (!settings) return;

  settings->setWindowSize(static_cast<int>(getWidth()),
                          static_cast<int>(getHeight()));
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

void Engine::touchesBegin( TouchEvent event )
{
  mLastTouchTime = static_cast<float>(getElapsedSeconds());
  mIdling = false;

  mTouchManager.touchesBegin(event);
}

void Engine::touchesMoved( TouchEvent event )
{
  mLastTouchTime = static_cast<float>(getElapsedSeconds());
  mIdling = false;

  mTouchManager.touchesMoved(event);
}

void Engine::touchesEnded( TouchEvent event )
{
  mLastTouchTime = static_cast<float>(getElapsedSeconds());
  mIdling = false;

  mTouchManager.touchesEnded(event);
}

tuio::Client &Engine::getTuioClient()
{
  return mTuio;
}

void Engine::mouseTouchBegin( MouseEvent event, int id )
{
  mTouchManager.mouseTouchBegin(event, id);
}

void Engine::mouseTouchMoved( MouseEvent event, int id )
{
  mTouchManager.mouseTouchMoved(event, id);
}

void Engine::mouseTouchEnded( MouseEvent event, int id )
{
  mTouchManager.mouseTouchEnded(event, id);
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

} // namespace ds
