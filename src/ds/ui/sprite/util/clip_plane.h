#pragma once
#ifndef DS_UI_CLIP_PLANE_H
#define DS_UI_CLIP_PLANE_H

#include <cinder/gl/GlslProg.h>

namespace ds {
namespace ui {
namespace clip_plane {

void enableClipping(float x0, float y0, float x1, float y1);
void disableClipping();
void passClipPlanesToShader(ci::gl::GlslProgRef shaderProg);

} // namespace clip_plane
} // namespace ui
} // namespace ds

#endif//DS_UI_CLIP_PLANE_H
