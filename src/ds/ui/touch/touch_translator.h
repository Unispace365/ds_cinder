#pragma once
#ifndef DS_UI_TOUCH_TOUCHTRANSLATOR_H_
#define DS_UI_TOUCH_TOUCHTRANSLATOR_H_

#include <cinder/Rect.h>
#include <cinder/Vector.h>
#include <ds/ui/touch/touch_event.h>

namespace ds {
namespace ui {

/**
 * \class TouchTranslator
 * \brief Translate coordinates from screen space to world space.
 */
class TouchTranslator {
public:
	TouchTranslator();

	ci::ivec2			toWorldi(const int x, const int y) const;
	ci::vec2			toWorldf(const float x, const float y) const;

	ds::ui::TouchEvent	toWorldSpace(const ds::ui::TouchEvent& inputEvent);

	void				setTranslation(const float x, const float y);
	void				setScale(const float x, const float y);

	ci::vec2			getTranslate() const		{ return ci::vec2(mTx, mTy); }
	ci::vec2			getScale() const			{ return ci::vec2(mSx, mSy); }

private:
	bool			mHasTouch;
	float			mTx, mTy,
					mSx, mSy;
};

} // namespace ui
} // namespace ds

// Make the TouchTranslator available to standard stream operators
std::ostream&			operator<<(std::ostream&, const ds::ui::TouchTranslator&);
std::wostream&			operator<<(std::wostream&, const ds::ui::TouchTranslator&);

#endif
