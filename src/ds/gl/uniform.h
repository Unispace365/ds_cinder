#pragma once
#ifndef DS_GL_UNIFORM_H_
#define DS_GL_UNIFORM_H_

#include <map>
#include <string>
#include <cinder/Matrix22.h>
#include <cinder/Matrix33.h>
#include <cinder/MatrixAffine2.h>
#include <cinder/Matrix44.h>
#include <cinder/Vector.h>
#include <cinder/gl/GlslProg.h>

namespace ds {
namespace gl {

/**
 * \class ds::gl::Uniform
 * Storage (and eventually network transport) for uniform data.
 * It's been overly complicated because people started making use
 * of uniforms before there was support in the engine, but in an ideal
 * world, they would only support vectors of floats.
 */
class Uniform {
public:
	Uniform();

	void									setMatrix44f(const std::string& name, const ci::Matrix44f&);
	void									setVec2i(const std::string& name, const ci::Vec2i&);
	void									setVec4f(const std::string& name, const ci::Vec4f&);

	void									applyTo(ci::gl::GlslProg&) const;

private:
	std::map<std::string, ci::Matrix44f>	mMatrix44f;
	std::map<std::string, ci::Vec2i>		mVec2i;
	std::map<std::string, ci::Vec4f>		mVec4f;
};

} // namespace gl
} // namespace ds

#endif // DS_GL_UNIFORM_H_
