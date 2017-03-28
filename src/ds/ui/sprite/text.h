#pragma once
#ifndef DS_UI_TEXT_H
#define DS_UI_TEXT_H

#include "ds/ui/sprite/text_pango.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::Text
 * This is here for compatibility. All text is TextPango now. See that header for API.
 */
class Text : public TextPango
{
public:
	Text(SpriteEngine& e) : TextPango(e){}
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TEXT_H
