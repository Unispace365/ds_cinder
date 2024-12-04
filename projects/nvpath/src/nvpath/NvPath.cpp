/*
Copyright (c) 2023, Paul Houx Creative Coding - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable : 4715)
#pragma warning(disable : 4996)

#include "nvpath/NvPath.h"
#include "nvpath/NvPathSvg.h"
#include "nvpath/NvPathUtil.h"

#include <cinder/Log.h>
#include <cinder/Utilities.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/scoped.h>

#include <ds/util/float_util.h> // for approxEqual method.

using namespace ci;

namespace nvpath {

bool hasNvPathRendering() {
	assert(ci::gl::context()); // We must have an active OpenGL context first!
	return bool(GLAD_GL_NV_path_rendering);
}

void reportNoNvPathRendering() {
	static bool sReported = false;
	if (!sReported) {
		sReported = true;
		CI_LOG_F("This GPU does not support the NVidia Path Rendering extension!");
	}
}

bool isPreMultiplied() { // TODO: add this to Cinder.
	const auto ctx = gl::context();
	if (ctx) {
		GLenum src, dst, srcAlpha, dstAlpha;
		ctx->getBlendFuncSeparate(&src, &dst, &srcAlpha, &dstAlpha);
		return src == GL_ONE &&
			   dst == GL_ONE_MINUS_SRC_ALPHA; // && srcAlpha == GL_ONE && dstAlpha == GL_ONE_MINUS_SRC_ALPHA;
	}
	return false;
}

bool Paint::Stop::operator==(const Stop& other) const {
	return ds::approxEqual(offset, other.offset) && color == other.color;
}

bool Paint::Stop::operator==(float value) const {
	return ds::approxEqual(offset, value);
}

Paint Paint::color(const ColorA8u& color, const std::string& id) {
	Paint result{COLOR, id};
	result.setColor(color);
	return result;
}

Paint Paint::linear(const std::string& id) {
	return Paint{LINEAR_GRADIENT, id};
}

Paint Paint::radial(const std::string& id) {
	return Paint{RADIAL_GRADIENT, id};
}

Paint::Paint()
  : Paint(NONE) {}

Paint::Paint(Type type, std::string id)
  : mType(type)
  , mId(std::move(id)) {
	mStops.emplace_back(0.0f, ColorA8u::black());

	// Defaults for radial gradients are different.
	if (type == RADIAL_GRADIENT) {
		mCoords0 = mCoords1 = vec2(0.5f, 0.5f);

		mRadius0 = 0.5f;
		mRadius1 = 0.0f;
	}
}

Paint::Paint(const ColorA8u& color)
  : Paint(COLOR) {
	mStops.emplace_back(0.0f, color);
}

Paint::Paint(std::string url)
  : mNeedsResolve(true)
  , mId(std::move(url)) {
	// None of the other data members are initialized, because this instance will be replaced later.
}

bool Paint::isTransparent() const {
	for (const auto& [offset, color] : mStops)
		if (color.a < 1) return true;

	return false;
}

const ColorA8u& Paint::getColor() const {
	static ColorA8u sBlack{0, 0, 0, 255};
	static ColorA8u sTransparent{0, 0, 0, 0};

	switch (mType) {
	case NONE:
		return sTransparent;
	case COLOR:
		return getColor(0);
	default:
		if (mFallback) return mFallback->getColor();
		return sBlack;
	}
}

void Paint::setColor(const ColorA8u& color) {
	mStops = {{0, color}};
}

void Paint::set(float offset, const ColorA8u& color) {
	// Allow duplicates, see: https://svgwg.org/svg2-draft/pservers.html#GradientStops
	// "If two gradient stops have the same offset value, then the latter gradient stop controls the color value at
	// the overlap point."
	const auto t   = clamp(offset, 0.0f, 1.0f);
	const auto itr = std::upper_bound(mStops.begin(), mStops.end(), t, [](float a, const Stop& b) {
		return a < b.offset && !ds::approxEqual(a, b.offset);
	});
	mStops.insert(itr, Stop{offset, color});
}

ColorA8u Paint::at(float t, bool preMultiply) const {
	ColorA8u result;

	const auto& lo = floor(t);
	const auto& hi = ceil(t);

	// See: https://svgwg.org/svg2-draft/pservers.html#GradientStops
	// "If two gradient stops have the same offset value, then the latter gradient stop controls the color value at
	// the overlap point."
	if (ds::approxEqual(lo.offset, hi.offset)) {
		result = hi.color; // TODO: check specifiesColor?
	} else {
		const float f = clamp((t - lo.offset) / (hi.offset - lo.offset), 0.0f, 1.0f);
		result		  = lo.color.lerp(uint8_t(f * 255.0f), hi.color); // TODO: check specifiesColor?
	}

	return preMultiply ? result.premultiplied() : result;
}

const Paint::Stop& Paint::floor(float t) const {
	assert(!mStops.empty());

	for (auto itr = mStops.rbegin(); itr != mStops.rend(); ++itr) {
		if (itr->offset <= t) return *itr;
	}

	return mStops.front();
}

const Paint::Stop& Paint::ceil(float t) const {
	assert(!mStops.empty());

	for (auto itr = mStops.begin(); itr != mStops.end(); ++itr) {
		if (itr->offset > t) return *itr;
	}

	return mStops.back();
}

std::unique_ptr<uint8_t[]> Paint::data(int32_t width, int32_t height, bool preMultiply, float from, float to) const {
	auto result = std::make_unique<uint8_t[]>(size_t(width) * size_t(height) * sizeof(ColorA8u));

	for (int x = 0; x < width; ++x) {
		const float t = mix(from, to, float(x) / float(width - 1));

		const auto c = at(t, preMultiply);
		for (int y = 0; y < height; ++y) {
			const int64_t i		  = (int64_t(x) + int64_t(y) * int64_t(width)) * sizeof(ColorA8u);
			result[size_t(i) + 0] = uint8_t(c.r);
			result[size_t(i) + 1] = uint8_t(c.g);
			result[size_t(i) + 2] = uint8_t(c.b);
			result[size_t(i) + 3] = uint8_t(c.a);
		}
	}

	return result;
}

bool Paints::contains(const std::string& paintId) const {
	return mPaints.count(paintId) != 0;
}

bool Paints::contains(const Paint& paint) const {
	return mPaints.count(paint.getId()) != 0;
}

float Paints::index(const std::string& paintId) const {
	const auto itr = mPaints.find(paintId);
	if (itr == mPaints.end()) return 0.0f;
	const auto index = std::distance(mPaints.begin(), itr);
	const auto size	 = textureSize();
	return (static_cast<float>(index) + 0.5f) / static_cast<float>(size);
}

float Paints::index(const Paint& paint) const {
	return index(paint.getId());
}

float Paints::set(const Paint& paint) {
	mDirty |= mPaints.count(paint.getId()) == 0 || mPaints[paint.getId()] != paint;
	mPaints[paint.getId()] = paint;
	const auto index	   = std::distance(mPaints.begin(), mPaints.find(paint.getId()));
	const auto size		   = textureSize();
	return (static_cast<float>(index) + 0.5f) / static_cast<float>(size);
}

void Paints::setSpreadMethod(SpreadMethod method) const {
	if (mTexture) {
		mTexture->setWrapS(method == SpreadMethod::REFLECT	? GL_MIRRORED_REPEAT
						   : method == SpreadMethod::REPEAT ? GL_REPEAT
															: GL_CLAMP_TO_EDGE);
	}
}

Shader::Type Paints::prepareLinearGradient(const Paint& paint, float opacity, bool prepareShader) {
	assert(paint.isLinearGradient());

	const auto index = set(paint);
	setSpreadMethod(paint.getSpreadMethod());

	if (prepareShader) {
		ScopedShader scpShader(Shader::Type::LINEAR_GRADIENT);
		scpShader.setColor(paint.getColor());
		scpShader.setCoords(paint.useObjectBoundingBox() ? GL_PATH_OBJECT_BOUNDING_BOX_NV : GL_OBJECT_LINEAR_NV,
							paint.getTransform());
		scpShader.uniform("index", index);
		scpShader.uniform("gradTab", 0);
		scpShader.uniform("gradStart", paint.getCoords0());
		scpShader.uniform("gradEnd", paint.getCoords1());
		scpShader.uniform("opacity", opacity);
	}

	return Shader::Type::LINEAR_GRADIENT;
}

Shader::Type Paints::prepareRadialGradient(const Paint& paint, float opacity, bool prepareShader) {
	assert(paint.isRadialGradient());

	const float index = set(paint);
	setSpreadMethod(paint.getSpreadMethod());

	if (prepareShader) {
		ScopedShader scpShader(Shader::Type::RADIAL_GRADIENT);
		scpShader.setColor(paint.getColor());
		scpShader.setCoords(paint.useObjectBoundingBox() ? GL_PATH_OBJECT_BOUNDING_BOX_NV : GL_OBJECT_LINEAR_NV,
							paint.getTransform());
		scpShader.uniform("index", index);
		scpShader.uniform("gradTab", 0);
		scpShader.uniform("focalToCenter", paint.getCoords0() - paint.getCoords1());
		scpShader.uniform("centerRadius", paint.getRadius0());
		scpShader.uniform("focalRadius", paint.getRadius1());
		scpShader.uniform("translationPoint", paint.getCoords1());
		scpShader.uniform("opacity", opacity);
	}

	return Shader::Type::RADIAL_GRADIENT;
}

Shader::Type Paints::preparePaint(const Paint& paint, float opacity, bool prepareShader) {
	if (paint.isLinearGradient()) return prepareLinearGradient(paint, opacity, prepareShader);
	if (paint.isRadialGradient()) return prepareRadialGradient(paint, opacity, prepareShader);

	if (prepareShader) {
		ScopedShader scpShader(Shader::Type::SOLID_COLOR);
		scpShader.setColor(paint.getColor());
		scpShader.uniform("opacity", opacity);
	}

	return Shader::Type::SOLID_COLOR;
}

GLint Paints::textureSize() const {
	const GLint size = glm::max(128, static_cast<GLint>(mPaints.size()));
	return isPowerOf2(size) ? size : static_cast<GLint>(nextPowerOf2(size));
}

void Paints::bind(gl::Context* ctx, uint8_t textureUnit) {
	assert(ctx);

	mTextureUnit = textureUnit;

	if (const auto size = textureSize(); mDirty || !mTexture || mCtx != ctx || mTexture->getHeight() < size) {
		const auto data = std::make_unique<uint8_t[]>(128 * size * sizeof(ColorA8u));

		auto ptr = data.get();
		for (const auto& [id, paint] : mPaints) {
			for (int x = 0; x < 128; ++x) {
				const float t = static_cast<float>(x) / static_cast<float>(128 - 1);
				const auto	c = paint.at(t, false);
				*ptr++		  = static_cast<uint8_t>(c.r);
				*ptr++		  = static_cast<uint8_t>(c.g);
				*ptr++		  = static_cast<uint8_t>(c.b);
				*ptr++		  = static_cast<uint8_t>(c.a);
			}
		}

		static const gl::Texture2d::Format FORMAT =
			gl::Texture2d::Format().wrapT(GL_REPEAT).internalFormat(GL_RGBA).target(GL_TEXTURE_2D).loadTopDown();

		GLint wrapS = GL_CLAMP_TO_EDGE;
		if (mTexture) {
			gl::ScopedTextureBind tbs(mTexture);
			glGetTexParameteriv(mTexture->getTarget(), GL_TEXTURE_WRAP_S, &wrapS);
		}

		mCtx	 = ctx;
		mTexture = gl::Texture::create(128, size, FORMAT);
		mTexture->update(data.get(), GL_RGBA, GL_UNSIGNED_BYTE, 0, 128, size);
		mTexture->setWrapS(wrapS);
		mDirty = false;
	}

	if (mTexture) mCtx->pushTextureBinding(mTexture->getTarget(), mTexture->getId(), mTextureUnit);
}

void Paints::unbind(const gl::Context* ctx) const {
	assert(ctx == mCtx);

	if (mTexture) mCtx->popTextureBinding(mTexture->getTarget(), mTextureUnit);
}

Path::~Path() {
	if (mPathId > 0) glDeletePathsNV(mPathId, 1);
}

Path::Path(const Path& other) {
	if (other.mPathId > 0) {
		mPathId = glGenPathsNV(1);
		glCopyPathNV(mPathId, other.mPathId);
	}
}

Path::Path(Path&& other) noexcept {
	if (mPathId > 0) glDeletePathsNV(mPathId, 1);
	mPathId		  = other.mPathId;
	other.mPathId = 0;
}

Path& Path::operator=(const Path& other) {
	if (other.mPathId > 0 && this != &other) {
		if (mPathId == 0) mPathId = glGenPathsNV(1);
		glCopyPathNV(mPathId, other.mPathId);
	}
	return *this;
}

Path& Path::operator=(Path&& other) noexcept {
	if (this != &other) {
		if (mPathId > 0) glDeletePathsNV(mPathId, 1);
		mPathId		  = other.mPathId;
		other.mPathId = 0;
	}
	return *this;
}

Path::Path(const Path2d& path) {
	if (hasNvPathRendering()) {
		std::vector<GLubyte> commands;

		commands.reserve(path.getNumSegments() + 1 /* implicit MOVE_TO at start */);
		commands.push_back(GL_MOVE_TO_NV);

		for (size_t i = 0; i < path.getNumSegments(); ++i)
			commands.push_back(toPathCommand(path.getSegmentType(i)));

		const auto& points = path.getPoints();

		mPathId = glGenPathsNV(1);
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(points.size() * 2 /* each vec2 contains 2 floats */), GL_FLOAT,
						 points.data());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(ci::gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const Shape2d& shape) {
	if (hasNvPathRendering()) {
		std::vector<GLubyte> commands;
		std::vector<vec2>	 coords;

		const auto& contours = shape.getContours();
		for (const auto& contour : contours) {
			const auto& points = contour.getPoints();
			coords.insert(coords.end(), points.begin(), points.end());

			const auto numCommands = static_cast<GLsizei>(contour.getNumSegments());

			commands.reserve(commands.size() + numCommands + 1 /* implicit MOVE_TO at start */);
			commands.push_back(GL_MOVE_TO_NV);

			for (GLsizei i = 0; i < numCommands; ++i)
				commands.push_back(toPathCommand(contour.getSegmentType(i)));
		}

		mPathId = glGenPathsNV(1);
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(coords.size() * 2 /* each vec2 contains 2 floats */), GL_FLOAT,
						 coords.data());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const PolyLine2& polyLine) {
	if (hasNvPathRendering()) {
		const auto& points = polyLine.getPoints();

		std::vector<GLubyte> commands;

		const auto numCommands = points.size() + static_cast<size_t>(polyLine.isClosed());
		commands.reserve(numCommands);
		commands.push_back(GL_MOVE_TO_NV);

		for (size_t i = 0; i < numCommands; ++i)
			commands.push_back(GL_LINE_TO_NV);

		if (polyLine.isClosed()) commands.push_back(GL_CLOSE_PATH_NV);

		mPathId = glGenPathsNV(1);
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(points.size() * 2 /* each vec2 contains 2 floats */), GL_FLOAT,
						 points.data());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const std::string& path, PathFormat format) {
	if (hasNvPathRendering()) {
		mPathId = glGenPathsNV(1);
		glPathStringNV(mPathId, static_cast<GLenum>(format), static_cast<GLsizei>(path.length()), path.c_str());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const std::vector<GLubyte>& commands, const std::vector<vec2>& points) {
	if (hasNvPathRendering()) {
		mPathId = glGenPathsNV(1);
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(points.size() * 2 /* each vec2 contains 2 floats */), GL_FLOAT,
						 points.data());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const std::vector<GLubyte>& commands, const std::vector<GLfloat>& points) {
	if (hasNvPathRendering()) {
		mPathId = glGenPathsNV(1);
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(points.size()), GL_FLOAT, points.data());
		// if (const GLenum err = gl::getError(); err != GL_NO_ERROR) CI_LOG_E(gl::getErrorString(err));
	} else
		reportNoNvPathRendering();
}

