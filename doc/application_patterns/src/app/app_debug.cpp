#include "na/app/app_debug.h"

#include <ofxWorldEngine.h>

#include "na/app/globals.h"
#include "na/app/app_resources.h"

namespace na {

namespace {
const int         CENTER_TOUCH_KEY = 'c';
}

/**
 * na::AppDebug
 */
AppDebug::AppDebug(ofxWorldEngine& world, const ds::AppSettings& appSettings)
	: mWorld(world)
  , mDebugKeysEnabled(false)
  , mDebugKeyCenterTouchOn(false)
  , mCenterTouchX(0)
  , mCenterTouchY(0)
	, mAutomator(appSettings, "debug.xml", world, na::FONT_NETAPPSCRIPT_OTF)
{
  mAutomator.setPeriod(0.1f);
}

void AppDebug::setup(const na::Globals& g)
{
  mDebugKeysEnabled = g.mDebug.getBool("enable_keys", 0, false);

  mCenterTouchX = mWorld.getMasterWidth() / 2;
  mCenterTouchY = mWorld.getMasterHeight() / 2;

	mAutomator.setFrame(ds::RectF(0, 0, mWorld.getMasterWidth(), mWorld.getMasterHeight()));
}

void AppDebug::update()
{
	mAutomator.update();
}

void AppDebug::keyPressed(int key)
{
  if (!mDebugKeysEnabled) return;

  if (key == CENTER_TOUCH_KEY) {
    if (!mDebugKeyCenterTouchOn) mWorld.sendTouchDown(mCenterTouchX, mCenterTouchY, 1);
    mDebugKeyCenterTouchOn = true;
  }
}

void AppDebug::keyReleased(int key)
{
	if (mAutomator.keyReleased(key)) return;
  if (!mDebugKeysEnabled) return;

  if (key == CENTER_TOUCH_KEY) {
    if (mDebugKeyCenterTouchOn) mWorld.sendTouchUp(mCenterTouchX, mCenterTouchY, 1);
    mDebugKeyCenterTouchOn = false;
  }
}

} // namespace na