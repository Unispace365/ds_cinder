#include "ds/gl/uniform.h"
#include <cinder/gl/GlslProg.h>

namespace ds {
namespace gl {

namespace {
static std::string EMPTY_SZ("");
static UniformData EMPTY_DATA;
} // !anonymous namespace

UniformData::UniformData(int count /*= 0*/, bool transpose /*= false*/)
	: mCount(count)
	, mTranspose(transpose)
{}

bool UniformData::operator==(const UniformData& rhs) const
{
	return rhs.mTranspose == mTranspose && rhs.mCount == mCount;
}

UniformVisitor::UniformVisitor(ci::gl::GlslProg& shader)
	: mShader(shader)
	, mData(EMPTY_DATA)
	, mName(EMPTY_SZ)
{}

void UniformVisitor::operator()(int data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Vec2i &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const int *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(const ci::Vec2i *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(float data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Vec2f &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Vec3f &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Vec4f &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Color &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::ColorA &data)
{
	mShader.uniform(mName, data);
}

void UniformVisitor::operator()(const ci::Matrix22f &data)
{
	mShader.uniform(mName, data, mData.mTranspose);
}

void UniformVisitor::operator()(const ci::Matrix33f &data)
{
	mShader.uniform(mName, data, mData.mTranspose);
}

void UniformVisitor::operator()(const ci::Matrix44f &data)
{
	mShader.uniform(mName, data, mData.mTranspose);
}

void UniformVisitor::operator()(const float *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(const ci::Vec2f *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(const ci::Vec3f *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(const ci::Vec4f *data)
{
	mShader.uniform(mName, data, mData.mCount);
}

void UniformVisitor::operator()(const ci::Matrix22f *data)
{
	mShader.uniform(mName, data, mData.mCount, mData.mTranspose);
}

void UniformVisitor::operator()(const ci::Matrix33f *data)
{
	mShader.uniform(mName, data, mData.mCount, mData.mTranspose);
}

void UniformVisitor::operator()(const ci::Matrix44f *data)
{
	mShader.uniform(mName, data, mData.mCount, mData.mTranspose);
}

/**
 * \class ds::gl::Uniform
 */
Uniform::Uniform()
	: mIsEmpty(true)
	, mEmptyDirty(true)
{}

bool Uniform::operator==(const Uniform& o) const {
	if (mVariantUniforms != o.mVariantUniforms) return false;
	return true;
}

bool Uniform::empty() const {
	if (mEmptyDirty) {
		mIsEmpty = true;
		if (!mVariantUniforms.empty()) return false;
		mEmptyDirty = false;
	}
	return mIsEmpty;
}

void Uniform::clear() {
	mVariantUniforms.clear();
	mIsEmpty = true;
	mEmptyDirty = false;
}

void Uniform::setFloat(const std::string& key, const float value) {
	if (key.empty()) return;
	set(key, value);
}

void Uniform::setFloats(const std::string& key, const std::vector<float>& value) {
	if (key.empty()) return;
	setInternal(key, UniformVisitor::SupportedVariants((float*)value.data()), UniformData(value.size()));
}

void Uniform::setInt(const std::string& key, const int value) {
	if (key.empty()) return;
	set(key, value);
}

void Uniform::setMatrix44f(const std::string& key, const ci::Matrix44f& value) {
	if (key.empty()) return;
	set(key, value);
}

void Uniform::setVec2i(const std::string& key, const ci::Vec2i& value) {
	if (key.empty()) return;
	set(key, value);
}

void Uniform::setVec4f(const std::string& key, const ci::Vec4f& value) {
	if (key.empty()) return;
	set(key, value);
}

void Uniform::applyTo(ci::gl::GlslProg& shader) const {
	if (!mVariantUniforms.empty())
	{
		// constructing this is really cheap. so no worries.
		auto local_visitor = UniformVisitor(shader);

		for (auto it = mVariantUniforms.cbegin(), end = mVariantUniforms.cend(); it != end; ++it)
		{
			local_visitor.mData = it->second.second;
			local_visitor.mName = it->first;
			boost::apply_visitor(local_visitor, it->second.first);
		}
	}
}

void Uniform::setInternal(const std::string& key, const UniformVisitor::SupportedVariants& val, const UniformData& data)
{
	if (key.empty()) return;
	mVariantUniforms[key] = std::make_pair(val, data);
	mEmptyDirty = true;
}

// set overloads

void Uniform::set(const std::string &name, int data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Vec2i &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const int *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, const ci::Vec2i *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, float data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Vec2f &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Vec3f &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Vec4f &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Color &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::ColorA &data)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData());
}

void Uniform::set(const std::string &name, const ci::Matrix22f &data, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(0, transpose));
}

void Uniform::set(const std::string &name, const ci::Matrix33f &data, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(0, transpose));
}

void Uniform::set(const std::string &name, const ci::Matrix44f &data, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(0, transpose));
}

void Uniform::set(const std::string &name, const float *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, const ci::Vec2f *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, const ci::Vec3f *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, const ci::Vec4f *data, int count)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count));
}

void Uniform::set(const std::string &name, const ci::Matrix22f *data, int count, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count, transpose));
}

void Uniform::set(const std::string &name, const ci::Matrix33f *data, int count, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count, transpose));
}

void Uniform::set(const std::string &name, const ci::Matrix44f *data, int count, bool transpose /*= false*/)
{
	setInternal(name, UniformVisitor::SupportedVariants(data), UniformData(count, transpose));
}

} // namespace gl
} // namespace ds
