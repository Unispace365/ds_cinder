#include "stdafx.h"

#include "donut_arc.h"

#include <ds/app/environment.h>
#include <ds/math/math_defs.h>

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>


namespace ds {
namespace ui {

DonutArc::DonutArc(ds::ui::SpriteEngine& engine, const float theSize)
	: ds::ui::Sprite(engine, theSize, theSize)
	, mDonutWidth(5.0f)
	, mPercent(1.0f)
	, mDrawPercent(1.0f)
	, mIsAntiClock(false) {

	setTransparent(false);
}

DonutArc::~DonutArc() {
	mEngine.getTweenline().getTimeline().removeTarget(&mDrawPercent);
}


void DonutArc::setDonutWidth(const float donutWidth){
	mDonutWidth = donutWidth;
}

void DonutArc::setPercent(const float percent, const bool applyImmediately){
	mPercent = percent;
	if(applyImmediately){
		mDrawPercent = mPercent;
	}
}

float DonutArc::getPercent(){
	return mPercent;
}

void DonutArc::setAntiClock(const bool isAntiClock){
	mIsAntiClock = isAntiClock;
}

bool DonutArc::getIsAntiClock() {
	return mIsAntiClock;
}

void DonutArc::resetDrawPercent(){
	mDrawPercent = 0.0f;
}

void DonutArc::drawLocalClient() {
	if(visible()) {

	//this->getColor(), this->getDrawOpacity()
		// ci::gl::color(ci::ColorA(getColor(), getDrawOpacity()));
		// Number of points based on the outer radius to maintain smoothness
		const int resolution = 32 + (int)(255.0f * fabsf(mDrawPercent));
		auto center = ci::vec3(getWidth() / 2.0f, getHeight() / 2.0f, 0.0f);
		float outerRadius = getWidth() / 2.0f;
		float innerRadius = outerRadius - mDonutWidth;

		ci::gl::begin(GL_TRIANGLE_STRIP);
		ci::vec3 innerPos = ci::vec3(0.0f, -innerRadius, 0);
		ci::vec3 outerPos = ci::vec3(0.0f, -outerRadius, 0);
		for(int i = 0; i <= resolution; ++i){
			ci::gl::vertex(innerPos + center);
			ci::gl::vertex(outerPos + center);

			float rotationDegrees = ((mDrawPercent / (float)resolution)  * 360.5f);
			if(mIsAntiClock) rotationDegrees = -rotationDegrees;
			innerPos = rotateZ(innerPos, ci::toRadians(rotationDegrees));
			outerPos = rotateZ(outerPos, ci::toRadians(rotationDegrees));
		}

		ci::gl::end();
	}

}

void DonutArc::animateOn(const float duration, const float delay, ci::EaseFn theEaseFunction) {
	callAfterDelay([this, duration, theEaseFunction]() {
		mEngine.getTweenline().getTimeline().applyPtr(&mDrawPercent, mPercent, duration, theEaseFunction);
	}, delay);
}

}  // namespace ui
}  // namespace ds
