#pragma once
#ifndef DS_EXAMPLE_GLOBE_UI_GLOBE_VIEW
#define DS_EXAMPLE_GLOBE_UI_GLOBE_VIEW


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/mesh.h>

#include "ui/touch/delayed_momentum.h"

namespace globe_example {
class Globals;

/**
* \class GlobeView - where the magic happens (TM).
*/
class GlobeView : public ds::ui::Sprite  {
public:

	GlobeView(Globals& g);
	ci::vec3 getGlobeRotation() { return mGlobeRotation; }


private:
	Globals&			mGlobals; 
	ds::ui::Sprite&		mTouchGrabber;

	ci::gl::Texture2dRef mTexDiffuse;
	ci::gl::Texture2dRef mTexNormal;
	ci::gl::Texture2dRef mTexMask;

	virtual void		onUpdateServer(const ds::UpdateParams& updateParams) override;
	virtual void		drawLocalClient() override;

	ci::gl::BatchRef	mEarth;

	DelayedMomentum		mXMomentum;
	DelayedMomentum		mYMomentum;

	float				mMinTilt;
	float				mMaxTilt;

	ci::vec3			mGlobeRotation;

};

} // namespace globe_example

#endif
