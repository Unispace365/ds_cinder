#include "stdafx.h"

#include "delayed_momentum.h"

namespace ds { namespace ui {

	/**
	 * \class DelayedMomentum
	 */
	DelayedMomentum::DelayedMomentum(ds::ui::SpriteEngine& enging, const float deltaBurndown, const int maxSmoothFrames)
	  : inherited(enging)
	  , mCurrentDelta(0.0f)
	  , mDeltaBurndown(deltaBurndown)
	  , mNumSmoothFrames(maxSmoothFrames) {}

	void DelayedMomentum::addDeltaPosition(const float delta) {
		mDeltas.push_back(delta);
	}

	const float DelayedMomentum::getDelta() {
		return mCurrentDelta;
	}

	void DelayedMomentum::update(const ds::UpdateParams& p) {
		if (mDeltas.empty()) {
			mCurrentDelta = 0.0f;
			return;
		}

		if (!mDeltas.empty()) {
			float totesMahgoats = 0.0f;
			for (int i = 0; i < mDeltas.size(); i++) {
				totesMahgoats += mDeltas[i];
				mDeltas[i] = mDeltas[i] * mDeltaBurndown;
			}
			totesMahgoats /= (float)mDeltas.size();
			mCurrentDelta = totesMahgoats;
		}

		while (mDeltas.size() > mNumSmoothFrames) {
			mDeltas.erase(mDeltas.begin());
		}
	}

	void DelayedMomentum::clear() {
		mCurrentDelta = 0.0f;
		mDeltas.clear();
	}

}} // namespace ds::ui
