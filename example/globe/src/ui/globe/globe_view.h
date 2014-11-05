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
class GlobeView final : public ds::ui::Sprite  {
public:

	GlobeView(Globals& g);

private:
	Globals&			mGlobals; 
	ds::ui::Mesh&		mGlobeMesh;
	ds::ui::Sprite&		mTouchGrabber;
	ds::ui::Sprite*		mGlow;

	virtual void		drawClient(const ci::Matrix44f &trans, const ds::DrawParams &drawParams);

	DelayedMomentum		mXMomentum;
	DelayedMomentum		mYMomentum;

};

} // namespace globe_example

#endif