Path::Path(const PathHelper& commands)
  : Path(commands.getCommands(), commands.getCoords()) {}

bool Path::getPointAlongPath(float distance, vec2& point, vec2& tangent) const {
	if (mPathId > 0) {
		return glPointAlongPathNV(mPathId, 0, getNumSegments(), distance, &point.x, &point.y, &tangent.x, &tangent.y);
	}
	return false;
}

float Path::getLength() const {
	float length{0};

	if (mPathId > 0) length = glGetPathLengthNV(mPathId, 0, getNumSegments());

	return length;
}

Rectf Path::getFillBounds() const {
	Rectf bounds;

	if (mPathId > 0) glGetPathParameterfvNV(mPathId, GL_PATH_FILL_BOUNDING_BOX_NV, reinterpret_cast<GLfloat*>(&bounds));

	return bounds;
}

Rectf Path::getStrokeBounds() const {
	Rectf bounds;

	if (mPathId > 0)
		glGetPathParameterfvNV(mPathId, GL_PATH_STROKE_BOUNDING_BOX_NV, reinterpret_cast<GLfloat*>(&bounds));

	return bounds;
}

float Path::getClientLength() const {
	float clientLength = 0;

	if (mPathId > 0) glGetPathParameterfvNV(mPathId, GL_PATH_CLIENT_LENGTH_NV, &clientLength);

	return clientLength;
}

void Path::setClientLength(float length) const {
	if (mPathId > 0) gl::pathParameterfNV(mPathId, GL_PATH_CLIENT_LENGTH_NV, glm::max(0.0f, length));
}

int Path::getNumSegments() const {
	int numSegments = 0;

	if (mPathId > 0) glGetPathParameterivNV(mPathId, GL_PATH_COMMAND_COUNT_NV, &numSegments);

	return numSegments;
}

void Path::resetDashPattern() const {
	if (mPathId > 0) {
		glPathDashArrayNV(mPathId, 0, nullptr);
		gl::pathParameteriNV(mPathId, GL_PATH_DASH_CAPS_NV, static_cast<GLint>(CapsStyle::FLAT));
		gl::pathParameteriNV(mPathId, GL_PATH_INITIAL_DASH_CAP_NV, static_cast<GLint>(CapsStyle::FLAT));
		gl::pathParameteriNV(mPathId, GL_PATH_TERMINAL_DASH_CAP_NV, static_cast<GLint>(CapsStyle::FLAT));
	}
}

void Path::setDashPattern(const std::vector<float>& pattern) const {
	if (mPathId > 0) {
		glPathDashArrayNV(mPathId, static_cast<GLsizei>(pattern.size()), pattern.data());
	}
}

void Path::setDashOffset(float offset, PathStyle style) const {
	if (mPathId > 0) {
		gl::pathParameterfNV(mPathId, GL_PATH_DASH_OFFSET_NV, offset);
		gl::pathParameteriNV(mPathId, GL_PATH_DASH_OFFSET_RESET_NV, static_cast<GLint>(style));
	}
}

void Path::setDashCaps(CapsStyle caps) const {
	setDashCaps(caps, caps);
}

void Path::setDashCaps(CapsStyle initialCap, CapsStyle terminalCap) const {
	if (mPathId > 0) {
		gl::pathParameteriNV(mPathId, GL_PATH_INITIAL_DASH_CAP_NV, static_cast<GLint>(initialCap));
		gl::pathParameteriNV(mPathId, GL_PATH_TERMINAL_DASH_CAP_NV, static_cast<GLint>(terminalCap));
	}
}

void Path::setEndCaps(CapsStyle caps) const {
	setEndCaps(caps, caps);
}

void Path::setEndCaps(CapsStyle initialCap, CapsStyle terminalCap) const {
	if (mPathId > 0) {
		gl::pathParameteriNV(mPathId, GL_PATH_INITIAL_END_CAP_NV, static_cast<GLint>(initialCap));
		gl::pathParameteriNV(mPathId, GL_PATH_TERMINAL_END_CAP_NV, static_cast<GLint>(terminalCap));
	}
}

