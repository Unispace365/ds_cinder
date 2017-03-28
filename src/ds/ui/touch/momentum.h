#pragma once
#ifndef DS_UI_TOUCH_SPRITE_MOMENTUM_H_
#define DS_UI_TOUCH_SPRITE_MOMENTUM_H_

#include <vector>
#include <cinder/Vector.h>
#include <Poco/Timestamp.h>
#include <ds/app/auto_update.h>

namespace ds {
namespace ui {
class Sprite;
} // namespace ui

/**
 * \class ds::Momentum
 * A helper class to have sprites move with momentum in 2d. 
 * This class supersedes drag_assist.
 * 
 * Use:
 *  - Construct as a normal property on your Sprite.
 *  - When you want momentum to take over (say when there are no fingers on your sprite) call activate.
 *  - When the velocity goes below the deactivation threshold, this will deactivate itself.
 *  - If momentum is active and moves the sprite, update will return true so you can check bounds or update graphics, etc.
 *  - Deactivate at any time.
 */

class Momentum : public ds::AutoUpdate {
public:
	Momentum(ds::ui::SpriteEngine& engine);

	// Call activate to begin using momentum. 
	// Use care not to call this more then once when starting, otherwise nothing will happen.
	void						activate();
	// Turn momentum off. Can be called as many times as you like.
	// When not active, this class will continue to keep track of the parent,
	// so you can reactivate whenever you like.
	void						deactivate();
	// Friction is the coefficient of friction. 0.0 is no friction. 1.0 is a lot of friction
	void						setFriction(const double newFriction){ mFriction = newFriction; }
	// How 'heavy' the object feels.
	void						setMass(const double newMass){ mMass = newMass; }
	// how small the velocity is when this class automatically deactivates.
	void						setDeactivationThreshold( const float newThresh ){ mDeactivateThreshold = newThresh; }
	// How many frames of position data to keep for momentum smoothing.
	// When fps = 60, 4-6 frames is a good choice
	// fps = 30, 2-3 frames is good
	// fps = 1000, 30-60 frames is good
	void						setNumberSmoothFrames(const int numFrames){ mNumSmoothFrames = numFrames; }

	void						setMomentumParent(ds::ui::Sprite*);

	// If the sprite moved on the last update, returns true. Otherwise false;
	bool						recentlyMoved(){ return mMoved; }

	// returns the current velocity
	const ci::vec2&			getVelocity();

protected:
	virtual void				update(const ds::UpdateParams&);

private:
	typedef ds::AutoUpdate		inherited;

	ds::ui::Sprite*				mSprite;
	bool						mActive;

	ci::vec2					mLastPosition;
	std::vector<ci::vec2>		mPositionHistory;
	int							mNumSmoothFrames;

	ci::vec2					mVelocity;
	Poco::Timestamp::TimeVal	mLastTime;

	double						mFriction;
	double						mMass;
	double						mDeactivateThreshold;

	double						mVel;
	double						mDir;

	bool						mMoved;
};

} // namespace ds

#endif // DS_UI_TOUCH_SPRITE_MOMENTUM_H_