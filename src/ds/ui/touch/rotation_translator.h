#pragma once
#ifndef DS_UI_TOUCH_ROTATIONTRANSLATOR_H_
#define DS_UI_TOUCH_ROTATIONTRANSLATOR_H_

#include <unordered_map>
#include <cinder/Rect.h>
#include <cinder/Matrix22.h>
#include <cinder/Matrix33.h>
#include <cinder/Matrix44.h>

namespace ds {
namespace ui {
class Sprite;
struct TouchInfo;

/**
 * \class ds::ui::RotationTranslator
 * \brief Utility to translate rotated sprite coords for the touch system.
 */
class RotationTranslator {
public:
	RotationTranslator();

	void						down(TouchInfo&);
	void						move(TouchInfo&, const ci::Vec3f &previous_global_pt);
	void						up(TouchInfo&);

private:
	ci::Matrix44f				buildRotationMatrix(ds::ui::Sprite*) const;

	// Kind of a hack, but if the sprite has been set to account for
	// rotation, then build a rotation matrix on the touch down. This
	// is pretty fragile.
    std::unordered_map<int, ci::Matrix44f>	mMatrix;
};

} // namespace ui
} // namespace ds

#endif
