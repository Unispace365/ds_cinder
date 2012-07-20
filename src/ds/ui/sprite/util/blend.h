#pragma once
#ifndef DS_UI_BLEND_H
#define DS_UI_BLEND_H

namespace ds {
namespace ui {

enum BlendMode
{
  NORMAL,
  MULTIPLY,
  SCREEN,
  ADD,
  SUBTRACT
};

bool premultiplyAlpha(const BlendMode &blendMode);
void applyBlendingMode(const BlendMode &blendMode);

}
}

#endif//DS_UI_BLEND_H
