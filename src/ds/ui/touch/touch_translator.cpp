#include "touch_translator.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::TouchTranslator
 */
TouchTranslator::TouchTranslator()
		: mHasTouch(false)
		, mTx(0.0f)
		, mTy(0.0f)
		, mSx(1.0f)
		, mSy(1.0f) {
}

ci::Vec2i TouchTranslator::toWorldi(const int x, const int y) const {
	const ci::Vec2f		f(toWorldf(static_cast<float>(x), static_cast<float>(y)));
	return ci::Vec2i(	static_cast<int>(f.x),
						static_cast<int>(f.y));
}

ci::Vec2f TouchTranslator::toWorldf(const float _x, const float _y) const {
	float				x(_x),
						y(_y);
	// If we've got a touch overlay override, run it through that, first.
#if 0
	if (mHasTouch) {
		x -= mTouchDst.x1;
		y -= mTouchDst.y1;

//		x -= mTouchSrc.x1;
//		y -= mTouchSrc.y1;
//		x *= ((mTouchDst.x2-mTouchDst.x1) / (mTouchSrc.x2-mTouchSrc.x1));
//		y *= ((mTouchDst.y2-mTouchDst.y1) / (mTouchSrc.y2-mTouchSrc.y1));
//		x += 
	}
#endif
	// Perform the translation to world space
	return ci::Vec2f(	mTx + (x * mSx),
						mTy + (y * mSy));
}

void TouchTranslator::setTranslation(const float x, const float y) {
	mTx = x;
	mTy = y;
}

void TouchTranslator::setScale(const float x, const float y) {
	mSx = x;
	mSy = y;
}

void TouchTranslator::setTouchOverlay(const ci::Rectf &src, const ci::Rectf &dst) {
	mHasTouch = false;
	mTouchSrc = src;
	mTouchDst = dst;
	if (src.x2 > src.x1 && src.y2 > src.y1 && dst.x2 > dst.x1 && dst.y2 > dst.y1) {
		mHasTouch = true;
	}
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