void Path::setJoinStyle(JoinStyle joins) const {
	if (mPathId > 0) {
		gl::pathParameteriNV(mPathId, GL_PATH_JOIN_STYLE_NV, static_cast<GLint>(joins));
	}
}

void Path::setStrokeWidth(float width) const {
	if (mPathId > 0) {
		gl::pathParameterfNV(mPathId, GL_PATH_STROKE_WIDTH_NV, static_cast<GLfloat>(width));
	}
}

void Path::setMiterLimit(float limit) const {
	if (mPathId > 0) {
		gl::pathParameterfNV(mPathId, GL_PATH_MITER_LIMIT_NV, static_cast<GLfloat>(limit));
	}
}

void Path::setStroke(CapsStyle caps, JoinStyle join, float strokeWidth) const {
	if (mPathId > 0) {
		gl::pathParameterfNV(mPathId, GL_PATH_STROKE_WIDTH_NV, strokeWidth);
		gl::pathParameteriNV(mPathId, GL_PATH_END_CAPS_NV, static_cast<GLint>(caps));
		gl::pathParameteriNV(mPathId, GL_PATH_JOIN_STYLE_NV, static_cast<GLint>(join));
	}
}

void Path::stencilStroke(GLuint stencilMask) const {
	if (mPathId > 0) {
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilStrokePathNV(mPathId, 0x1, stencilMask);
	}
}

void Path::stencilFill(GLuint stencilMask, GLenum fillMode) const {
	if (mPathId > 0) {
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilFillPathNV(mPathId, fillMode, stencilMask);
	}
}

void Path::stencilStrokeInstanced(const std::vector<glm::mat3x2>& transforms, GLuint stencilMask) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		stencilStrokeInstanced(sPaths, transforms, stencilMask);
	}
}

void Path::stencilStrokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
								  GLuint stencilMask) const {
	if (mPathId > 0) {
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilStrokePathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(), mPathId,
									   0x1, stencilMask, GL_AFFINE_2D_NV,
									   reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::stencilFillInstanced(const std::vector<glm::mat3x2>& transforms, GLuint stencilMask, GLenum fillMode) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		stencilFillInstanced(sPaths, transforms, stencilMask, fillMode);
	}
}

void Path::stencilFillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
								GLuint stencilMask, GLenum fillMode) const {
	if (mPathId > 0) {
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilFillPathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(), mPathId,
									 fillMode, stencilMask, GL_AFFINE_2D_NV,
									 reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::clearStroke(GLuint stencilMask) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for clearStroke().");
			return;
		}

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, stencilMask);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverStrokePathNV(mPathId, GL_CONVEX_HULL_NV);
	}
}

void Path::clearFill(GLuint stencilMask) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for clearFill().");
			return;
		}

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, stencilMask);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverFillPathNV(mPathId, GL_CONVEX_HULL_NV);
	}
}

void Path::coverStroke(const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverStroke().");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverStrokePathNV(mPathId, GL_CONVEX_HULL_NV);
	}
}

void Path::coverFill(const ColorA& color, float opacity, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverFill().");
			return;
		}

		ScopedShader scpShader(color, opacity);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverFillPathNV(mPathId, GL_BOUNDING_BOX_NV);
	}
}

void Path::coverFill(const Paint& paint, float opacity, bool clearStencil) const {
	if (mPathId > 0 && !paint.isNone()) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverFill().");
			return;
		}

		auto& paints = getPaints();

		ScopedShader scpShader(paints.preparePaint(paint, opacity));
		ScopedPaints scpGradients(paints);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverFillPathNV(mPathId, GL_BOUNDING_BOX_NV);
	}
}

void Path::coverFill(const Paint& paint, const gl::Texture2dRef& texture, float opacity, bool clearStencil) const {
	if (mPathId > 0 && !paint.isNone()) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverFill().");
			return;
		}

		auto& paints = getPaints();

		ScopedShader scpShader(paints.preparePaint(paint, opacity));

		gl::ScopedTextureBind scpTexture(texture, 0);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glCoverFillPathNV(mPathId, GL_BOUNDING_BOX_NV);
	}
}

void Path::coverFill(const ColorA& color, const Rectf& bounds, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverFill().");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		gl::drawSolidRect(bounds);
	}
}

void Path::coverFill(const gl::Texture2dRef& texture, const Rectf& bounds, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for coverFill().");
			return;
		}

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		draw(texture, bounds);
	}
}

void Path::stroke(const ColorA& color, float opacity, bool clearStencil) const {
	if (mPathId > 0) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		ScopedShader scpShader(color, opacity);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverStrokePathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverStrokePathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::stroke(const Paint& paint, float opacity, bool clearStencil) const {
	if (mPathId > 0 && !paint.isNone()) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		auto& paints = getPaints();

		ScopedShader scpShader(paints.preparePaint(paint, opacity));
		ScopedPaints scpGradients(paints);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverStrokePathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverStrokePathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::strokeInstanced(const std::vector<glm::mat3x2>& transforms, const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		strokeInstanced(sPaths, transforms, color, clearStencil);
	}
}

void Path::strokeInstanced(const std::vector<glm::mat4x3>& transforms, const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		strokeInstanced(sPaths, transforms, color, clearStencil);
	}
}

void Path::strokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
						   const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for instanced rendering.");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverStrokePathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(),
												mPathId, GL_COUNT_UP_NV, 0xFF, GL_CONVEX_HULL_NV, GL_AFFINE_2D_NV,
												reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::strokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat4x3>& transforms,
						   const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for instanced rendering.");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverStrokePathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(),
												mPathId, GL_COUNT_UP_NV, 0xFF, GL_CONVEX_HULL_NV, GL_AFFINE_3D_NV,
												reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::fill(const ColorA& color, float opacity, bool clearStencil) const {
	if (mPathId > 0) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		ScopedShader scpShader(color, opacity);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverFillPathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::fill(const Paint& paint, float opacity, bool clearStencil) const {
	if (mPathId > 0 && !paint.isNone()) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		auto&		 paints = getPaints();
		ScopedShader scpShader(paints.preparePaint(paint, opacity));
		ScopedPaints scpGradients(paints);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverFillPathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::fill(const Paint& paint, const gl::TextureRef& texture, float opacity, bool clearStencil) const {
	if (mPathId > 0 && !paint.isNone()) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		auto&		 paints = getPaints();
		ScopedShader scpShader(paints.preparePaint(paint, opacity));

		gl::ScopedTextureBind scpTexture(texture, 0);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverFillPathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::fill(const gl::TextureRef& texture, const Rectf& bounds, float opacity, bool clearStencil) const {
	if (mPathId > 0) {
		const auto pathBounds	 = getFillBounds();
		const auto textureBounds = Rectf(texture->getBounds());
		const auto fit			 = bounds.getCenteredFit(textureBounds, true).scaled(1.0f / textureBounds.getSize());

		auto normalized =
			Rectf(pathBounds.getUpperLeft() - bounds.getUpperLeft(), pathBounds.getLowerRight() - bounds.getUpperLeft())
				.scaled(fit.getSize() / bounds.getSize());
		normalized.offset(fit.getUpperLeft());

		const auto flip				  = texture->isTopDown();
		const auto upperLeftTexCoord  = vec2{normalized.x1, flip ? normalized.y2 : normalized.y1};
		const auto lowerRightTexCoord = vec2{normalized.x2, flip ? normalized.y1 : normalized.y2};
		fill(texture, upperLeftTexCoord, lowerRightTexCoord, opacity, clearStencil);
	}
}

void Path::fill(const gl::TextureRef& texture, const vec2& upperLeftTexCoord, const vec2& lowerRightTexCoord,
				float opacity, bool clearStencil) const {
	if (mPathId > 0) {
		GLuint clipMask	 = 0x80 >> sClipPaths.size();
		GLuint coverMask = clipMask - 1;
		GLuint bitMask	 = coverMask << 1 | 0x01;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.
		gl::ScopedState	  scpStencil(GL_STENCIL_TEST, GL_TRUE);

		if (!sClipPaths.empty()) {
			// Render clipped path.
			gl::stencilFunc(GL_LESS, GLint(~bitMask & 0xFF), 0xFF); // Only render if all clip bits are set.
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
			gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);
		}

		gl::ScopedTextureBind scpTex(texture, 0);

		ScopedShader scpShader(Shader::Type::IMAGE);
		scpShader.setColor(ColorA::white());
		scpShader.uniform("image", 0);
		scpShader.uniform("opacity", opacity);

		const vec2 s{lowerRightTexCoord.x - upperLeftTexCoord.x, lowerRightTexCoord.y - upperLeftTexCoord.y};
		const mat3 m{1.0f / s.x, 0, 0, 0, 1.0f / s.y, 0, -upperLeftTexCoord.x, -upperLeftTexCoord.y, 1};
		scpShader.setCoords(GL_PATH_OBJECT_BOUNDING_BOX_NV, m);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathNV(mPathId, GL_COUNT_UP_NV, coverMask & 0xFF, GL_CONVEX_HULL_NV);

		if (!sClipPaths.empty()) {
			// Remove path from stencil buffer.
			ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glCoverFillPathNV(mPathId, GL_CONVEX_HULL_NV);
		}
	}
}

void Path::fillInstanced(const std::vector<glm::mat3x2>& transforms, const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		fillInstanced(sPaths, transforms, color, clearStencil);
	}
}

void Path::fillInstanced(const std::vector<glm::mat4x3>& transforms, const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (sPaths.size() < transforms.size()) sPaths.resize(transforms.size(), 0);
		fillInstanced(sPaths, transforms, color, clearStencil);
	}
}

void Path::fillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
						 const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for instanced rendering.");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(),
											  mPathId, GL_COUNT_UP_NV, 0xFF, GL_CONVEX_HULL_NV, GL_AFFINE_2D_NV,
											  reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::fillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat4x3>& transforms,
						 const ColorA& color, bool clearStencil) const {
	if (mPathId > 0) {
		if (!sClipPaths.empty()) {
			CI_LOG_E("Clip-paths are currently not supported for instanced rendering.");
			return;
		}

		ScopedShader scpShader(color);

		gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP);

		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glStencilThenCoverFillPathInstancedNV(static_cast<GLsizei>(transforms.size()), GL_UNSIGNED_INT, paths.data(),
											  mPathId, GL_COUNT_UP_NV, 0xFF, GL_CONVEX_HULL_NV, GL_AFFINE_3D_NV,
											  reinterpret_cast<const GLfloat*>(transforms.data()));
	}
}

