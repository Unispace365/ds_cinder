#include "stdafx.h"

#include "touch_translator.h"

namespace ds {
namespace ui {

/**
 * \class TouchTranslator
 */
TouchTranslator::TouchTranslator()
		: mHasTouch(false)
		, mTx(0.0f)
		, mTy(0.0f)
		, mSx(1.0f)
		, mSy(1.0f) {
}

ci::ivec2 TouchTranslator::toWorldi(const int x, const int y) const {
	const ci::vec2		f(toWorldf(static_cast<float>(x), static_cast<float>(y)));
	return ci::ivec2(	static_cast<int>(f.x),
						static_cast<int>(f.y));
}

ci::vec2 TouchTranslator::toWorldf(const float _x, const float _y) const {
	float				x(_x),
						y(_y);

	// Perform the translation to world space
	return ci::vec2(	mTx + (x * mSx),
						mTy + (y * mSy));
}

ds::ui::TouchEvent TouchTranslator::toWorldSpace(const ds::ui::TouchEvent& touchEvent){

	if(touchEvent.getInWorldSpace()){
		return touchEvent;
	}

	std::vector<ci::app::TouchEvent::Touch>	touches;
	for(auto it = touchEvent.getTouches().begin(), end = touchEvent.getTouches().end(); it != end; ++it) {
		ci::vec2 possy = it->getPos();
		ci::vec2 prevPossy = it->getPrevPos();
		possy = toWorldf(possy.x, possy.y);
		prevPossy = toWorldf(prevPossy.x, prevPossy.y);
		touches.push_back(ci::app::TouchEvent::Touch(possy,
			prevPossy,
			it->getId(),
			it->getTime(),
			(void*)it->getNative()));
	}

	return ds::ui::TouchEvent(touchEvent.getWindow(), touches, touchEvent.getInWorldSpace());
}

void TouchTranslator::setTranslation(const float x, const float y) {
	mTx = x;
	mTy = y;
}

void TouchTranslator::setScale(const float x, const float y) {
	mSx = x;
	mSy = y;
}

} // namespace ui
} // namespace ds

template<typename T>
static void write_trans(T& os, const ds::ui::TouchTranslator& o) {
	os << "Touch Translator t=(" << o.getTranslate().x << "," << o.getTranslate().y << ") s=("
			<< o.getScale().x << "," << o.getScale().y << ")";
}

std::ostream& operator<<(std::ostream& os, const ds::ui::TouchTranslator& o) {
	write_trans(os, o);
	return os;
}

std::wostream& operator<<(std::wostream& os, const ds::ui::TouchTranslator& o) {
	write_trans(os, o);
	return os;
}
