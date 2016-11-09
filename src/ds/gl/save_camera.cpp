#include "ds/gl/save_camera.h"

#include <cinder/gl/gl.h>

namespace ds {
namespace gl {

/**
 * \class ds::gl::SaveCamera
 */
SaveCamera::SaveCamera()
	: mViewport(ci::Area(ci::gl::getViewport().first, ci::gl::getViewport().second))
	, mModel(ci::gl::getModelMatrix())
	, mView(ci::gl::getViewMatrix())
	, mProjection(ci::gl::getProjectionMatrix()) {
}

SaveCamera::~SaveCamera() {
	ci::gl::viewport(mViewport.getUL(), mViewport.getSize());

	ci::gl::setProjectionMatrix(mProjection);
	ci::gl::setModelMatrix(mModel);
	ci::gl::setViewMatrix(mView);
}

} // namespace gl
} // namespace ds
