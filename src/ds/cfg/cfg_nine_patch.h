#pragma once
#ifndef DS_CFG_CFGNINEPATCH_H_
#define DS_CFG_CFGNINEPATCH_H_

#include "ds/data/key_value_store.h"

namespace ds {
namespace ui {
class NinePatch;
class Sprite;
class SpriteEngine;
}

namespace cfg {
class Settings;
} // namespace cfg
} // namespace ds

namespace ds {
namespace cfg {

/**
 * \class ds::cfg::NinePatch
 * Experimental configuration for a nine patch.
 */
class NinePatch {
public:
	enum Type	{ EMPTY, ARC_DROP_SHADOW };

	NinePatch();
	NinePatch(const Type);

	// Create a new text sprite from this cfg. The throw reference throws the error if anything goes wrong.
	ds::ui::NinePatch*		create(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr) const;
	ds::ui::NinePatch&		createOrThrow(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr, const char* error = nullptr) const;

	void					configure(ds::ui::NinePatch&) const;

	float					getRadius() const;

	Type					mType;
	KeyValueStore			mStore;
};

} // namespace cfg
} // namespace ds

#endif // DS_CFG_CFGNINEPATCH_H_
