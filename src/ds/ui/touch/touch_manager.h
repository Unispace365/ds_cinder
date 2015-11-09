#pragma once
#ifndef DS_UI_TOUCH_MANAGER_H
#define DS_UI_TOUCH_MANAGER_H

#include <map>
#include <memory>
#include <cinder/app/TouchEvent.h>
#include <cinder/app/MouseEvent.h>
#include <cinder/Color.h>
#include <cinder/Rect.h>
#include "touch_mode.h"
#include "touch_info.h"

namespace ds {
class Engine;

namespace ui {
class RotationTranslator;
class Sprite;
class TouchEvent;

class TouchManager {
public:
class Capture {
public:
	Capture()				{ }
	virtual ~Capture()		{ }
	virtual void			touchBegin(const TouchInfo&) = 0;
	virtual void			touchMoved(const TouchInfo&) = 0;
	virtual void			touchEnd(const TouchInfo&) = 0;
};

public:
	TouchManager(Engine&, const TouchMode::Enum&);

	void						setTouchMode(const TouchMode::Enum&);

	void                        mouseTouchBegin(const ci::app::MouseEvent&, int id );
	void                        mouseTouchMoved(const ci::app::MouseEvent&, int id );
	void                        mouseTouchEnded(const ci::app::MouseEvent&, int id );

	void                        touchesBegin(const ds::ui::TouchEvent&);
	void                        touchesMoved(const ds::ui::TouchEvent&);
	void                        touchesEnded(const ds::ui::TouchEvent&);

	void                        clearFingers( const std::vector<int> &fingers );

	void						setSpriteForFinger( const int fingerId, ui::Sprite* theSprite );
	Sprite*						getSpriteForFinger( const int fingerId );

	void						setOverrideTranslation( const bool doOverride ){ mOverrideTranslation = doOverride; }
	void						setOverrideDimensions( const ci::Vec2f& dimensions ){ mTouchDimensions = dimensions; }
	void						setOverrideOffset( const ci::Vec2f& offset ){ mTouchOffset = offset; }
	void						setTouchFilterRect( const ci::Rectf &filterRect ){ mTouchFilterRect = filterRect; }

	bool						getOverrideEnabled(){ return mOverrideTranslation; }

	// If you've set the override for translation, actually do that translation
	void						overrideTouchTranslation(ci::Vec2f& inOutPoint);

	// If we have a rect defined to discard touches, discard that shit!
	bool						shouldDiscardTouch( const ci::Vec2f& p );

	void						setCapture(Capture*);

	std::map<int, ci::Vec3f>&	getPreviousTouchPoints(){ return mTouchPreviousPoint;  }

  private:
	// Utility to get the hit sprite in either the othorganal or perspective root sprites
	Sprite*                     getHit(const ci::Vec3f &point);

	// If the window is stretched, the mouse points will be off. Fix that shit!
	ci::Vec2f					translateMousePoint(const ci::Vec2i);

	Engine &mEngine;

	std::map<int, ui::Sprite*>	mFingerDispatcher;
	std::map<int, ci::Vec3f>	mTouchStartPoint;
	std::map<int, ci::Vec3f>	mTouchPreviousPoint;
	std::map<int, bool>			mDiscardTouchMap;

	ci::Vec2f					mTouchDimensions;
	ci::Vec2f					mTouchOffset;
	bool						mOverrideTranslation;
	ci::Rectf					mTouchFilterRect;

	TouchMode::Enum				mTouchMode;
	// If system multitouch is on, Cinder will get both mouse and touch events for the first touch.
	// So we track the first touch id to ignore that finger (cause the mouse will count for that)
	int							mIgnoreFirstTouchId;
	// Hack to support the touch trails
	Capture*					mCapture;

	// This is overkill but done this way so I can make changes to
	// the rotation translator without causing a recompile.
	std::shared_ptr<RotationTranslator>	mRotationTranslatorPtr;
	RotationTranslator&					mRotationTranslator;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_MANAGER_H
