#pragma once
#ifndef DS_UI_BLEND_H
#define DS_UI_BLEND_H

#include <string>

namespace ds {
namespace ui {

enum BlendMode
{
  NORMAL,
  MULTIPLY,
  SCREEN,
  ADD,
  SUBTRACT,
  LIGHTEN,
  DARKEN,
  TRANSPARENT_BLACK
};

BlendMode getBlendModeByString(const std::string& blendString);
const std::string getStringForBlendMode(const BlendMode& blendMode);

bool premultiplyAlpha(const BlendMode &blendMode);
void applyBlendingMode(const BlendMode &blendMode);

}
}

#endif//DS_UI_BLEND_H
