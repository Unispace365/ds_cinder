#pragma once
#ifndef DS_GL_SAVECAMERA_H_
#define DS_GL_SAVECAMERA_H_

#include <cinder/Area.h>
#include <cinder/gl/gl.h>

namespace ds {
namespace gl {

/**
 * \class ds::gl::SaveCamera
 * \brief Retain and restore the camera and viewport state.
 */
class SaveCamera {
public:
	SaveCamera();
	~SaveCamera();

private:
	const ci::Area		mViewport;
	const ci::mat4		mModelView,
						mProjection;
};

} // namespace gl
} // namespace ds

#endif // DS_GL_UNIFORM_H_
