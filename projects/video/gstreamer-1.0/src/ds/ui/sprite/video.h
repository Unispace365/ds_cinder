#pragma once
#ifndef DS_UI_SPRITE_VIDEO_H_
#define DS_UI_SPRITE_VIDEO_H_

#include <ds/ui/sprite/gst_video.h>

namespace ds {
namespace ui {

/**
 * A convenience for apps that have a single playback system and want to
 * use a generic class that might be replaced at some point.
 */
typedef GstVideo Video;

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_VIDEO_H_