void Path::pushClipPath(const Path& mask, bool showMask) {
	if (mask.mPathId > 0) {
		if (sClipPaths.size() < 6) {
			GLuint clipMask	 = 0x80 >> sClipPaths.size();
			GLuint coverMask = clipMask - 1;

			// Render shape to stencil buffer to use it as a clip-path.
			ScopedShader	  scpShader(Color(1, 1, 0));
			ScopedColorMask	  scpColorMask(showMask, showMask, showMask,
										   showMask);				// Don't write to color buffer, unless requested.
			ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

			gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			gl::stencilFunc(GL_NOTEQUAL, GLint(clipMask), coverMask);

			glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
			glStencilFillPathNV(mask.mPathId, GL_COUNT_UP_NV,
								coverMask & 0xFF); // Write path to LSB portion.

			glCoverFillPathNV(mask.mPathId, GL_CONVEX_HULL_NV); // Convert LSB portion to clip bit.

			sClipPaths.push_back(mask.mPathId);
		} else {
			CI_LOG_E("Maximum number of clip-paths reached.");
			sClipPaths.push_back(0);
		}
	} else {
		sClipPaths.push_back(0);
	}
}

void Path::popClipPath() {
	if (!sClipPaths.empty()) {
		GLuint pathId = sClipPaths.back();
		if (pathId > 0) {
			GLuint clipMask	 = 0x80 >> (sClipPaths.size() - 1);
			GLuint coverMask = clipMask - 1;

			// Render shape to stencil buffer to use clear the clip-path.
			ScopedColorMask	  scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.
			ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

			gl::ScopedState scpStencil(GL_STENCIL_TEST, GL_TRUE);
			gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			gl::stencilFunc(GL_ALWAYS, GLint(clipMask), coverMask);

			glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
			glCoverFillPathNV(pathId, GL_CONVEX_HULL_NV); // Clear stencil buffer bits.
		}
		sClipPaths.pop_back();
	}
}

Path Path::operator+(const Path& other) const {
	Path path(*this);
	path += other;
	return path;
}

Path Path::operator-(const Path& other) const {
	Path path(*this);
	path -= other;
	return path;
}

Path& Path::operator+=(const Path& other) {
	if (mPathId > 0) {
		// Get the commands and coords of the other path.
		std::vector<GLubyte> commands;
		std::vector<GLfloat> coords;
		other.getPath(commands, coords);

		GLint ourNumCommands = 0;
		GLint ourNumCoords	 = 0;
		glGetPathParameterivNV(mPathId, GL_PATH_COMMAND_COUNT_NV, &ourNumCommands);
		glGetPathParameterivNV(mPathId, GL_PATH_COORD_COUNT_NV, &ourNumCoords);

		glPathSubCommandsNV(mPathId, ourNumCommands, 0, static_cast<GLsizei>(commands.size()), commands.data(),
							static_cast<GLsizei>(coords.size()), GL_FLOAT, coords.data());
	} else
		*this = Path(other);

	return *this;
}

Path& Path::operator-=(const Path& other) {
	*this += other.reversed();
	return *this;
}

void Path::transform(const glm::mat3x2& m) const {
	if (mPathId > 0) glTransformPathNV(mPathId, mPathId, GL_AFFINE_2D_NV, reinterpret_cast<const GLfloat*>(&m));
}

Path Path::transformed(const glm::mat3x2& m) const {
	Path path(*this);
	path.transform(m);
	return path;
}

void Path::translate(float x, float y) const {
	transform({1, 0, 0, 1, x, y});
}

Path Path::translated(float x, float y) const {
	Path path(*this);
	path.translate(x, y);
	return path;
}

void Path::rotate(float radians) const {
	if (mPathId > 0) {
		const auto c = glm::cos(radians);
		const auto s = glm::sin(radians);
		transform({c, s, -s, c, 0, 0});
	}
}

void Path::rotate(float radians, const vec2& center) const {
	if (mPathId > 0) {
		const auto c = glm::cos(radians);
		const auto s = glm::sin(radians);
		transform({c, s, -s, c, center.x - c * center.x + s * center.y, center.y - s * center.x - c * center.y});
	}
}

Path Path::rotated(float radians) const {
	Path path(*this);
	path.rotate(radians);
	return path;
}

Path Path::rotated(float radians, const vec2& center) const {
	Path path(*this);
	path.rotate(radians, center);
	return path;
}

void Path::scale(float sx, float sy) const {
	if (mPathId > 0) transform({sx, 0, 0, sy, 0, 0});
}

void Path::scale(float sx, float sy, const vec2& center) const {
	if (mPathId > 0) transform({sx, 0, 0, sy, center.x - sx * center.x, center.y - sy * center.y});
}

void Path::scale(const vec2& s, const vec2& center) const {
	if (mPathId > 0) scale(s.x, s.y, center);
}

Path Path::scaled(float sx, float sy) const {
	Path path(*this);
	path.scale(sx, sy);
	return path;
}

Path Path::scaled(const vec2& s, const vec2& center) const {
	Path path(*this);
	path.scale(s, center);
	return path;
}

void Path::skewX(float radiansX) const {
	if (mPathId > 0) transform({1, 0, glm::tan(radiansX), 1, 0, 0});
}

void Path::skewY(float radiansY) const {
	if (mPathId > 0) transform({1, glm::tan(radiansY), 0, 1, 0, 0});
}

void Path::skew(float radiansX, float radiansY) const {
	if (mPathId > 0) transform({1, glm::tan(radiansY), glm::tan(radiansX), 1, 0, 0});
}

Path Path::skewedX(float radiansX) const {
	Path path(*this);
	path.skewX(radiansX);
	return path;
}

Path Path::skewedY(float radiansY) const {
	Path path(*this);
	path.skewY(radiansY);
	return path;
}

Path Path::skewed(float radiansX, float radiansY) const {
	Path path(*this);
	path.skew(radiansX, radiansY);
	return path;
}

// void Path::optimize() const {
//	if (mPathId > 0) {
//		// Get the commands and coords of the path.
//		std::vector<GLubyte> commands;
//		std::vector<GLfloat> coords;
//		getCommandsAndCoords(commands, coords);
//
//		// Optimize the path.
//		optimizeImpl(commands, coords);
//
//		// Store the optimized path.
//		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
//						 static_cast<GLsizei>(coords.size()), GL_FLOAT, coords.data());
//	}
// }
//
// Path Path::optimized() const {
//	Path path(*this);
//	path.optimize();
//	return path;
// }

void Path::reverse() const {
	if (mPathId > 0) {
		// Get the commands and coords of the path.
		std::vector<GLubyte> commands;
		std::vector<GLfloat> coords;
		getPath(commands, coords);

		// Reverse the path.
		reverseImpl(commands, coords);

		// Store the reversed path.
		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(coords.size()), GL_FLOAT, coords.data());
	}
}

Path Path::reversed() const {
	Path path(*this);
	path.reverse();
	return path;
}

bool Path::fillContains(const vec2& point) const {
	if (mPathId > 0) return glIsPointInFillPathNV(mPathId, 0xFF, point.x, point.y);

	return false;
}

bool Path::strokeContains(const vec2& point) const {
	if (mPathId > 0) return glIsPointInStrokePathNV(mPathId, point.x, point.y);

	return false;
}

