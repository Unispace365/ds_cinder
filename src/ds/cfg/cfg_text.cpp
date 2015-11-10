#include "ds/cfg/cfg_text.h"

#include "ds/ui/sprite/multiline_text.h"

namespace ds {
namespace cfg {

namespace {
const std::string	EMPTY_SZ("");
const char*			CREATE_ERROR = "ds::cfg::Text can't create Text sprite";
}

/**
 * \class ds::cfg::Text
 */
Text::Text()
		: mCenter(0.0f, 0.0f)
		, mAlignment(ds::ui::Alignment::kLeft) {
}

Text::Text(	const std::string& font, const float size, const float leading,
			const ci::ColorA& c, const ds::ui::Alignment::Enum& alignment)
		: mFont(font)
		, mSize(size)
		, mLeading(leading)
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

ds::ui::Text& Text::createOrThrow(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent, const char* error) const {
	ds::ui::Text*				s = create(se);
	if (!s) throw std::runtime_error(error ? error : CREATE_ERROR);
	if (parent) parent->addChild(*s);
	return *s;
}

ds::ui::MultilineText* Text::createMultiline(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent) const {
	if (mFont.empty()) return nullptr;
	ds::ui::MultilineText*		s = new ds::ui::MultilineText(se);
	if (!s) return nullptr;
	configure(*s);
	if (parent) parent->addChild(*s);
	return s;
}

ds::ui::MultilineText& Text::createOrThrowMultiline(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent, const char* error) const {
	ds::ui::MultilineText*		s = createMultiline(se);
	if (!s) throw std::runtime_error(error ? error : CREATE_ERROR);
	if (parent) parent->addChild(*s);
	return *s;
}

void Text::configure(ds::ui::Text& s) const {
	s.setFont(mFont, mSize);
	s.setColorA(mColor);
	ds::ui::MultilineText*		mt = dynamic_cast<ds::ui::MultilineText*>(&s);
	if (mt) {
		if (mLeading >= 0.0f) mt->setLeading(mLeading);
		mt->setAlignment(mAlignment);
	}
}

float Text::retrunFontSize() const
{
	if (mSize > 0)
		return mSize;
}

} // namespace cfg
} // namespace ds
