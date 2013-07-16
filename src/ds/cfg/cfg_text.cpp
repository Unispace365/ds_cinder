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
{
}

Text::Text(const std::string& font, const float size, const float lineH, const ci::ColorA& c)
	: mFont(font)
	, mSize(size)
	, mLineHeight(lineH)
	, mColor(c)
	, mCenter(0.0f, 0.0f)
{
}

ds::ui::Text* Text::create(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent) const
{
	if (mFont.empty()) return nullptr;
	ds::ui::Text*				s = new ds::ui::Text(se);
	if (!s) return nullptr;
	configure(*s);
	if (parent) parent->addChild(*s);
	return s;
}

ds::ui::Text& Text::createOrThrow(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent, const char* error) const
{
	ds::ui::Text*				s = create(se);
	if (!s) throw std::runtime_error(error ? error : CREATE_ERROR);
	if (parent) parent->addChild(*s);
	return *s;
}

ds::ui::MultilineText* Text::createMultiline(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent) const
{
	if (mFont.empty()) return nullptr;
	ds::ui::MultilineText*		s = new ds::ui::MultilineText(se);
	if (!s) return nullptr;
	configure(*s);
	if (parent) parent->addChild(*s);
	return s;
}

ds::ui::MultilineText& Text::createOrThrowMultiline(ds::ui::SpriteEngine& se, ds::ui::Sprite* parent, const char* error) const
{
	ds::ui::MultilineText*		s = createMultiline(se);
	if (!s) throw std::runtime_error(error ? error : CREATE_ERROR);
	if (parent) parent->addChild(*s);
	return *s;
}

void Text::configure(ds::ui::Text& s) const
{
	s.setFont(mFont, mSize);
	s.setColorA(mColor);
}

} // namespace cfg
} // namespace ds
