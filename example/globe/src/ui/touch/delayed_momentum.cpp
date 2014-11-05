#include "delayed_momentum.h"

#include "app/globals.h"

namespace globe_example {

/**
* \class ds::Momentum
*/
DelayedMomentum::DelayedMomentum(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mCurrentDelta(0.0f)
{
	mDeltaBurndown = mGlobals.getSettingsLayout().getFloat("story:scroll:momentum:delta_burndown", 0, 0.9f);
	mNumSmoothFrames = mGlobals.getSettingsLayout().getInt("story:scroll:momentum:max_smooth_frames", 0, 10);
}

void DelayedMomentum::addDeltaPosition(const float delta){
	mDeltas.push_back(delta);
}

const float DelayedMomentum::getDelta(){
	return mCurrentDelta;
}

void DelayedMomentum::update(const ds::UpdateParams& p) {
	if(mDeltas.empty()){
		mCurrentDelta = 0.0f;
		return;
	}

	if(!mDeltas.empty()){
		float totesMahgoats = 0.0f;
		for(int i = 0; i < mDeltas.size(); i++){
			totesMahgoats += mDeltas[i];
			mDeltas[i] = mDeltas[i] * mDeltaBurndown;
		}
		totesMahgoats /= (float)mDeltas.size();
		mCurrentDelta = totesMahgoats;
	}

	while(mDeltas.size() > mNumSmoothFrames){
		mDeltas.erase(mDeltas.begin());
	}
}

void DelayedMomentum::clear(){
	mCurrentDelta = 0.0f;
	mDeltas.clear();
}


} // namespace ds
