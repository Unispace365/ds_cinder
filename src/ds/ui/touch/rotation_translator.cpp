#include "rotation_translator.h"

#include <ds/math/math_defs.h>
#include <ds/ui/sprite/sprite.h>
#include "touch_info.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::RotationTranslator
 */
RotationTranslator::RotationTranslator() {
}

void RotationTranslator::down(TouchInfo &ti) {
	if (!(ti.mPickedSprite && ti.mPickedSprite->isRotateTouches())) return;

	mMatrix[ti.mFingerId] = buildRotationMatrix(ti.mPickedSprite);
}

void RotationTranslator::move(TouchInfo &ti, const ci::vec3 &previous_global_pt) {
	if (!(ti.mPickedSprite && ti.mPickedSprite->isRotateTouches())) return;

	ci::mat4			m(ci::mat4::identity());

	auto f = mMatrix.find(ti.mFingerId);
	if (f != mMatrix.end()) {
		m = f->second;
	} else {
		m = buildRotationMatrix(ti.mPickedSprite);
		mMatrix[ti.mFingerId] = m;
	}

	ci::vec3				pt(ti.mCurrentGlobalPoint - ti.mStartPoint);
	pt = ti.mStartPoint + m.transformPoint(pt);
	ti.mCurrentGlobalPoint = pt;
	ti.mDeltaPoint = ti.mCurrentGlobalPoint - previous_global_pt;
}

void RotationTranslator::up(TouchInfo &ti) {
	if (mMatrix.empty()) return;

	try {
		auto f = mMatrix.find(ti.mFingerId);
		if (f != mMatrix.end()) mMatrix.erase(f);
	} catch (std::exception const&) {
	}
}

ci::mat4 RotationTranslator::buildRotationMatrix(ds::ui::Sprite *s) const {
	ci::mat4					m(ci::mat4::identity());
	if (!s) return m;

	// Build the matrix in order from root parent down to child
	std::vector<ds::ui::Sprite*>	vec;
	while (s) {
		vec.push_back(s);
		s = s->getParent();
	}
	for (auto it=vec.rbegin(), end=vec.rend(); it!=end; ++it) {
		const ci::vec3				rotation((*it)->getRotation());
		m.rotate(ci::vec3(1.0f, 0.0f, 0.0f), rotation.x * math::DEGREE2RADIAN);
		m.rotate(ci::vec3(0.0f, 1.0f, 0.0f), rotation.y * math::DEGREE2RADIAN);
		m.rotate(ci::vec3(0.0f, 0.0f, 1.0f), rotation.z * math::DEGREE2RADIAN);
	}
	return m;
}

} // namespace ui
} // namespace ds
