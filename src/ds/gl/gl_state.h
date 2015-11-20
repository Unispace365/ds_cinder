#pragma once
#ifndef DS_GL_GLSTATE_H_
#define DS_GL_GLSTATE_H_

#include <cstdint>
#include <vector>

namespace ds {
namespace ui { class Sprite; }
namespace gl {

/**
 * @class ds::gl::ClipPlaneState
 * @brief A stack of the current clip plane state.
 */
class ClipPlaneState {
public:
	ClipPlaneState() { }

	void				push(ds::ui::Sprite&);
	void				pop();

private:
	void				enableClipping(ds::ui::Sprite&);
	size_t				getDepth() const;

	// We allow for non-clipped sprites to be pushed, even though
	// they shouldn't. It's a safer API for clients, it just complicates
	// me a little.
	std::vector<bool>	mEnabled;
};

} // namespace gl
} // namespace ds

#endif
