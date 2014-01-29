#include "ds/cfg/cfg_nine_patch.h"

#include <ds/ui/image_source/image_drop_shadow.h>
#include <ds/ui/sprite/nine_patch.h>

namespace ds {
namespace cfg {

namespace {
const std::string	EMPTY_SZ("");
const std::string	RADIUS_SZ("radius");
const std::string	FALLOFF_SZ("falloff");
const std::string	COLOR_SZ("color");
const float			DEFAULT_RADIUS(5.0f);
const char*			CREATE_ERROR = "ds::cfg::NinePatch can't create NinePatch sprite";
}

/**
 * \class ds::cfg::NinePatch
 */
NinePatch::NinePatch()
		: mType(EMPTY) {
}

NinePatch::NinePatch(const Type t)
		: mType(t) {
}

ds::ui::NinePatch* NinePatch::create(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent) const {
	if (mType == EMPTY) return nullptr;
	ds::ui::NinePatch*				s = new ds::ui::NinePatch(se);
	if (!s) return nullptr;
	configure(*s);
	if (parent) parent->addChild(*s);
	return s;
}

ds::ui::NinePatch& NinePatch::createOrThrow(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent, const char* error) const {
	ds::ui::NinePatch*				s = create(se, parent);
	if (!s) throw std::runtime_error(error ? error : CREATE_ERROR);
	return *s;
}

void NinePatch::configure(ds::ui::NinePatch& s) const {
	if (mType == ARC_DROP_SHADOW) {
		const float			radius = mStore.getFloat(RADIUS_SZ, 0, DEFAULT_RADIUS),
							falloff = mStore.getFloat(FALLOFF_SZ, 0, 1.0f);
		const ci::ColorA	clr = mStore.getColorA(COLOR_SZ, 0, ci::ColorA(1.0f, 1.0f, 1.0f));
		s.setImage(ds::ui::ImageDropShadow(radius, falloff, ci::Vec2f(0.0f, 0.0f), clr));
	}
}

float NinePatch::getRadius() const {
	return mStore.getFloat(RADIUS_SZ, 0, DEFAULT_RADIUS);
}

} // namespace cfg
} // namespace ds
