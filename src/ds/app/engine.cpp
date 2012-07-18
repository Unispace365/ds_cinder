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

namespace {

const int NUMBER_OF_NETWORK_THREADS = 2;

}

const ds::BitMask	ds::ENGINE_LOG = ds::Logger::newModule("engine");

namespace ds {

const int Engine::NumberOfNetworkThreads = 2;

/**
 * \class ds::Engine
 */
Engine::Engine(const ds::cfg::Settings &settings)
  : mRootSprite(*this, ROOT_SPRITE_ID)
  , mIdleTime(300.0f)
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
  mTouchManager.setTouchColor(settings.getColor("touch_color", 0, ci::Color(1.0f, 1.0f, 1.0f)));
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

void Engine::updateServer()
{
}

void Engine::drawClient()
{
  {
    gl::SaveFramebufferBinding bindingSaver;

    // bind the framebuffer - now everything we draw will go there
    mFbo.bindFramebuffer();

    mCamera.setOrtho(mFbo.getBounds().getX1(), mFbo.getBounds().getX2(), mFbo.getBounds().getY2(), mFbo.getBounds().getY1(), -1.0f, 1.0f);
    gl::setMatrices(mCamera);

    gl::enableAlphaBlending();
    gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

    mRootSprite.drawClient(Matrix44f::identity(), mDrawParams);

    mTouchManager.drawTouches();
  }


  mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  gl::setMatrices(mCamera);
  gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
  gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
  Rectf screen(0.0f, getHeight(), getWidth(), 0.0f);
  gl::draw( mFbo.getTexture(0), screen );
}

void Engine::drawServer()
{
  gl::enableAlphaBlending();
  gl::clear( Color( 0.5f, 0.5f, 0.5f ) );

  gl::setMatrices(mCamera);

  mRootSprite.drawServer(Matrix44f::identity(), mDrawParams);

  mTouchManager.drawTouches();
}

void Engine::setup()
{
  mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  gl::setMatrices(mCamera);
  //gl::disable(GL_CULL_FACE);
  //////////////////////////////////////////////////////////////////////////

  gl::Fbo::Format format;
  format.setColorInternalFormat(GL_RGBA32F);
  mFbo = gl::Fbo(getWidth(), getHeight(), format);
  //////////////////////////////////////////////////////////////////////////

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

void Engine::setCamera()
{
  gl::setViewport(Area(mScreenRect.getX1(), mScreenRect.getY2(), mScreenRect.getX2(), mScreenRect.getY1()));
  mCamera.setOrtho(mScreenRect.getX1(), mScreenRect.getX2(), mScreenRect.getY2(), mScreenRect.getY1(), -1.0f, 1.0f);
  gl::setMatrices(mCamera);
}

} // namespace ds
