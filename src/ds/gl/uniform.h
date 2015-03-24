#pragma once
#ifndef DS_GL_UNIFORM_H_
#define DS_GL_UNIFORM_H_

#include <map>
#include <string>

#include <cinder/Matrix.h>
#include <cinder/Vector.h>
#include <cinder/Color.h>

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

namespace cinder {
namespace gl {
class GlslProg;
}} // !namespace ci::gl

namespace ds {
namespace gl {

/*!
 * \class UniformData
 * a placeholder for (extra) informations about a uniform.
 * \note transpose will be used only for GlslProg::uniform overloads that accept it. (matrices)
 * \note count will be used only for GlslProg::uniform overloads that accept it. (arrays)
 */
struct UniformData
{
	UniformData(int count = 0, bool transpose = false);
	bool			operator==(const UniformData& rhs) const;
	bool			mTranspose{ false }; // Will be used for MatXX Cinder uniforms only
	int				mCount{ 0 }; // Will be used for array types.
};

/*!
 * \class ds::gl::UniformVisitor
 * \note This is a static visitor for common types that can be passed
 * as uniforms to a shader.
 * Use as: boost::apply_visitor( my_visitor, variant_val );
 * in which my_visitor is an instance of UniformVisitor and
 * variant_val is an instance of boost::variant.
 * \see http://www.boost.org/doc/libs/1_57_0/doc/html/variant.html
 * \see <cinder/gl/GlslProg.h>. All overloads, match Cinder's uniform helper.
 */
class UniformVisitor final : public boost::static_visitor < void >
{
public:
	// A typedef of all (20) Cinder supported uniforms.
	typedef boost::variant <
		int, float, const int*, const float*,
		ci::Vec2f, ci::Vec2i, ci::Vec3f, ci::Vec4f,
		const ci::Vec2f*, const ci::Vec2i*, const ci::Vec3f*, const ci::Vec4f*,
		ci::Matrix22f, ci::Matrix33f, ci::Matrix44f,
		const ci::Matrix22f*, const ci::Matrix33f*, const ci::Matrix44f*,
		ci::Color, ci::ColorA > SupportedVariants;

public:
	UniformVisitor() = delete;
	UniformVisitor(ci::gl::GlslProg& shader);

	// Below all are uniform types, supported by Cinder.
	void					operator()(int data);
	void					operator()(const ci::Vec2i &data);
	void					operator()(const int *data);
	void					operator()(const ci::Vec2i *data);
	void					operator()(float data);
	void					operator()(const ci::Vec2f &data);
	void					operator()(const ci::Vec3f &data);
	void					operator()(const ci::Vec4f &data);
	void					operator()(const ci::Color &data);
	void					operator()(const ci::ColorA &data);
	void					operator()(const ci::Matrix22f &data);
	void					operator()(const ci::Matrix33f &data);
	void					operator()(const ci::Matrix44f &data);
	void					operator()(const float *data);
	void					operator()(const ci::Vec2f *data);
	void					operator()(const ci::Vec3f *data);
	void					operator()(const ci::Vec4f *data);
	void					operator()(const ci::Matrix22f *data);
	void					operator()(const ci::Matrix33f *data);
	void					operator()(const ci::Matrix44f *data);

private:
	ci::gl::GlslProg&		mShader; //shader that will receive the passed variant

public:
	UniformData&			mData; //just a placeholder
	std::string&			mName; //just a placeholder
};

/**
 * \class ds::gl::Uniform
 * Storage for GLSL uniform data. Will be used for network-replication.
 * \note ARRAYS ARE OF-COURSE NOT SENT DOWN OVER A NETWORK! Use custom
 * networking methods and send only hints down the network to what to
 * do with Uniform arrays on client side. don't send the entire array.
 */
class Uniform {
public:
	Uniform();

	/*!
	 * \name set
	 * \arg name: A string representing the uniform in shader.
	 * \arg value: A value to set for the uniform.
	 * \arg count: Count of objects passed to shader (if using an array of uniforms)
	 * \arg transpose: transpose matrix passed to shader (if using ci::MatXX classes to pass to shader)
	 */
	void			set(const std::string &name, int data);
	void			set(const std::string &name, const ci::Vec2i &data);
	void			set(const std::string &name, const int *data, int count);
	void			set(const std::string &name, const ci::Vec2i *data, int count);
	void			set(const std::string &name, float data);
	void			set(const std::string &name, const ci::Vec2f &data);
	void			set(const std::string &name, const ci::Vec3f &data);
	void			set(const std::string &name, const ci::Vec4f &data);
	void			set(const std::string &name, const ci::Color &data);
	void			set(const std::string &name, const ci::ColorA &data);
	void			set(const std::string &name, const ci::Matrix22f &data, bool transpose = false);
	void			set(const std::string &name, const ci::Matrix33f &data, bool transpose = false);
	void			set(const std::string &name, const ci::Matrix44f &data, bool transpose = false);
	void			set(const std::string &name, const float *data, int count);
	void			set(const std::string &name, const ci::Vec2f *data, int count);
	void			set(const std::string &name, const ci::Vec3f *data, int count);
	void			set(const std::string &name, const ci::Vec4f *data, int count);
	void			set(const std::string &name, const ci::Matrix22f *data, int count, bool transpose = false);
	void			set(const std::string &name, const ci::Matrix33f *data, int count, bool transpose = false);
	void			set(const std::string &name, const ci::Matrix44f *data, int count, bool transpose = false);

	bool			operator==(const Uniform&) const;
	bool			empty() const;
	void			clear();

	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setFloat(const std::string& name, const float);
	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setFloats(const std::string& name, const std::vector<float>&);
	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setInt(const std::string& name, const int);
	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setMatrix44f(const std::string& name, const ci::Matrix44f&);
	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setVec2i(const std::string& name, const ci::Vec2i&);
	// DEPRECATED LEGACY API, kept here for backward compatibility. use Uniform::set
	void			setVec4f(const std::string& name, const ci::Vec4f&);

	void			applyTo(ci::gl::GlslProg&) const;

private:
	// Internally handles inserting boost::variant's into a map for shaders to consume.
	void			setInternal(const std::string&, const UniformVisitor::SupportedVariants&, const UniformData&);

private:
	std::map < std::string, std::pair < UniformVisitor::SupportedVariants, UniformData > >
					mVariantUniforms;
	mutable bool	mIsEmpty;
	mutable bool	mEmptyDirty;
};

} // namespace gl
} // namespace ds

#endif // DS_GL_UNIFORM_H_
