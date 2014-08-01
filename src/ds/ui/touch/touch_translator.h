#pragma once
#ifndef DS_UI_TOUCH_TOUCHTRANSLATOR_H_
#define DS_UI_TOUCH_TOUCHTRANSLATOR_H_

#include <cinder/Vector.h>

namespace ds {
namespace ui {

/**
 * \class ds::ui::TouchTranslator
 * \brief Translate coordinates from screen space to world space.
 */
class TouchTranslator {
public:
	TouchTranslator();

	ci::Vec2i		toWorldi(const int x, const int y) const;
	ci::Vec2f		toWorldf(const float x, const float y) const;

	void			setTranslation(const float x, const float y);
	void			setScale(const float x, const float y);

private:
	float			mTx, mTy,
					mSx, mSy;
};

} // namespace ui
} // namespace ds

#endif