Shape2d Path::toShape2d() const {
	Shape2d result;

	if (mPathId > 0) {
		// Get the commands and coords of the path.
		std::vector<GLubyte> commands;
		std::vector<GLfloat> coords;
		getPath(commands, coords);

		GLfloat* coord = coords.data();
		for (const auto cmd : commands) {
			vec2 cp{0}, pep{0};

			const auto& contours = result.getContours();
			if (!contours.empty()) {
				const auto& contour = contours.back();
				const auto& points	= contour.getPoints();
				if (!points.empty()) {
					cp = points.back();
					if (points.size() > 1) pep = points[points.size() - 2];
				}
			}

			switch (cmd) {
			case GL_MOVE_TO_NV:
				result.moveTo(coord[0], coord[1]);
				coord += 2;
				break;
			case GL_RELATIVE_MOVE_TO_NV:
				result.moveTo(cp.x + coord[0], cp.y + coord[1]);
				coord += 2;
				break;
			case GL_LINE_TO_NV:
				result.lineTo(coord[0], coord[1]);
				coord += 2;
				break;
			case GL_RELATIVE_LINE_TO_NV:
				result.lineTo(cp.x + coord[0], cp.y + coord[1]);
				coord += 2;
				break;
			case GL_HORIZONTAL_LINE_TO_NV:
				result.lineTo(coord[0], cp.y);
				coord += 1;
				break;
			case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
				result.lineTo(cp.x + coord[0], cp.y);
				coord += 1;
				break;
			case GL_VERTICAL_LINE_TO_NV:
				result.lineTo(cp.x, coord[0]);
				coord += 1;
				break;
			case GL_RELATIVE_VERTICAL_LINE_TO_NV:
				result.lineTo(cp.x, cp.y + coord[0]);
				coord += 1;
				break;
			case GL_QUADRATIC_CURVE_TO_NV:
				result.quadTo(coords[0], coords[1], coords[2], coords[3]);
				coord += 4;
				break;
			case GL_RELATIVE_QUADRATIC_CURVE_TO_NV:
				result.quadTo(cp.x + coords[0], cp.y + coords[1], cp.x + coords[2], cp.y + coords[3]);
				coord += 4;
				break;
			case GL_CUBIC_CURVE_TO_NV:
				result.curveTo(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
				coord += 6;
				break;
			case GL_RELATIVE_CUBIC_CURVE_TO_NV:
				result.curveTo(cp.x + coords[0], cp.y + coords[1], cp.x + coords[2], cp.y + coords[3], cp.x + coords[4],
							   cp.y + coords[5]);
				coord += 6;
				break;
			case GL_SMOOTH_QUADRATIC_CURVE_TO_NV:
				result.quadTo(2 * cp.x - pep.x, 2 * cp.y - pep.y, coords[0], coords[1]);
				coord += 2;
				break;
			case GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV:
				result.quadTo(2 * cp.x - pep.x, 2 * cp.y - pep.y, cp.x + coords[0], cp.y + coords[1]);
				break;
			case GL_SMOOTH_CUBIC_CURVE_TO_NV:
				result.curveTo(2 * cp.x - pep.x, 2 * cp.y - pep.y, coords[0], coords[1], coords[2], coords[3]);
				coord += 4;
				break;
			case GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV:
				result.curveTo(2 * cp.x - pep.x, 2 * cp.y - pep.y, cp.x + coords[0], cp.y + coords[1], cp.x + coords[2],
							   cp.y + coords[3]);
				coord += 4;
				break;
			case GL_CLOSE_PATH_NV:
				result.close();
				break;
			default:
				throw std::runtime_error("Unknown path command");
			}
		}
	}

	return result;
}

GLint Path::getCoordCount(GLubyte command) {
	switch (command) {
	case GL_CLOSE_PATH_NV:
	case GL_RESTART_PATH_NV:
		return 0;
	case GL_HORIZONTAL_LINE_TO_NV:
	case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
	case GL_VERTICAL_LINE_TO_NV:
	case GL_RELATIVE_VERTICAL_LINE_TO_NV:
		return 1;
	case GL_MOVE_TO_NV:
	case GL_RELATIVE_MOVE_TO_NV:
	case GL_LINE_TO_NV:
	case GL_RELATIVE_LINE_TO_NV:
	case GL_SMOOTH_QUADRATIC_CURVE_TO_NV:
	case GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV:
		return 2;
	case GL_QUADRATIC_CURVE_TO_NV:
	case GL_RELATIVE_QUADRATIC_CURVE_TO_NV:
	case GL_SMOOTH_CUBIC_CURVE_TO_NV:
	case GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV:
	case GL_DUP_FIRST_CUBIC_CURVE_TO_NV:
	case GL_DUP_LAST_CUBIC_CURVE_TO_NV:
	case GL_RECT_NV:
	case GL_RELATIVE_RECT_NV:
		return 4;
	case GL_SMALL_CCW_ARC_TO_NV:
	case GL_RELATIVE_SMALL_CCW_ARC_TO_NV:
	case GL_SMALL_CW_ARC_TO_NV:
	case GL_RELATIVE_SMALL_CW_ARC_TO_NV:
	case GL_LARGE_CCW_ARC_TO_NV:
	case GL_RELATIVE_LARGE_CCW_ARC_TO_NV:
	case GL_LARGE_CW_ARC_TO_NV:
	case GL_RELATIVE_LARGE_CW_ARC_TO_NV:
	case GL_CONIC_CURVE_TO_NV:
	case GL_RELATIVE_CONIC_CURVE_TO_NV:
	case GL_ROUNDED_RECT_NV:
	case GL_RELATIVE_ROUNDED_RECT_NV:
	case GL_CIRCULAR_CCW_ARC_TO_NV:
	case GL_CIRCULAR_CW_ARC_TO_NV:
	case GL_CIRCULAR_TANGENT_ARC_TO_NV:
		return 5;
	case GL_CUBIC_CURVE_TO_NV:
	case GL_RELATIVE_CUBIC_CURVE_TO_NV:
	case GL_ROUNDED_RECT2_NV:
	case GL_RELATIVE_ROUNDED_RECT2_NV:
		return 6;
	case GL_ARC_TO_NV:
	case GL_RELATIVE_ARC_TO_NV:
		return 7;
	case GL_ROUNDED_RECT4_NV:
	case GL_RELATIVE_ROUNDED_RECT4_NV:
		return 8;
	case GL_ROUNDED_RECT8_NV:
	case GL_RELATIVE_ROUNDED_RECT8_NV:
		return 12;
	}

	__debugbreak(); // Error!
}

GLint Path::getCoordOffset(GLubyte command) {
	switch (command) {
	case GL_MOVE_TO_NV:
	case GL_RELATIVE_MOVE_TO_NV:
	case GL_LINE_TO_NV:
	case GL_RELATIVE_LINE_TO_NV:
	case GL_HORIZONTAL_LINE_TO_NV:
	case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
	case GL_VERTICAL_LINE_TO_NV:
	case GL_RELATIVE_VERTICAL_LINE_TO_NV:
	case GL_SMOOTH_QUADRATIC_CURVE_TO_NV:
	case GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV:
	case GL_RECT_NV:
	case GL_RELATIVE_RECT_NV:
		return 0;
	case GL_QUADRATIC_CURVE_TO_NV:
	case GL_RELATIVE_QUADRATIC_CURVE_TO_NV:
	case GL_SMOOTH_CUBIC_CURVE_TO_NV:
	case GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV:
	case GL_DUP_FIRST_CUBIC_CURVE_TO_NV:
	case GL_DUP_LAST_CUBIC_CURVE_TO_NV:
	case GL_CONIC_CURVE_TO_NV:
	case GL_RELATIVE_CONIC_CURVE_TO_NV:
		return 2;
	case GL_SMALL_CCW_ARC_TO_NV:
	case GL_RELATIVE_SMALL_CCW_ARC_TO_NV:
	case GL_SMALL_CW_ARC_TO_NV:
	case GL_RELATIVE_SMALL_CW_ARC_TO_NV:
	case GL_LARGE_CCW_ARC_TO_NV:
	case GL_RELATIVE_LARGE_CCW_ARC_TO_NV:
	case GL_LARGE_CW_ARC_TO_NV:
	case GL_RELATIVE_LARGE_CW_ARC_TO_NV:
		return 3;
	case GL_CUBIC_CURVE_TO_NV:
	case GL_RELATIVE_CUBIC_CURVE_TO_NV:
		return 4;
	case GL_ARC_TO_NV:
	case GL_RELATIVE_ARC_TO_NV:
		return 5;
	}

	__debugbreak(); // Error!
}

GLubyte Path::toPathCommand(Path2d::SegmentType type) {
	switch (type) {
	case Path2d::SegmentType::MOVETO:
		return GL_MOVE_TO_NV;
	case Path2d::SegmentType::LINETO:
		return GL_LINE_TO_NV;
	case Path2d::SegmentType::QUADTO:
		return GL_QUADRATIC_CURVE_TO_NV;
	case Path2d::SegmentType::CUBICTO:
		return GL_CUBIC_CURVE_TO_NV;
	case Path2d::SegmentType::CLOSE:
		return GL_CLOSE_PATH_NV;
	}

	return 0;
}

Paints& Path::getPaints() {
	thread_local static Paints sGradients;
	return sGradients;
}

void Path::optimizeImpl(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords) {
	std::vector<GLubyte> revcommands;
	std::vector<GLfloat> revcoords;
	vec2				 cp, np;

	revcommands.reserve(commands.size());
	revcoords.reserve(coords.size());

	float* coord = coords.data();
	for (const auto cmd : commands) {
		switch (cmd) {
		case GL_MOVE_TO_NV:
			revcommands.push_back(GL_MOVE_TO_NV);
			cp = *reinterpret_cast<vec2*>(coord);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
			revcommands.push_back(cmd);
			cp.x += *coord;
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_VERTICAL_LINE_TO_NV:
			revcommands.push_back(cmd);
			cp.y += *coord;
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV:
			revcommands.push_back(cmd);
			cp += *reinterpret_cast<vec2*>(coord);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_QUADRATIC_CURVE_TO_NV:
		case GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV:
			revcommands.push_back(cmd);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			cp += *reinterpret_cast<vec2*>(coord);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_CUBIC_CURVE_TO_NV:
			revcommands.push_back(cmd);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			cp += *reinterpret_cast<vec2*>(coord);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			break;
		case GL_RELATIVE_ARC_TO_NV:
			revcommands.push_back(cmd);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			cp += *reinterpret_cast<vec2*>(coord);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			break;
		case GL_LINE_TO_NV:
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			if (ds::approxEqual(cp.x, np.x)) {
				revcommands.push_back(GL_RELATIVE_VERTICAL_LINE_TO_NV);
				revcoords.push_back(np.y - cp.y);
			} else if (ds::approxEqual(cp.y, np.y)) {
				revcommands.push_back(GL_RELATIVE_HORIZONTAL_LINE_TO_NV);
				revcoords.push_back(np.x - cp.x);
			} else {
				revcommands.push_back(GL_RELATIVE_LINE_TO_NV);
				revcoords.push_back(np.x - cp.x);
				revcoords.push_back(np.y - cp.y);
			}
			cp = np;
			break;
		case GL_HORIZONTAL_LINE_TO_NV:
			revcommands.push_back(GL_RELATIVE_HORIZONTAL_LINE_TO_NV);
			np.x = *coord++;
			revcoords.push_back(np.x - cp.x);
			cp.x = np.x;
			break;
		case GL_VERTICAL_LINE_TO_NV:
			revcommands.push_back(GL_RELATIVE_VERTICAL_LINE_TO_NV);
			np.y = *coord++;
			revcoords.push_back(np.y - cp.y);
			cp.y = np.y;
			break;
		case GL_QUADRATIC_CURVE_TO_NV:
			revcommands.push_back(GL_RELATIVE_QUADRATIC_CURVE_TO_NV);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			cp = np;
			break;
		case GL_CUBIC_CURVE_TO_NV:
			revcommands.push_back(GL_RELATIVE_CUBIC_CURVE_TO_NV);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			cp = np;
			break;
		case GL_SMOOTH_QUADRATIC_CURVE_TO_NV:
			coord += 2;
			break;
		case GL_SMOOTH_CUBIC_CURVE_TO_NV:
			coord += 4;
			break;
		case GL_SMALL_CCW_ARC_TO_NV:
			coord += 5;
			break;
		case GL_SMALL_CW_ARC_TO_NV:
			coord += 5;
			break;
		case GL_LARGE_CCW_ARC_TO_NV:
			coord += 5;
			break;
		case GL_LARGE_CW_ARC_TO_NV:
			coord += 5;
			break;
		case GL_ARC_TO_NV:
			revcommands.push_back(GL_RELATIVE_ARC_TO_NV);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			revcoords.push_back(*coord++);
			np = *reinterpret_cast<vec2*>(coord);
			coord += 2;
			revcoords.push_back(np.x - cp.x);
			revcoords.push_back(np.y - cp.y);
			cp = np;
			break;
		case GL_CLOSE_PATH_NV:
			revcommands.push_back(GL_CLOSE_PATH_NV);
			break;
		}
	}

	std::swap(commands, revcommands);
	std::swap(coords, revcoords);
}

void Path::reverseImpl(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords) {
	// Optimize the path, so that we can more easily reverse it.
	optimizeImpl(commands, coords);

	// Find each sub-path and reverse it.
	std::vector<GLubyte> revcommands;
	std::vector<GLfloat> revcoords;

	auto coordinate = coords.data();
	auto startCmd	= commands.begin();
	auto endCmd		= commands.begin();
	while (endCmd != commands.end()) {
		vec2 sp, cp;
		sp = cp = *reinterpret_cast<vec2*>(coordinate);

		coordinate += getCoordCount(*endCmd++);
		// Find the end of the sub-path.
		while (endCmd != commands.end() && *endCmd != GL_CLOSE_PATH_NV && *endCmd != GL_MOVE_TO_NV &&
			   *endCmd != GL_RELATIVE_MOVE_TO_NV) {
			switch (*endCmd) {
			case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
				cp.x += *coordinate;
				break;
			case GL_RELATIVE_VERTICAL_LINE_TO_NV:
				cp.y += *coordinate;
				break;
			default:
				cp += *reinterpret_cast<vec2*>(coordinate + getCoordOffset(*endCmd));
				break;
			}

			coordinate += getCoordCount(*endCmd++);
		}

		// Reverse the sub-path.
		revcommands.push_back(GL_MOVE_TO_NV);
		if (*endCmd == GL_CLOSE_PATH_NV) {
			revcoords.push_back(sp.x);
			revcoords.push_back(sp.y);
			if (ds::approxEqual(sp.x, cp.x)) {
				if (!ds::approxEqual(sp.y, cp.y)) {
					revcommands.push_back(GL_RELATIVE_VERTICAL_LINE_TO_NV);
					revcoords.push_back(cp.y - sp.y);
				}
			} else if (ds::approxEqual(sp.y, cp.y)) {
				if (!ds::approxEqual(sp.x, cp.x)) {
					revcommands.push_back(GL_RELATIVE_HORIZONTAL_LINE_TO_NV);
					revcoords.push_back(cp.x - sp.x);
				}
			} else {
				revcommands.push_back(GL_RELATIVE_LINE_TO_NV);
				revcoords.push_back(cp.x - sp.x);
				revcoords.push_back(cp.y - sp.y);
			}
		} else {
			revcoords.push_back(cp.x);
			revcoords.push_back(cp.y);
			sp = cp;
		}

		auto coord = coordinate;
		auto cmd   = endCmd;
		while (--cmd != startCmd) {
			revcommands.push_back(*cmd);
			coord -= getCoordCount(*cmd);
			switch (*cmd) {
			case GL_RELATIVE_HORIZONTAL_LINE_TO_NV:
				revcoords.push_back(-coord[0]);
				cp.x -= *coord;
				break;
			case GL_RELATIVE_VERTICAL_LINE_TO_NV:
				revcoords.push_back(-coord[0]);
				cp.y -= *coord;
				break;
			case GL_RELATIVE_LINE_TO_NV:
			case GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV: // Not tested.
				revcoords.push_back(-coord[0]);
				revcoords.push_back(-coord[1]);
				cp -= *reinterpret_cast<vec2*>(coord);
				break;
			case GL_RELATIVE_QUADRATIC_CURVE_TO_NV:
			case GL_SMOOTH_CUBIC_CURVE_TO_NV: // Not tested.
				revcoords.push_back(coord[0] - coord[2]);
				revcoords.push_back(coord[1] - coord[3]);
				revcoords.push_back(-coord[2]);
				revcoords.push_back(-coord[3]);
				cp -= *reinterpret_cast<vec2*>(coord + 2);
				break;
			case GL_RELATIVE_CUBIC_CURVE_TO_NV:
				revcoords.push_back(coord[2] - coord[4]);
				revcoords.push_back(coord[3] - coord[5]);
				revcoords.push_back(coord[0] - coord[4]);
				revcoords.push_back(coord[1] - coord[5]);
				revcoords.push_back(-coord[4]);
				revcoords.push_back(-coord[5]);
				cp -= *reinterpret_cast<vec2*>(coord + 4);
				break;
			case GL_RELATIVE_ARC_TO_NV:
				revcoords.push_back(coord[0]);
				revcoords.push_back(coord[1]);
				revcoords.push_back(coord[2]);
				revcoords.push_back(coord[3]);
				revcoords.push_back(1.0f - coord[4]);
				revcoords.push_back(-coord[5]);
				revcoords.push_back(-coord[6]);
				cp -= *reinterpret_cast<vec2*>(coord + 5);
				break;
				// TODO: small and large arcs + conics.
			}
		}

		// Use CLOSE_PATH_NV if applicable.
		if (ds::approxEqual(sp.x, cp.x) && ds::approxEqual(sp.y, cp.y)) {
			if (revcommands.back() == GL_RELATIVE_LINE_TO_NV ||
				revcommands.back() == GL_RELATIVE_HORIZONTAL_LINE_TO_NV ||
				revcommands.back() == GL_RELATIVE_VERTICAL_LINE_TO_NV) {
				for (size_t i = 0; i < getCoordCount(revcommands.back()); ++i)
					revcoords.pop_back();
				revcommands.pop_back();
				revcommands.push_back(GL_CLOSE_PATH_NV);
			}
		}

		++endCmd;
	}

	std::swap(commands, revcommands);
	std::swap(coords, revcoords);
}

void Path::getPath(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords) const {
	if (mPathId > 0) {
		GLint numCommands;
		glGetPathParameterivNV(mPathId, GL_PATH_COMMAND_COUNT_NV, &numCommands);
		GLint numCoords;
		glGetPathParameterivNV(mPathId, GL_PATH_COORD_COUNT_NV, &numCoords);

		commands.resize(numCommands);
		glGetPathCommandsNV(mPathId, commands.data());

		coords.resize(numCoords);
		glGetPathCoordsNV(mPathId, coords.data());
	}
}

void Path::setPath(const std::vector<GLubyte>& commands, const std::vector<GLfloat>& coords) {
	if (hasNvPathRendering()) {
		if (!mPathId) mPathId = glGenPathsNV(1);

		glPathCommandsNV(mPathId, static_cast<GLsizei>(commands.size()), commands.data(),
						 static_cast<GLsizei>(coords.size()), GL_FLOAT, coords.data());
	} else
		reportNoNvPathRendering();
}

void Path::setPath(const std::string& svg) {
	if (hasNvPathRendering()) {
		if (!mPathId) mPathId = glGenPathsNV(1);

		gl::pathStringNV(mPathId, GL_PATH_FORMAT_SVG_NV, static_cast<GLsizei>(svg.length()), svg.c_str());
	} else
		reportNoNvPathRendering();
}

void PathHelper::set(const Path& path) {
	mCommands.clear();
	mCoords.clear();
	path.getPath(mCommands, mCoords);
}

void PathHelper::removeLastCommand() {
	if (!mCommands.empty()) {
		const auto count = getCoordCount(mCommands.back());
		mCommands.pop_back();
		mCoords.resize(mCoords.size() - count);
	}
}

bool PathHelper::isSelfIntersecting() const {
	if (mCommands.size() < 3) return false;

	std::vector<vec2> intersections;

	const float* a = mCoords.data();
	for (size_t i = 1; i + 1 < mCommands.size(); ++i) {
		const float* b = a + getCoordCount(mCommands[i]);
		for (size_t j = i + 1; j < mCommands.size(); ++j) {
			switch (mCommands[i]) {
			case GL_LINE_TO_NV:
				switch (mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineLine(util::Line{a}, util::Line{b});
					if (!intersections.empty()) return true;
					break;
				case GL_QUADRATIC_CURVE_TO_NV:
					intersections = intersectLineQuad(util::Line{a}, util::QuadraticCurve{b});
					if (!intersections.empty()) return true;
					break;
				case GL_CUBIC_CURVE_TO_NV:
					intersections = intersectLineCubic(util::Line{a}, util::CubicCurve{b});
					if (!intersections.empty()) return true;
					break;
				case GL_ARC_TO_NV:
					intersections = intersectLineArc(util::Line{a}, util::Arc{b});
					if (!intersections.empty()) return true;
					break;
				}

				break;
			case GL_QUADRATIC_CURVE_TO_NV:
				switch (mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineQuad(util::Line{b}, util::QuadraticCurve{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			case GL_CUBIC_CURVE_TO_NV:
				switch (mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineCubic(util::Line{b}, util::CubicCurve{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			case GL_ARC_TO_NV:
				switch (mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineArc(util::Line{b}, util::Arc{a});
					if (!intersections.empty()) return true;
					break;
				case GL_ARC_TO_NV:
					intersections = intersectArcArc(util::Arc{b}, util::Arc{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			}
			b += getCoordCount(mCommands[j]);
		}
		a += getCoordCount(mCommands[i]);
	}

	return false;
}

bool PathHelper::isIntersecting(const PathHelper& other) const {
	if (this == &other) return true;

	if (mCommands.size() < 2) return false;
	if (other.mCommands.size() < 2) return false;

	std::vector<vec2> intersections;

	const float* a = mCoords.data();
	for (size_t i = 1; i < mCommands.size(); ++i) {
		const float* b = other.mCoords.data();
		for (size_t j = 1; j < other.mCommands.size(); ++j) {
			switch (mCommands[i]) {
			case GL_LINE_TO_NV:
				switch (other.mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineLine(util::Line{a}, util::Line{b});
					if (!intersections.empty()) return true;
					break;
				case GL_QUADRATIC_CURVE_TO_NV:
					intersections = intersectLineQuad(util::Line{a}, util::QuadraticCurve{b});
					if (!intersections.empty()) return true;
					break;
				case GL_CUBIC_CURVE_TO_NV:
					intersections = intersectLineCubic(util::Line{a}, util::CubicCurve{b});
					if (!intersections.empty()) return true;
					break;
				case GL_ARC_TO_NV:
					intersections = intersectLineArc(util::Line{a}, util::Arc{b});
					if (!intersections.empty()) return true;
					break;
				}

				break;
			case GL_QUADRATIC_CURVE_TO_NV:
				switch (other.mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineQuad(util::Line{b}, util::QuadraticCurve{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			case GL_CUBIC_CURVE_TO_NV:
				switch (other.mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineCubic(util::Line{b}, util::CubicCurve{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			case GL_ARC_TO_NV:
				switch (other.mCommands[j]) {
				case GL_LINE_TO_NV:
					intersections = intersectLineArc(util::Line{b}, util::Arc{a});
					if (!intersections.empty()) return true;
					break;
				case GL_ARC_TO_NV:
					intersections = intersectArcArc(util::Arc{b}, util::Arc{a});
					if (!intersections.empty()) return true;
					break;
				}
				break;
			}

			b += getCoordCount(other.mCommands[j]);
		}
		a += getCoordCount(mCommands[i]);
	}

	return false;
}

void PathHelper::moveTo(float x, float y) {
	mCommands.push_back(GL_MOVE_TO_NV);
	mCoords.push_back(x);
	mCoords.push_back(y);
}

void PathHelper::lineTo(float x, float y) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_LINE_TO_NV);
	mCoords.push_back(x);
	mCoords.push_back(y);
}

void PathHelper::quadTo(float x1, float y1, float x2, float y2) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_QUADRATIC_CURVE_TO_NV);
	mCoords.push_back(x1);
	mCoords.push_back(y1);
	mCoords.push_back(x2);
	mCoords.push_back(y2);
}

void PathHelper::curveTo(float x1, float y1, float x2, float y2, float x3, float y3) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_CUBIC_CURVE_TO_NV);
	mCoords.push_back(x1);
	mCoords.push_back(y1);
	mCoords.push_back(x2);
	mCoords.push_back(y2);
	mCoords.push_back(x3);
	mCoords.push_back(y3);
}

void PathHelper::horizontalLineTo(float x) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_HORIZONTAL_LINE_TO_NV);
	mCoords.push_back(x);
}

void PathHelper::verticalLineTo(float y) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_VERTICAL_LINE_TO_NV);
	mCoords.push_back(y);
}

void PathHelper::smoothQuadTo(float x2, float y2) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_SMOOTH_QUADRATIC_CURVE_TO_NV);
	mCoords.push_back(x2);
	mCoords.push_back(y2);
}

void PathHelper::smoothCurveTo(float x2, float y2, float x3, float y3) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_SMOOTH_CUBIC_CURVE_TO_NV);
	mCoords.push_back(x2);
	mCoords.push_back(y2);
	mCoords.push_back(x3);
	mCoords.push_back(y3);
}

void PathHelper::arcTo(float radiusX, float radiusY, float xAxisRotation, bool large, bool sweep, float x, float y) {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_ARC_TO_NV);
	mCoords.push_back(radiusX);
	mCoords.push_back(radiusY);
	mCoords.push_back(xAxisRotation);
	mCoords.push_back(large ? 1.0f : 0.0f);
	mCoords.push_back(sweep ? 1.0f : 0.0f);
	mCoords.push_back(x);
	mCoords.push_back(y);
}

void PathHelper::close() {
	assert(!mCommands.empty()); // First command must be GL_MOVE_TO_NV.
	mCommands.push_back(GL_CLOSE_PATH_NV);
}

// void PathCommands::arc(float centerX, float centerY, float radius, float startRadians, float endRadians, bool
// forward) {
//	// TODO: Implement.
// }
//
// void PathCommands::arcTo(float x, float y, float tanX, float tanY, float radius) {
//	// TODO: Implement.
// }

size_t PathHelper::getCoordCount(GLubyte command) {
	switch (command) {
	case GL_HORIZONTAL_LINE_TO_NV:
	case GL_VERTICAL_LINE_TO_NV:
		return 1;
	case GL_MOVE_TO_NV:
	case GL_LINE_TO_NV:
	case GL_SMOOTH_QUADRATIC_CURVE_TO_NV:
		return 2;
	case GL_QUADRATIC_CURVE_TO_NV:
	case GL_SMOOTH_CUBIC_CURVE_TO_NV:
		return 4;
	case GL_CUBIC_CURVE_TO_NV:
		return 6;
	case GL_ARC_TO_NV:
		return 7;
	default:
		return 0;
	}
}

Shader::Shader(Type type)
  : mType(type) {
	switch (type) {
	case Type::SOLID_COLOR: {
		static const char* glsl = "#version 330 core\n"
								  "#extension GL_ARB_separate_shader_objects : enable\n"
								  "precision highp float;"
								  "layout(location = 0) in vec4 color;"
								  "uniform float opacity = 1.0;"
								  "uniform bool premultiplied = false;"
								  "out vec4 fragColor;"
								  "void main() {"
								  "    fragColor = color;"
								  "    fragColor.a *= opacity;"
								  "    if(premultiplied) fragColor.rgb *= fragColor.a;"
								  "}";

		mProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &glsl);
		break;
	}
	case Type::LINEAR_GRADIENT: {
		static const char* glsl =
			"#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"precision highp float;"
			"const  float NOISE_GRANULARITY = 0.5/127.0;" // Assumes 8-bit color, see below.
			"layout(location = 0) in vec4 color;"
			"layout(location = 1) in vec2 uv;"
			"uniform float index = 0.5;"
			"uniform float opacity = 1.0;"
			"uniform bool premultiplied = false;"
			"uniform sampler2D gradTab;"
			"uniform vec2 gradStart;"
			"uniform vec2 gradEnd;"
			"out vec4 fragColor;"
			"float random( vec2 coords) {"
			"    return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);"
			"}"
			"void main() {"
			"    vec2 gradVec = gradEnd - gradStart;"
			"    float gradTabIndex = dot(gradVec, uv - gradStart) / (gradVec.x * gradVec.x + gradVec.y * "
			"gradVec.y);"
			"    fragColor = texture(gradTab, vec2(gradTabIndex, index));"
			// The following line of code reduces color banding for 8-bit color, see:
			// https://shader-tutorial.dev/advanced/color-banding-dithering/
			"    fragColor.rgb += vec3(mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(gl_FragCoord.xy * 0.01)));"
			"    fragColor.a *= opacity;"
			"    if(premultiplied) fragColor.rgb *= fragColor.a;"
			"}";

		mProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &glsl);
		break;
	}
	case Type::RADIAL_GRADIENT: {
		static const char* glsl =
			"#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"precision highp float;"
			"const  float NOISE_GRANULARITY = 0.5/127.0;" // Assumes 8-bit color, see below.
			"uniform sampler2D gradTab;"
			"uniform float index = 0.5;"
			"uniform float opacity = 1.0;"
			"uniform bool premultiplied = false;"
			"uniform vec2 focalToCenter;"
			"uniform float centerRadius;"
			"uniform float focalRadius;"
			"uniform vec2 translationPoint;"
			"layout(location = 0) in vec4 color;"
			"layout(location = 1) in vec2 uv;"
			"out vec4 fragColor;"
			"float random( vec2 coords) {"
			"    return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);"
			"}"
			"void main() {"
			"    vec2 coord = uv - translationPoint;"
			"    float rd = centerRadius - focalRadius;"
			"    float b = 2.0 * (rd * focalRadius + dot(coord, focalToCenter));"
			"    float fmp2_m_radius2 = -focalToCenter.x * focalToCenter.x - focalToCenter.y * focalToCenter.y + "
			"rd * "
			"rd;"
			"    float inverse_2_fmp2_m_radius2 = 1.0 / (2.0 * fmp2_m_radius2);"
			"    float det = b * b - 4.0 * fmp2_m_radius2 * ((focalRadius * focalRadius) - dot(coord, coord));"
			"    fragColor = vec4(0.0);"
			"    if (det >= 0.0) {"
			"        float detSqrt = sqrt(det);"
			"        float w = max((-b - detSqrt) * inverse_2_fmp2_m_radius2, (-b + detSqrt) * "
			"inverse_2_fmp2_m_radius2);"
			"        if (focalRadius + w * (centerRadius - focalRadius) >= 0.0)"
			"            fragColor = texture(gradTab, vec2(w, index));"
			"    }"
			// The following line of code reduces color banding for 8-bit color, see:
			// https://shader-tutorial.dev/advanced/color-banding-dithering/
			"    fragColor.rgb += vec3(mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(gl_FragCoord.xy * 0.01)));"
			"    fragColor.a *= opacity;"
			"    if(premultiplied) fragColor.rgb *= fragColor.a;"
			"}";

		mProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &glsl);
		break;
	}
	case Type::CONICAL_GRADIENT: { // UNTESTED
		static const char* glsl = "#version 330 core\n"
								  "#extension GL_ARB_separate_shader_objects : enable\n"
								  "precision highp float;"
								  "#define INVERSE_2PI 0.1591549430918953358"
								  "uniform sampler2D gradTab;"
								  "uniform float index = 0.5;"
								  "uniform float opacity = 1.0;"
								  "uniform bool premultiplied = false;"
								  "uniform float angle;"
								  "uniform vec2 translationPoint;"
								  "layout(location = 0) in vec4 color;"
								  "layout(location = 1) in vec2 uv;"
								  "out vec4 fragColor;"
								  "void main() {"
								  "    vec2 coord = uv - translationPoint;"
								  "    float t;"
								  "    if (abs(coord.y) == abs(coord.x))"
								  "        t = (atan(-coord.y + 0.002, coord.x) + angle) * INVERSE_2PI;"
								  "    else"
								  "        t = (atan(-coord.y, coord.x) + angle) * INVERSE_2PI;"
								  "    fragColor = texture(gradTab, vec2(t - floor(t), index));"
								  "    fragColor.a *= opacity;"
								  "    if(premultiplied) fragColor.rgb *= fragColor.a;"
								  "}";

		mProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &glsl);
		break;
	}
	case Type::IMAGE: {
		static const char* glsl =
			"#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"precision highp float;"
			"uniform sampler2D image;"
			"uniform float opacity = 1.0;"
			"uniform bool premultiplied = false;"
			"layout(location = 0) in vec4 color;"
			"layout(location = 1) in vec2 uv;"
			"out vec4 fragColor;"
			"void main() {"
			"    fragColor = texture(image, vec2( uv.x, 1.0 - uv.y ) );" // Flip texture vertically.
			"    if( uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1 ) {"
			"        fragColor.a = 0;"
			"    }"
			"    fragColor.a *= opacity;"
			"    if(premultiplied) fragColor.rgb *= fragColor.a;"
			"}";

		mProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &glsl);
		break;
	}
	default:
		mProgram = 0;
		break;
	}

	GLint status = 0;
	glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
	if (!status) {
		// error!
		GLchar	buffer[2048];
		GLsizei length = 0;
		glGetProgramInfoLog(mProgram, 2048, &length, buffer);
		CI_LOG_E(std::string(buffer, length));

		return;
	}

	glGenProgramPipelines(1, &mPipeline);
	glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mProgram);
	glActiveShaderProgram(mPipeline, mProgram);
	glValidateProgramPipeline(mPipeline);

	status = 0;
	glGetProgramPipelineiv(mPipeline, GL_VALIDATE_STATUS, &status);
	if (!status) {
		// error!
		GLchar	buffer[2048];
		GLsizei length = 0;
		glGetProgramInfoLog(mProgram, 2048, &length, buffer);
		CI_LOG_E(std::string(buffer, length));

		return;
	}

	switch (type) {
	case Type::LINEAR_GRADIENT:
	case Type::RADIAL_GRADIENT:
	case Type::CONICAL_GRADIENT:
	case Type::IMAGE: {
		const GLfloat data[6] = {1, 0, 0, 0, 1, 0};
		glProgramPathFragmentInputGenNV(mProgram, 0, GL_PATH_OBJECT_BOUNDING_BOX_NV, 2, &data[0]);
		break;
	}
	case Type::SOLID_COLOR: {
		break;
	}
	default:
		break;
	}
}

