#include "stdafx.h"

#include "draw_touch_view.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"

#include <cinder/TriMesh.h>
#include <cinder/Triangulate.h>

#include "ds/ui/sprite/circle.h"

namespace ds{
namespace ui{

DrawTouchView::DrawTouchView(ds::ui::SpriteEngine& e)
	: ds::ui::Sprite(e)
{
}

DrawTouchView::DrawTouchView(ds::ui::SpriteEngine& e, ds::cfg::Settings &settings, ds::ui::TouchManager& tm)
	: ds::ui::Sprite(e)
	, mCircleRadius(20.0f)
	, mCircleColor(ci::ColorA::white())
	, mCircleFilled(false) {

	tm.setCapture(this);

	mCircleRadius = settings.getFloat("touch_overlay:debug_circle_radius", 0, mCircleRadius);
	mCircleColor = settings.getColorA(dynamic_cast<ds::Engine&>(e), "touch_overlay:debug_circle_color", 0, mCircleColor);
	mCircleFilled = settings.getBool("touch_overlay:debug_circle_filled", 0, mCircleFilled);
}


void DrawTouchView::drawLocalServer(){
	Sprite::drawLocalClient();
}

void DrawTouchView::touchBegin(const ds::ui::TouchInfo &ti){
	if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->show();
	} else {
		Circle* circley = new Circle(mEngine, mCircleFilled, 0.0f);
		circley->setColorA(mCircleColor);
		circley->setDrawDebug(true);
		circley->setCenter(0.5f, 0.5f);
		addChildPtr(circley);
		mCircles[ti.mFingerId] = circley;
	}
	if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->setPosition(ti.mCurrentGlobalPoint);
		mCircles[ti.mFingerId]->setRadius(mCircleRadius);
	}
}

void DrawTouchView::touchMoved(const ds::ui::TouchInfo &ti){
	if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->setPosition(ti.mCurrentGlobalPoint);
	}
}

void DrawTouchView::touchEnd(const ds::ui::TouchInfo &ti){
	if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->hide();
	}
}

}
}
