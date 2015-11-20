#include "gl_scoped.h"

#include "gl_state.h"
#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace gl {

/**
 * @class ds::gl::ScopedClipPlane
 */
ScopedClipPlane::ScopedClipPlane(ClipPlaneState &state, ds::ui::Sprite &s)
		: mState(state) {
	if (s.getClipping()) {
		mState.push(s);
		mPushed = true;
	}
}

ScopedClipPlane::~ScopedClipPlane() {
	if (mPushed) {
		mState.pop();
	}
}

} // namespace gl
} // namespace ds
