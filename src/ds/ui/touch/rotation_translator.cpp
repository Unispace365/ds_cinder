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
#if 0
	mMatrix[ti.mFingerId] = buildRotationMatrix(ti.mPickedSprite);
std::cout << "down assign for " << ti.mFingerId << std::endl;
#endif
}

void RotationTranslator::move(TouchInfo &ti, const ci::Vec3f &previous_global_pt) {
	if (!(ti.mPickedSprite && ti.mPickedSprite->isRotateTouches())) return;

#if 0
	ci::Matrix44f			m(ci::Matrix44f::identity());

	auto f = mMatrix.find(ti.mFingerId);
	if (f != mMatrix.end()) {
		m = f->second;
std::cout << "move for " << ti.mFingerId << std::endl;
	} else {
		m = buildRotationMatrix(ti.mPickedSprite);
		mMatrix[ti.mFingerId] = m;
std::cout << "move assign for " << ti.mFingerId << std::endl;
	}

m = buildRotationMatrix(ti.mPickedSprite);

m = ci::Matrix44f::identity();
m.rotate(ci::Vec3f(1.0f, 0.0f, 0.0f), 0.0f * math::DEGREE2RADIAN);
m.rotate(ci::Vec3f(0.0f, 1.0f, 0.0f), 0.0f * math::DEGREE2RADIAN);
m.rotate(ci::Vec3f(0.0f, 0.0f, 1.0f), 180.0f * math::DEGREE2RADIAN);

	ci::Vec3f				pt(ti.mCurrentGlobalPoint - ti.mStartPoint);
	pt = ti.mStartPoint + m.transformPoint(pt);
	ti.mCurrentGlobalPoint = pt;
	ti.mDeltaPoint = ti.mCurrentGlobalPoint - previous_global_pt;
#endif
}

void RotationTranslator::up(TouchInfo &ti) {
	if (mMatrix.empty()) return;
#if 0
	try {
		auto f = mMatrix.find(ti.mFingerId);
		if (f != mMatrix.end()) mMatrix.erase(f);
	} catch (std::exception const&) {
	}
#endif
}

ci::Matrix44f RotationTranslator::buildRotationMatrix(ds::ui::Sprite *s) const {
	ci::Matrix44f					m(ci::Matrix44f::identity());
	if (!s) return m;

	// Build the matrix in order from root parent down to child
	std::vector<ds::ui::Sprite*>	vec;
	while (s) {
		vec.push_back(s);
		s = s->getParent();
	}
std::cout << "build rotation" << std::endl;
	for (auto it=vec.rbegin(), end=vec.rend(); it!=end; ++it) {
		const ci::Vec3f				rotation((*it)->getRotation());
std::cout << "\t" << rotation << std::endl;
		m.rotate(ci::Vec3f(1.0f, 0.0f, 0.0f), rotation.x * math::DEGREE2RADIAN);
		m.rotate(ci::Vec3f(0.0f, 1.0f, 0.0f), rotation.y * math::DEGREE2RADIAN);
		m.rotate(ci::Vec3f(0.0f, 0.0f, 1.0f), rotation.z * math::DEGREE2RADIAN);
	}
	return m;
}

} // namespace ui
} // namespace ds
