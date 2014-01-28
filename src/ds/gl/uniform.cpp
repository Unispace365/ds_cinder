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

template<typename T>
void apply_vec(const std::map<std::string, T>& data, ci::gl::GlslProg& shader) {
	if (!data.empty()) {
		for (auto it=data.begin(), end=data.end(); it!=end; ++it) {
			if (it->second.size() > 0) {
				shader.uniform(it->first, &(it->second.front()), it->second.size());
			}
		}
	}
}

}

/**
 * \class ds::gl::Uniform
 */
Uniform::Uniform()
		: mIsEmpty(true)
		, mEmptyDirty(true) {
}

bool Uniform::empty() {
	if (mEmptyDirty) {
		mIsEmpty = true;
		if (!mFloat.empty()) mIsEmpty = false;
		else if (!mFloats.empty()) mIsEmpty = false;
		else if (!mInt.empty()) mIsEmpty = false;
		else if (!mMatrix44f.empty()) mIsEmpty = false;
		else if (!mVec2i.empty()) mIsEmpty = false;
		else if (!mVec4f.empty()) mIsEmpty = false;
		mEmptyDirty = false;
	}
	return mIsEmpty;
}

void Uniform::setFloat(const std::string& key, const float value) {
	if (key.empty()) return;
	mFloat[key] = value;
	mEmptyDirty = true;
}

void Uniform::setFloats(const std::string& key, const std::vector<float>& value) {
	if (key.empty()) return;
	mFloats[key] = value;
	mEmptyDirty = true;
	mEmptyDirty = true;
}

void Uniform::setInt(const std::string& key, const int value) {
	if (key.empty()) return;
	mInt[key] = value;
	mEmptyDirty = true;
}

void Uniform::setMatrix44f(const std::string& key, const ci::Matrix44f& value) {
	if (key.empty()) return;
	mMatrix44f[key] = value;
	mEmptyDirty = true;
}

void Uniform::setVec2i(const std::string& key, const ci::Vec2i& value) {
	if (key.empty()) return;
	mVec2i[key] = value;
	mEmptyDirty = true;
}

void Uniform::setVec4f(const std::string& key, const ci::Vec4f& value) {
	if (key.empty()) return;
	mVec4f[key] = value;
	mEmptyDirty = true;
}

void Uniform::applyTo(ci::gl::GlslProg& shader) const {
	apply<float>(mFloat, shader);
	apply_vec<std::vector<float>>(mFloats, shader);
	apply<int>(mInt, shader);
	apply<ci::Matrix44f>(mMatrix44f, shader);
	apply<ci::Vec2i>(mVec2i, shader);
	apply<ci::Vec4f>(mVec4f, shader);
}

} // namespace gl
} // namespace ds
