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
	, mTouchTrailsUse(false)
{
}

DrawTouchView::DrawTouchView(ds::ui::SpriteEngine& e, const ds::cfg::Settings &settings, ds::ui::TouchManager& tm)
	: ds::ui::Sprite(e)
	, mTouchTrailsUse(false)
	, mTouchTrailsLength(5)
	, mTouchTrailsIncrement(5.0f)
	, mCircleRadius(20.0f)
	, mCircleColor(ci::ColorA::white())
	, mCircleFilled(false)

{
	mTouchTrailsUse = settings.getBool("touch_overlay:trails:use", 0, mTouchTrailsUse);
	mTouchTrailsLength = settings.getInt("touch_overlay:trails:length", 0, mTouchTrailsLength);
	mTouchTrailsIncrement = settings.getFloat("touch_overlay:trails:increment", 0, mTouchTrailsIncrement);

	if(mTouchTrailsUse){
		setTransparent(false); 
		setColor(settings.getColor("touch_color", 0, ci::Color(1.0f, 1.0f, 1.0f)));
	}

	tm.setCapture(this);

	mCircleRadius = settings.getFloat("touch_overlay:debug_circle_radius", 0, mCircleRadius);
	mCircleColor  = settings.getColorA("touch_overlay:debug_circle_color", 0, mCircleColor);
	mCircleFilled = settings.getBool("touch_overlay:debug_circle_filled", 0, mCircleFilled);
}

void DrawTouchView::drawTrails(){
	ds::ui::applyBlendingMode(ds::ui::NORMAL);

	const float			incrementy = mTouchTrailsIncrement;
	for(auto it = mTouchPointHistory.begin(), it2 = mTouchPointHistory.end(); it != it2; ++it) {
		float sizey = incrementy;
		int secondSize = it->second.size();
		ci::Vec2f prevPos = ci::Vec2f::zero();
		for(int i = 0; i < secondSize; i++){
			ci::Vec2f		pos(it->second[i].xy());
			ci::gl::drawSolidCircle(pos, sizey);

			if(i < secondSize - 1 && i > 0){
				// Find the angle between this point and the previous point
				// PI / 2 is a 90 degree rotation, or perpendicular
				float angle = atan2f(pos.y - prevPos.y, pos.x - prevPos.x) + ds::math::PI / 2.0f;
				float smallSize = (sizey - incrementy);
				float bigSize = sizey;
				ci::Vec2f p1 = ci::Vec2f(pos.x + bigSize * cos(angle), pos.y + bigSize * sin(angle));
				ci::Vec2f p2 = ci::Vec2f(pos.x - bigSize * cos(angle), pos.y - bigSize * sin(angle));
				ci::Vec2f p3 = ci::Vec2f(prevPos.x + smallSize * cos(angle), prevPos.y + smallSize * sin(angle));
				ci::Vec2f p4 = ci::Vec2f(prevPos.x - smallSize * cos(angle), prevPos.y - smallSize * sin(angle));
				glBegin(GL_QUADS);
				ci::gl::vertex(p1);
				ci::gl::vertex(p3);
				ci::gl::vertex(p4);
				ci::gl::vertex(p2);
				glEnd();
			}

			sizey += incrementy;

			prevPos = pos;
		}
	}
}

void DrawTouchView::drawLocalServer(){
	if(mTouchTrailsUse) {
		drawTrails();
	} else {
		Sprite::drawLocalClient();
	}
}

void DrawTouchView::drawLocalClient(){
	if(mTouchTrailsUse) {
		drawTrails();
	} else {
		Sprite::drawLocalClient();
	}
}

void DrawTouchView::touchBegin(const ds::ui::TouchInfo &ti){
	if(mTouchTrailsUse) {
		mTouchPointHistory[ti.mFingerId] = std::vector<ci::Vec3f>();
		mTouchPointHistory[ti.mFingerId].push_back(ti.mCurrentGlobalPoint);
	} else {
		if(mCircles[ti.mFingerId]){
			mCircles[ti.mFingerId]->show();
		} else {
			Circle* circley = new Circle(mEngine, mCircleFilled, mCircleRadius);
			circley->setColorA(mCircleColor);
			circley->setDrawDebug(true);
			circley->setCenter(0.5f, 0.5f);
			addChildPtr(circley);
			mCircles[ti.mFingerId] = circley;
		}

		if(mCircles[ti.mFingerId]) mCircles[ti.mFingerId]->setPosition(ti.mCurrentGlobalPoint);
	}
}

void DrawTouchView::touchMoved(const ds::ui::TouchInfo &ti){
	if(mTouchTrailsUse) {
		mTouchPointHistory[ti.mFingerId].push_back(ti.mCurrentGlobalPoint);
		if((int)mTouchPointHistory[ti.mFingerId].size() > mTouchTrailsLength - 1) {
			mTouchPointHistory[ti.mFingerId].erase(mTouchPointHistory[ti.mFingerId].begin());
		}
	} else if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->setPosition(ti.mCurrentGlobalPoint);
	}
}

void DrawTouchView::touchEnd(const ds::ui::TouchInfo &ti){
	if(mTouchTrailsUse) {
		mTouchPointHistory.erase(ti.mFingerId);
	} else if(mCircles[ti.mFingerId]){
		mCircles[ti.mFingerId]->hide();
	}
}

}
}
