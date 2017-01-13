#include "clip_plane.h"
#include <cinder/gl/gl.h>
#include <cinder/Vector.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

namespace {

bool					sClippingIsEnabled = false;
std::vector<glm::mat4>	sClipPlaneStack = {	glm::mat4(
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
	) };

void pushClipPlaneStack() {
	sClipPlaneStack.push_back( sClipPlaneStack.back() );
}

void popClipPlaneStack() {
	sClipPlaneStack.pop_back();
}

}

namespace ds {
namespace ui {
namespace clip_plane {

void enableClipping(float x0, float y0, float x1, float y1)
{
	//glPushAttrib( GL_TRANSFORM_BIT | GL_ENABLE_BIT );
	//glEnable(GL_SCISSOR_TEST);
	//glScissor(x0, y0, x1, y1);
	glm::vec3 clippingPoints[4];
	clippingPoints[0] = glm::vec3(x0, y0, 0);
	clippingPoints[1] = glm::vec3(x0, y1, 0);
	clippingPoints[2] = glm::vec3(x1, y1, 0);
	clippingPoints[3] = glm::vec3(x1, y0, 0);
	pushClipPlaneStack();

	for(int i = 0; i < 4; ++i) {
		int j = (i + 1) % 4;
		int k = (i + 2) % 4;

		// Going clockwise around clipping points...
		glm::vec3 edgeA = clippingPoints[i] - clippingPoints[j];
		glm::vec3 edgeB = clippingPoints[j] - clippingPoints[k];

		// The edge-normal is found by first finding a vector perpendicular
		// to two consecutive edges.  Next, we cross that with the forward-
		// facing (clockwise) edge vector to get an inward-facing edge-
		// normal vector for that edge
		glm::vec3 norm = glm::normalize(glm::cross(edgeA, (glm::cross(edgeA, edgeB))));

		// the four points we pass to glClipPlane are the solutions of the
		// equation Ax + By + Cz + D = 0.  A, B, and C are the normal, and
		// we solve for D. C is always zero for the 2D case however, in the
		// 3D case, we must use a three-component normal vector.
		float d = glm::dot(-norm, clippingPoints[i]);

		glm::vec4 plane(norm.x, norm.y, norm.z, d);

		sClipPlaneStack.back()[i] = plane;

		ci::gl::enable( GL_CLIP_DISTANCE0 + i );
		DS_REPORT_GL_ERRORS();
	}
	sClippingIsEnabled = true;
}

void disableClipping() {
	if (sClipPlaneStack.size() <= 1) {
		DS_LOG_WARNING("Clip Plane: Trying to set invalid clipping state!");
		return;
	}
	popClipPlaneStack();

	if (sClipPlaneStack.size() == 1) {
		sClippingIsEnabled = false;
		for (int i=0; i<4; i++) {
			ci::gl::disable(GL_CLIP_DISTANCE0 + i);
		}
	}
}

void passClipPlanesToShader(ci::gl::GlslProgRef shaderProg) {
	shaderProg->uniform("uClipPlane0", sClipPlaneStack.back()[0]);
	shaderProg->uniform("uClipPlane1", sClipPlaneStack.back()[1]);
	shaderProg->uniform("uClipPlane2", sClipPlaneStack.back()[2]);
	shaderProg->uniform("uClipPlane3", sClipPlaneStack.back()[3]);
}

} // namespace clip_plane
} // namespace ui
} // namespace ds
