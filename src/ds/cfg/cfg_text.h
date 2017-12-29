#pragma once
#ifndef DS_CFG_CFGTEXT_H_
#define DS_CFG_CFGTEXT_H_

#include <string>
#include <cinder/Color.h>
#include <cinder/Vector.h>
#include <ds/ui/sprite/text_defs.h>

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;
class Text;
}

namespace cfg {
class Settings;

/**
 * \class ds::cfg::Text
 * Store config settings that can be used to generate a Text sprite
 */
class Text {
public:
	Text();
	Text(const std::string& font, const std::string& configName, const float size, const float leading,
			const ci::ColorA&, const ds::ui::Alignment::Enum& = ds::ui::Alignment::kLeft);

	/// Create a new text sprite from this cfg. Optionally add it to the parent immediately
	ds::ui::Text*			create(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr) const;

	/// Apply this configuration to a text sprite
	void					configure(ds::ui::Text&) const;

	std::string				mFont;
	std::string				mCfgName;
	float					mSize;
	float					mLeading;
	ci::ColorA	  			mColor;
	ci::vec2				mCenter;
	ds::ui::Alignment::Enum	mAlignment;
};

} // namespace cfg
} // namespace ds

#endif // DS_CFG_CFGTEXT_H_