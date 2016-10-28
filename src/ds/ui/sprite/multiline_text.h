#pragma once
#ifndef DS_UI_SPRITE_MULTILINETEXT_H
#define DS_UI_SPRITE_MULTILINETEXT_H

#include "ds/ui/sprite/text.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::MultilineText
 * A Text sprite specialized for multiline drawing. This is really nothing
 * but a text sprite with a multiline layout bundled in it, but it's such a
 * common case that it makes sense to provide it.
 */
class MultilineText : public Text
{
public:
	MultilineText(SpriteEngine& e) : Text(e){};
	/*
	~MultilineText();

	// Adjust the font leading value, where 0 = no space between lines,
	// and 1 = the default leading.
	float								getLeading() const;
	MultilineText&						setLeading(const float);

	Alignment::Enum						getAlignment() const;
	MultilineText&						setAlignment(const Alignment::Enum&);

	virtual float						getFontFullHeight() const;

private:
	typedef Text						inherited;

	ds::ui::TextLayoutVertical			mMultilineLayout;
	*/
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TEXT_H
