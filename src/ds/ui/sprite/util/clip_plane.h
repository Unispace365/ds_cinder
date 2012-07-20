#pragma once
#ifndef DS_UI_CLIP_PLANE_H
#define DS_UI_CLIP_PLANE_H

namespace ds {
namespace ui {

void enableClipping(float x0, float y0, float x1, float y1);
void disableClipping();

}
}

#endif//DS_UI_CLIP_PLANE_H
