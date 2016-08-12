#pragma once
#ifndef DS_CFG_CFGTEXT_H_
#define DS_CFG_CFGTEXT_H_

#include <string>
#include <cinder/Color.h>
#include <cinder/Vector.h>
#include <ds/ui/sprite/text_defs.h>

namespace ds {
namespace ui {
class MultilineText;
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

	// Create a new text sprite from this cfg. The throw reference throws the error if anything goes wrong.
	ds::ui::Text*			create(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr) const;
	ds::ui::Text&			createOrThrow(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr, const char* error = nullptr) const;
	ds::ui::MultilineText*	createMultiline(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr) const;
	ds::ui::MultilineText&	createOrThrowMultiline(ds::ui::SpriteEngine&, ds::ui::Sprite* parent = nullptr, const char* error = nullptr) const;

	void					configure(ds::ui::Text&) const;

	std::string				mFont;
	std::string				mCfgName;
	float					mSize;
	float					mLeading;
	ci::ColorA				mColor;
	ci::vec2				mCenter;
	ds::ui::Alignment::Enum	mAlignment;
};

} // namespace cfg
} // namespace ds

#endif // DS_CFG_CFGTEXT_H_