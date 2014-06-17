#pragma once
#ifndef NA_APP_APPDEBUG_H_
#define NA_APP_APPDEBUG_H_

#include <framework/debug/Automator.h>
class ofxWorldEngine;

namespace na {
class Globals;

/**
 * \class na::AppDebug
 */
class AppDebug
{
public:
	AppDebug(ofxWorldEngine&, const ds::AppSettings&);

	void								  setup(const na::Globals&);

  void                  update();

  void                  keyPressed(int key);
	void						      keyReleased(int key);

private:
	ofxWorldEngine&				mWorld;

  bool                  mDebugKeysEnabled;
  bool                  mDebugKeyCenterTouchOn;
  float                 mCenterTouchX, mCenterTouchY;

	ds::Automator		      mAutomator;
};

} // namespace na

#endif // NA_APP_APPDEBUG_H_
