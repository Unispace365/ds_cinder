#pragma once
#ifndef DS_UI_SPRITE_MULTILINETEXT_H
#define DS_UI_SPRITE_MULTILINETEXT_H

#include "ds/ui/sprite/text.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::MultilineText
 * Extends Text, which extends TextPango. See text_pango.h for API usage.
 */
class MultilineText : public Text
{
public:
	MultilineText(SpriteEngine& e) : Text(e){};
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TEXT_H
