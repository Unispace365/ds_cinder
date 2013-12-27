#include "ds/gl/uniform.h"

namespace ds {
namespace gl {

namespace {

template<typename T>
void apply(const std::map<std::string, T>& data, ci::gl::GlslProg& shader) {
	if (!data.empty()) {
		for (auto it=data.begin(), end=data.end(); it!=end; ++it) {
			shader.uniform(it->first, it->second);
		}
	}
}

}

/**
 * \class ds::gl::Uniform
 */
Uniform::Uniform() {
}

void Uniform::setMatrix44f(const std::string& key, const ci::Matrix44f& value) {
	if (key.empty()) return;
	mMatrix44f[key] = value;
}

void Uniform::setVec2i(const std::string& key, const ci::Vec2i& value) {
	if (key.empty()) return;
	mVec2i[key] = value;
}

void Uniform::setVec4f(const std::string& key, const ci::Vec4f& value) {
	if (key.empty()) return;
	mVec4f[key] = value;
}

void Uniform::applyTo(ci::gl::GlslProg& shader) const {
	apply<ci::Matrix44f>(mMatrix44f, shader);
	apply<ci::Vec2i>(mVec2i, shader);
	apply<ci::Vec4f>(mVec4f, shader);
}

} // namespace gl
} // namespace ds