Shader::~Shader() {
	if (mPipeline) glDeleteProgramPipelines(1, &mPipeline);
	if (mProgram) glDeleteProgram(mProgram);

	mProgram  = 0;
	mPipeline = 0;
}

void Shader::bind() const {
	if (mPipeline) glBindProgramPipeline(mPipeline);
}

void Shader::unbind() {
	glBindProgramPipeline(0);
}

void Shader::uniform(const std::string& name, GLint value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniform1i(mProgram, loc, value);
	}
}

void Shader::uniform(const std::string& name, GLfloat value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniform1f(mProgram, loc, value);
	}
}

void Shader::uniform(const std::string& name, const vec2& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniform2fv(mProgram, loc, 1, value_ptr(value));
	}
}

void Shader::uniform(const std::string& name, const vec3& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniform3fv(mProgram, loc, 1, value_ptr(value));
	}
}

void Shader::uniform(const std::string& name, const vec4& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniform4fv(mProgram, loc, 1, value_ptr(value));
	}
}

void Shader::uniform(const std::string& name, const mat3& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniformMatrix3fv(mProgram, loc, 1, false, value_ptr(value));
	}
}

void Shader::uniform(const std::string& name, const glm::mat3x2& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniformMatrix3x2fv(mProgram, loc, 1, false, value_ptr(value));
	}
}

