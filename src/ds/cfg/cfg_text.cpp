#include "stdafx.h"

#include "ds/cfg/cfg_text.h"


namespace ds {
namespace cfg {

Text::Text()
		: mCenter(0.0f, 0.0f)
		, mAlignment(ds::ui::Alignment::kLeft) {
}

Text::Text(const std::string& font, const std::string& configName, const float size, const float leading,
			const float letterSpacing, const ci::ColorA& c, const ds::ui::Alignment::Enum& alignment)
		: mFont(font)
		, mCfgName(configName)
		, mSize(size)
		, mLeading(leading)
		, mLetterSpacing(letterSpacing)
		, mColor(c)
		, mCenter(0.0f, 0.0f)
		, mAlignment(alignment) {
}

ds::ui::Text* Text::create(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent) const {
	if (mFont.empty()) return nullptr;
	ds::ui::Text*				s = new ds::ui::Text(se);
	if (!s) return nullptr;
	configure(*s);
	if (parent) parent->addChild(*s);
	return s;
}

void Text::configure(ds::ui::Text& s) const {
	s.setConfigName(mCfgName);
	s.setFont(mFont, mSize);
	s.setColorA(mColor);
	s.setLeading(mLeading);
	s.setLetterSpacing(mLetterSpacing);
	s.setAlignment(mAlignment);	
}

} // namespace cfg
} // namespace ds
