#pragma once
#ifndef DS_GL_GLSCOPED_H_
#define DS_GL_GLSCOPED_H_

namespace ds {
namespace ui { class Sprite; }
namespace gl {
class ClipPlaneState;

/**
 * @class ds::gl::ScopedClipPlane
 * @brief Push and pop the current clip plane state.
 */
class ScopedClipPlane {
public:
	ScopedClipPlane(ClipPlaneState&, ds::ui::Sprite&);
	~ScopedClipPlane();

private:
	ClipPlaneState&		mState;
	bool				mPushed = false;
};

} // namespace gl
} // namespace ds

#endif