void Shader::uniform(const std::string& name, const mat4& value) const {
	if (mPipeline && mProgram) {
		if (const GLint loc = glGetProgramResourceLocation(mProgram, GL_UNIFORM, name.c_str()); loc >= 0)
			glProgramUniformMatrix4fv(mProgram, loc, 1, false, value_ptr(value));
	}
}

void Shader::setColor(const ColorA& color) const {
	glProgramPathFragmentInputGenNV(mProgram, 0, GL_CONSTANT_NV, 4, color.ptr());
}

ScopedShader::ScopedShader(Shader::Type type)
  : mCtx(gl::context()) {
	mShader = Cache::loadShader(type);
	mShader->bind();
	mShader->uniform("premultiplied", isPreMultiplied());
}

ScopedShader::ScopedShader(const ColorA& color, float opacity)
  : ScopedShader(Shader::Type::SOLID_COLOR) {
	mShader->setColor(color);
	mShader->uniform("opacity", opacity);
	mShader->uniform("premultiplied", isPreMultiplied());
}

ScopedShader::~ScopedShader() {
	if (mShader) mShader->unbind();
}

ShaderRef Cache::loadShader(Shader::Type type) {
	Cache& self = get();

	if (self.mShaders.count(type) && static_cast<bool>(self.mShaders.at(type))) {
		return self.mShaders.at(type);
	}

	CI_LOG_V("Creating shader (" << std::this_thread::get_id() << ")");
	auto shader = Shader::create(type);
	self.mShaders.insert_or_assign(type, shader);

	return shader;
}

