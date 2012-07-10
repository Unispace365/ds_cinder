#include "ds/app/engine.h"

#include "ds/debug/console.h"
#include "ds/debug/debug_defines.h"
#include "cinder/app/App.h"
#include <GL/glu.h>

using namespace ci;
using namespace ci::app;

namespace {
#ifdef _DEBUG
ds::Console		GLOBAL_CONSOLE;
#endif
}

namespace ds {

/**
 * \class ds::Engine
 */
Engine::Engine()
{
	DS_DBG_CODE(GLOBAL_CONSOLE.create());
}

Engine::~Engine()
{
	DS_DBG_CODE(GLOBAL_CONSOLE.destroy());
}

void Engine::update()
{
#if defined DS_PLATFORM_SERVER || defined DS_PLATFORM_SERVERCLIENT
	mWorkManager.update();
#endif


  float curr = static_cast<float>(getElapsedSeconds());
  float dt = curr - mLastTime;
  mLastTime = curr;

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
  if (mRootSprite) {
    mRootSprite->draw(glm::mat4(1.0f), mDrawParams);
  }
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

  mRootSprite = std::move(std::unique_ptr<ui::Sprite>(new ui::Sprite));
  mLastTime = static_cast<float>(getElapsedSeconds());
}

void Engine::loadCinderSettings( ci::app::App::Settings *setting )
{

}

} // namespace ds
