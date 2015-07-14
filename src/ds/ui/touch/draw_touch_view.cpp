#include "draw_touch_view.h"

#include <ds/math/math_defs.h>

#include <cinder/TriMesh.h>
#include <cinder/Triangulate.h>

#include "ds/ui/sprite/circle.h"

namespace ds{
namespace ui{

DrawTouchView::DrawTouchView(ds::ui::SpriteEngine& e, const ds::cfg::Settings &settings, ds::ui::TouchManager& tm)
	: ds::ui::Sprite(e)
	, mTouchManager(tm)
	, mTouchTrailsUse(false)
	, mTouchTrailsLength(5)
	, mTouchTrailsIncrement(5.0f) {
	mTouchTrailsUse = settings.getBool("touch_overlay:trails:use", 0, mTouchTrailsUse);
	mTouchTrailsLength = settings.getInt("touch_overlay:trails:length", 0, mTouchTrailsLength);
	mTouchTrailsIncrement = settings.getFloat("touch_overlay:trails:increment", 0, mTouchTrailsIncrement);

	setTransparent(false);
	setColor(settings.getColor("touch_color", 0, ci::Color(1.0f, 1.0f, 1.0f)));

	tm.setCapture(this);
	setDrawDebug(true);
#if 0
	if(mTouchTrailsUse) {
		tm.setCapture(this);
	} else {

		// Create a vertex buffer for the circle
		float numPoints = 20.0f;
		float radius = 20.0f;
		float angle = 2 * ds::math::PI / numPoints;
		float centerX = radius;
		float centerY = radius;

		std::vector<ci::Vec2f> points;
		for(int i = 0; i < numPoints; i++) {
			float t = angle * (float)i;
			points.push_back(ci::Vec2f(
				centerX + radius * cos(t),
				centerY + radius * sin(t)
				));
		}

		ci::Shape2d shape = ci::Shape2d();
		shape.moveTo(points[0].x, points[0].y);
		for(auto it = points.begin() + 1; it < points.end(); ++it) {
			shape.lineTo((*it).x, (*it).y);
		}

		ci::TriMesh2d mesh = ci::Triangulator(shape, 1.0f).calcMesh(ci::Triangulator::WINDING_ODD);
		mCircleVbo = ci::gl::VboMesh(mesh);
	}
#endif
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

void DrawTouchView::drawTouches() {
	return;

	std::map<int, ci::Vec3f>& prevTouches = mTouchManager.getPreviousTouchPoints();
	if(prevTouches.empty())
		return;

	applyBlendingMode(NORMAL);

	for(auto it = prevTouches.begin(), it2 = prevTouches.end(); it != it2; ++it) {
		ci::Vec2f		pos(it->second.xy());
		ci::gl::drawStrokedCircle(pos, 20.0f);
	}
}

void DrawTouchView::drawLocalServer(){
	if(mTouchTrailsUse) {
		drawTrails();
	} else {
		drawTouches();
	}
}

void DrawTouchView::drawLocalClient(){
	if(mTouchTrailsUse) {
		drawTrails();
	} else {
		drawTouches();
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
			Circle* circley = new Circle(mEngine, false, 20.0f);
			circley->setColor(ci::Color(1.0f, 1.0f, 1.0f));
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

void DrawTouchView::updateServer(const ds::UpdateParams& updateParams){
	Sprite::updateServer(updateParams);
	sendToFront();
}


}
}
