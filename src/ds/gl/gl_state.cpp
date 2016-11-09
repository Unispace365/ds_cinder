#include "gl_state.h"

#include <cinder/gl/gl.h>
#include <ds/debug/debug_defines.h>
#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace gl {

namespace {
const int			CLIP_PLANE_COUNT = 4;
}

/**
 * @class ds::gl::ClipPlaneState
 */
void ClipPlaneState::push(ds::ui::Sprite &s) {
	try {
		if (!s.getClipping()) {
			mEnabled.push_back(false);
		} else {
			mEnabled.push_back(true);
			enableClipping(s);
		}
	} catch (std::exception const&) {
		DS_ASSERT_MSG(false, "ClipPlaneState push() error");
	}
}

void ClipPlaneState::pop() {
	if (mEnabled.empty()) {
		DS_ASSERT_MSG(false, "ClipPlaneState unbalanced pop()");
		return;
	}
	const bool			is_enabled = (mEnabled.back() == true);
	if (is_enabled) {
		DS_REPORT_GL_ERRORS();
		glPopAttrib();
		DS_REPORT_GL_ERRORS();
	}
	mEnabled.pop_back();
	if (is_enabled && getDepth() < 1) {
		DS_REPORT_GL_ERRORS();
		for (int i = 0; i < CLIP_PLANE_COUNT; ++i) {
			glDisable(GL_CLIP_PLANE0 + i);
		}
		DS_REPORT_GL_ERRORS();
	}
}

void ClipPlaneState::enableClipping(ds::ui::Sprite &s) {
	const ci::Rectf&		cb = s.getClippingBounds();
	const float				x0 = cb.getX1(),
							y0 = cb.getY1(),
							x1 = cb.getX2(),
							y1 = cb.getY2();

	glPushAttrib( GL_TRANSFORM_BIT | GL_ENABLE_BIT );
	ci::vec3				clippingPoints[4];
	clippingPoints[0].set( x0, y0, 0 );
	clippingPoints[1].set( x0, y1, 0 );
	clippingPoints[2].set( x1, y1, 0 );
	clippingPoints[3].set( x1, y0, 0 );

	for (int i = 0; i < CLIP_PLANE_COUNT; ++i) {
		int					j = (i+1) % 4,
							k = (i+2) % 4;

		// Going clockwise around clipping points...
		ci::vec3			edgeA = clippingPoints[i] - clippingPoints[j],
							edgeB = clippingPoints[j] - clippingPoints[k];

		// The edge-normal is found by first finding a vector perpendicular
		// to two consecutive edges.  Next, we cross that with the forward-
		// facing (clockwise) edge vector to get an inward-facing edge-
		// normal vector for that edge
		ci::vec3			norm = -(normalize(cross(cross( edgeA, edgeB ), edgeA)));

		// the four points we pass to glClipPlane are the solutions of the
		// equation Ax + By + Cz + D = 0.  A, B, and C are the normal, and
		// we solve for D. C is always zero for the 2D case however, in the
		// 3D case, we must use a three-component normal vector.
		float d = -norm.dot(clippingPoints[i]);

		DS_REPORT_GL_ERRORS();
		glEnable( GL_CLIP_PLANE0 + i );
		DS_REPORT_GL_ERRORS();
		GLdouble			equation[4] = { norm.x, norm.y, norm.z, d };
		glClipPlane( GL_CLIP_PLANE0 + i, equation );
		DS_REPORT_GL_ERRORS();
	}
}

size_t ClipPlaneState::getDepth() const {
	size_t		ans = 0;
	for (const auto& v : mEnabled) {
		if (v) ++ans;
	}
	return ans;
}

} // namespace gl
} // namespace ds
