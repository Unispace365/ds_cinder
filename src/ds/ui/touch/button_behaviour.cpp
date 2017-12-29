#include "stdafx.h"

#include "button_behaviour.h"

#include <ds/debug/logger.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>

namespace ds {

/**
 * dtv::ButtonGesture
 */
ButtonBehaviour::ButtonBehaviour(ds::ui::Sprite& owner)
		: mOwner(owner)
		, mState(STATE_EMPTY)
		, mOnDownFn(nullptr)
		, mOnEnterFn(nullptr)
		, mOnExitFn(nullptr)
		, mOnClickFn(nullptr)
		, mOnUpFn(nullptr)
		, mIsSetToScale(false)
		, mTouchInsideCheckFunction(nullptr)
{
	enable();
	owner.setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti) { this->handleTouch(ti); });
}

void ButtonBehaviour::setOnDownFn(const std::function<void(const ds::ui::TouchInfo&)>& fn) {
	mOnDownFn = fn;
}

void ButtonBehaviour::setOnEnterFn(const std::function<void(void)>& fn) {
	mOnEnterFn = fn;
}

void ButtonBehaviour::setOnExitFn(const std::function<void(void)>& fn) {
	mOnExitFn = fn;
}

void ButtonBehaviour::setOnClickFn(const std::function<void(void)>& fn) {
	mOnClickFn = fn;
}

void ButtonBehaviour::setOnUpFn(const std::function<void(void)>& fn) {
	mOnUpFn = fn;
}

void ButtonBehaviour::enable() {
	mOwner.enable(true);
	mOwner.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
}

void ButtonBehaviour::disable() {
	mOwner.enable(false);
	mOwner.disableMultiTouch();
}


namespace {
void			anim_scale_to(ds::ui::Sprite& s, const float scale, const float duration) {
	s.mAnimScale.stop();
	s.getEngine().getTweenline().apply(s, s.ANIM_SCALE(), ci::vec3(scale, scale, 1.0f), duration, ci::easeInOutQuad);
}

}

void ButtonBehaviour::setToScale(	const float scale, const float dur,
									const std::function<void(void)>& clickFn) {
	mIsSetToScale = true;
	setOnDownFn([this, scale, dur](const ds::ui::TouchInfo&) {
		anim_scale_to(this->mOwner, scale, dur);
	});
	setOnUpFn([this, dur]() {
		anim_scale_to(this->mOwner, 1.0f, dur);
	});
	setOnEnterFn([this, scale, dur]() {
		anim_scale_to(this->mOwner, scale, dur);
	});
	setOnExitFn([this, dur]() {
		anim_scale_to(this->mOwner, 1.0f, dur);
	});
	setOnClickFn([this, dur, clickFn]() {
		anim_scale_to(this->mOwner, 1.0f, dur);
		if (clickFn) clickFn();
	});
}

void ButtonBehaviour::setToScalePixel(	const float pixels, const float pressedAnimDuration,
										const std::function<void(void)>& clickFn) {
	const float	x = (mOwner.getWidth()-pixels) / mOwner.getWidth(),
				y = (mOwner.getHeight()-pixels) / mOwner.getHeight();
	float scale = (x < y ? x : y);
	if (scale < 0.5f) scale = 0.5f;
	else if (scale > 1.0f) scale = 1.0f;
	setToScale(scale, pressedAnimDuration, clickFn);
}

void ButtonBehaviour::handleTouch(const ds::ui::TouchInfo& ti) {
	try {
		if (ti.mPhase == ds::ui::TouchInfo::Added) {
			if (ti.mNumberFingers <= 1 || mFinger.empty()) {
				mFinger.clear();
				mState = STATE_INSIDE;
				if (mOnDownFn) mOnDownFn(ti);
			}
			mFinger[ti.mFingerId] = ti.mCurrentGlobalPoint;

		} else if (ti.mPhase == ds::ui::TouchInfo::Moved) {
			mFinger[ti.mFingerId] = ti.mCurrentGlobalPoint;
			State			state = STATE_INSIDE;
			for (auto it=mFinger.begin(), end=mFinger.end(); it!=end; ++it) {
				const bool	inside = ownerContains(it->second);
				if (!inside) {
					state = STATE_OUTSIDE;
					break;
				}
			}
			if (state != mState) {
				mState = state;
				if (mState == STATE_INSIDE) {
					if (mOnEnterFn) mOnEnterFn();
				} else if (mState == STATE_OUTSIDE) {
					if (mOnExitFn) mOnExitFn();
				}
			}
		} else if (ti.mPhase == ds::ui::TouchInfo::Removed) {
			if (!mFinger.empty()) {
				auto found = mFinger.find(ti.mFingerId);
				if (found != mFinger.end()) mFinger.erase(found);
			}
			// Yeah... so there's a bug with num fingers and it can get messed
			// up. Turns out my tracking is more reliable.
			if (ti.mNumberFingers < 1 || mFinger.empty()) {
				mState = STATE_EMPTY;
				const bool		inside = ownerContains(ti.mCurrentGlobalPoint);
				if (inside && mOnClickFn) {
					mOnClickFn();
				} else if (!inside && mOnUpFn) {
					mOnUpFn();
				}
			}
		}

	} catch (std::exception const& ex) {
		DS_LOG_WARNING("Exception handling touch in button behaviour: " << ex.what());
	}
}

bool ButtonBehaviour::ownerContains(const ci::vec3& point) const {
	if (mTouchInsideCheckFunction != nullptr)
		return mTouchInsideCheckFunction(point);
	else
		return mOwner.getEngine().getHit( point ) == &mOwner;
	return false;
}

} // namespace ds
