#ifndef DS_UI_SPRITE_VIDEO_H_
#define DS_UI_SPRITE_VIDEO_H_

#include <ds/ui/sprite/gst_video.h>

namespace ds {
namespace ui {

/*!
 * \typedef GstVideo Video
 *
 * \brief A convenience for clients who wish to use ONLY GStreamer for
 * their video requirements.
 *
 * \note If you are planning to use more than one video implementation
 * make sure you include individual headers.
 */
typedef GstVideo Video;

} // namespace ui
} // namespace ds

#endif //!DS_UI_SPRITE_VIDEO_H_