void Cache::clean() {
	auto& shaders = get().mShaders;
	for (auto itr = shaders.begin(); itr != shaders.end();) {
		const auto& item = *itr;
		if (item.second.use_count() < 2) {
			CI_LOG_V("Removing shader (" << std::this_thread::get_id() << ")");
			itr = shaders.erase(itr);
		} else
			++itr;
	}
}

void Cache::clear() {
	CI_LOG_V("Removing all shaders (" << std::this_thread::get_id() << ")");
	get().mShaders.clear();
}

Canvas::Canvas(int samples, int coverageSamples, bool useFloats)
  : mSamples(samples)
  , mCoverageSamples(coverageSamples)
  , mUseFloats(useFloats) {}

Canvas::Canvas(int width, int height, int samples, int coverageSamples, bool useFloats)
  : mWidth(width)
  , mHeight(height)
  , mSamples(samples)
  , mCoverageSamples(coverageSamples)
  , mUseFloats(useFloats) {}

void Canvas::bind() const {
	if (!mIsBound) {
		if (!mFbo) {
			mCtx = gl::context();
			mFbo = gl::Fbo::create(mWidth, mHeight, getFboFormat());
		}
		if (mCtx == gl::context()) {
			mCtx->pushFramebuffer(mFbo);
			mCtx->pushViewport(std::make_pair(ivec2(0), mFbo->getSize()));
			mCtx->pushGlslProg(nullptr);
			gl::pushMatrices();
			gl::pushModelMatrix();
			gl::setMatricesWindow(mWidth, mHeight);
			gl::popModelMatrix();
			glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
			glMatrixLoadfEXT(GL_PROJECTION, value_ptr(gl::getProjectionMatrix()));
			gl::clearColor(ColorA(0, 0, 0, 0));
			gl::clear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			gl::clearStencil(0);
			gl::stencilMask(~0);
			mIsBound = true;
		}
	}
}

void Canvas::unbind() const {
	if (mIsBound && mCtx == gl::context()) {
		mIsBound = false;
		gl::popMatrices();
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glMatrixLoadfEXT(GL_PROJECTION, value_ptr(gl::getProjectionMatrix()));
		mCtx->popGlslProg(true);
		mCtx->popViewport();
		mCtx->popFramebuffer();
	}
}

void Canvas::draw() const {
	if (mFbo && !mIsBound) {
		// Use pre-multiplied alpha!
		gl::ScopedBlendPremult scpBlend;
		gl::draw(mFbo->getColorTexture());
	}
}

void Canvas::draw(const Rectf& bounds) const {
	if (mFbo && !mIsBound) {
		gl::draw(mFbo->getColorTexture(), bounds);
	}
}

gl::Texture2dRef Canvas::getTexture(GLenum attachment) const {
	if (mFbo) return mFbo->getTexture2d(attachment);

	return nullptr;
}

gl::Fbo::Format Canvas::getFboFormat() const {
	if (mUseFloats) {
		// Using GL_NEAREST, we try to avoid additional blurring of the final image.
		const auto format = gl::Texture2d::Format()
								.internalFormat(GL_RGBA16F)
								.dataType(GL_FLOAT)
								.minFilter(GL_NEAREST)
								.magFilter(GL_NEAREST);
		return gl::Fbo::Format()
			.samples(mSamples)
			.coverageSamples(mCoverageSamples)
			.stencilBuffer()
			.disableDepth()
			.colorTexture(format);
	}

	// Using GL_NEAREST, we try to avoid additional blurring of the final image.
	const auto format = gl::Texture2d::Format().minFilter(GL_NEAREST).magFilter(GL_NEAREST);
	return gl::Fbo::Format()
		.samples(mSamples)
		.coverageSamples(mCoverageSamples)
		.stencilBuffer()
		.disableDepth()
		.colorTexture(format);
}

void Canvas::resize(const ivec2& size) {
	if (size.x > 0 && size.y > 0 && (mWidth != size.x || mHeight != size.y)) {
		mWidth	= size.x;
		mHeight = size.y;

		const bool isBound = mIsBound;
		if (isBound) unbind();

		mFbo.reset();

		if (isBound) bind();
	}
}

ScopedPathRendering::ScopedPathRendering()
  : mCtx(gl::context()) {
	mColor = mCtx->getCurrentColor();

	mCtx->setCurrentColor(Color::white());
	mCtx->pushGlslProg(nullptr);

	// Use pre-multiplied alpha blending.
	// mCtx->pushBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	gl::matrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	gl::matrixLoadfEXT(GL_PROJECTION, value_ptr(gl::getProjectionMatrix()));
}

ScopedPathRendering::~ScopedPathRendering() {
	// mCtx->popBlendFuncSeparate();
	mCtx->popGlslProg();
	mCtx->setCurrentColor(mColor);
}

ScopedCover::ScopedCover(bool clearStencil)
  : mCtx(gl::context()) {
	mCtx->pushBoolState(GL_STENCIL_TEST, GL_TRUE);
	gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);							   // TODO store current value and restore later
	gl::stencilOp(GL_KEEP, GL_KEEP, clearStencil ? GL_ZERO : GL_KEEP); // TODO store current value and restore later
}

ScopedCover::~ScopedCover() {
	mCtx->popBoolState(GL_STENCIL_TEST);
}

} // namespace nvpath

#pragma warning(pop)