#include "ds/app/engine.h"

#include "ds/debug/debug_defines.h"
#include "cinder/app/App.h"
#include <GL/glu.h>
#include "ds/math/math_defs.h"
#include "ds/config/settings.h"

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
  mScreenRect = settings.getRect("local_rect", 0, Rectf(0.0f, 0.0f, 0.0f, 0.0f));
}

Engine::~Engine()
{
  mTuio.disconnect();
}

void Engine::update()
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
    mRootSprite->update(mUpdateParams);
  }
}

ui::Sprite &Engine::getRootSprite()
{
  return *mRootSprite;
}

void Engine::draw()
{
  gl::enableAlphaBlending();
  gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

  if (mRootSprite) {
    mRootSprite->draw(Matrix44f::identity(), mDrawParams);
  }

  mTouchManager.drawTouches();
}

void Engine::setup()
{
  // setup gl view and projection
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glViewport( 0, 0, getWindowWidth(), getWindowHeight() );
  //gluPerspective( 41.95f, float(SCREEN_WIDTH)/SCREEN_HEIGHT, 0.1f, 1000.0f );
  gluOrtho2D( 0, getWindowWidth(), getWindowHeight(), 0 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  //////////////////////////////////////////////////////////////////////////

  mRootSprite = std::move(std::unique_ptr<ui::Sprite>(new ui::Sprite(*this)));
  float curr = static_cast<float>(getElapsedSeconds());
  mLastTime = curr;
  mLastTouchTime = 0;
}

void Engine::loadCinderSettings( ci::app::App::Settings *setting )
{

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

} // namespace ds
