#include "ds/gl/save_camera.h"

#include <cinder/gl/gl.h>

namespace ds {
namespace gl {

/**
 * \class ds::gl::SaveCamera
 */
SaveCamera::SaveCamera()
		: mViewport(ci::gl::getViewport())
		, mModelView(ci::gl::getModelView())
		, mProjection(ci::gl::getProjection()) {
}

SaveCamera::~SaveCamera() {
	ci::gl::setViewport(mViewport);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(mProjection.m);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(mModelView.m);
}

} // namespace gl
} // namespace ds
