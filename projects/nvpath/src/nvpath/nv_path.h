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

#pragma once

namespace nvpath {

// Forward declarations.
class Cache;
class Canvas;
class Paints;
class Path;
class PathHelper;
class Shader;

using CanvasRef = std::shared_ptr<Canvas>;
using PathRef	= std::shared_ptr<Path>;
using ShaderRef = std::shared_ptr<Shader>;

//! Returns whether NV Path Rendering is available on this system.
bool hasNvPathRendering();

void reportNoNvPathRendering();

//! Returns whether pre-multiplied alpha is currently enabled.
bool isPreMultiplied();

//! Returns a thread-local instance of the Paints cache. Using lazy initialization to avoid a rare crash in Debug mode.
Paints& sPaints();

//! Keeps track of the clip path stack. Using lazy initialization to avoid a rare crash in Debug mode.
std::vector<Path>& sClipPaths();

//! Applies the \a mask as a clip mask, causing subsequent rendering to be clipped to the path. We allow a maximum of 5
//! nested clip paths.
void pushClipPath(const Path& mask, GLuint stencilMask = 0xFF, bool showMask = false);

//! Removes the last clip path from the stack. See also: pushClipPath() and ScopedClipPath.
void popClipPath();

//!
enum class CapsStyle {
	FLAT	   = GL_FLAT,
	SQUARE	   = GL_SQUARE_NV,
	ROUND	   = GL_ROUND_NV,
	TRIANGULAR = GL_TRIANGULAR_NV,
	DEFAULT	   = GL_FLAT
};
//!
enum class JoinStyle {
	ROUND		   = GL_ROUND_NV,
	BEVEL		   = GL_BEVEL_NV,
	MITER_REVERT   = GL_MITER_REVERT_NV,
	MITER_TRUNCATE = GL_MITER_TRUNCATE_NV,
	DEFAULT		   = GL_MITER_REVERT_NV
};
//!
enum class PathStyle {
	MOVETO_RESETS	 = GL_MOVE_TO_RESETS_NV,
	MOVETO_CONTINUES = GL_MOVE_TO_CONTINUES_NV,
	DEFAULT			 = GL_MOVE_TO_RESETS_NV
};

//! Defines the coordinate system used by gradients and images.
enum class CoordinateSpace {
	OBJECT_BOUNDING_BOX = GL_PATH_OBJECT_BOUNDING_BOX_NV,
	USER_SPACE_ON_USE	= GL_OBJECT_LINEAR_NV,
	DEFAULT				= GL_PATH_OBJECT_BOUNDING_BOX_NV
};

//! Defines the spread method used by gradient and images.
enum class SpreadMethod { PAD = GL_CLAMP_TO_EDGE, REFLECT = GL_MIRRORED_REPEAT, REPEAT = GL_REPEAT, DEFAULT = PAD };

//!
enum class PathFormat { SVG = GL_PATH_FORMAT_SVG_NV, PS = GL_PATH_FORMAT_PS_NV };

//! Converts an affine 3x3 matrix to a 3x2 matrix.
inline glm::mat3x2 toMat3x2(const glm::mat3x3& m) {
	return glm::mat3x2{m[0][0], m[0][1], m[1][0], m[1][1], m[2][0], m[2][1]};
}
//! Converts an affine 4x4 matrix to a 3x2 matrix. Z-components are ignored.
inline glm::mat3x2 toMat3x2(const glm::mat4x4& m) {
	return glm::mat3x2{m[0][0], m[0][1], m[1][0], m[1][1], m[3][0], m[3][1]};
}
//! Converts an affine 3x2 matrix to a 3x3 matrix.
inline glm::mat3x3 toMat3x3(const glm::mat3x2& m) {
	return {m[0][0], m[0][1], 0, m[1][0], m[1][1], 0, m[2][0], m[2][1], 1};
}
//! Converts an affine 4x4 matrix to a 3x3 matrix. Z-components are ignored.
inline glm::mat3x3 toMat3x3(const glm::mat4x4& m) {
	return {m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], m[3][3]};
}
//! Converts an affine 3x2 matrix to a 4x4 matrix.
inline glm::mat4x4 toMat4x4(const glm::mat3x2& m) {
	return {m[0][0], m[0][1], 0, 0, m[1][0], m[1][1], 0, 0, 0, 0, 1, 0, m[2][0], m[2][1], 0, 1};
}
//! Converts an affine 3x3 matrix to a 4x4 matrix.
inline glm::mat4x4 toMat4x4(const glm::mat3x3& m) {
	return {m[0][0], m[0][1], 0, m[0][2], m[1][0], m[1][1], 0, m[1][2], 0, 0, 1, 0, m[2][0], m[2][1], 0, m[2][2]};
}

//! Shader for solid colors or gradients to be applied to paths.
//! Requires OpenGL v4.1 or GL_ARB_separate_shader_objects.
class Shader {
  public:
	enum class Type { SOLID_COLOR, LINEAR_GRADIENT, RADIAL_GRADIENT, CONICAL_GRADIENT, IMAGE, UNDEFINED };

	static ShaderRef create(Type type) { return std::make_shared<Shader>(type); }

	Shader() = default;
	explicit Shader(Type type);
	virtual ~Shader();

	Shader(const Shader&)			 = delete;
	Shader(Shader&&)				 = default;
	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&)		 = default;

	void		bind() const;
	static void unbind();

	void uniform(const std::string& name, GLint value) const;
	void uniform(const std::string& name, GLfloat value) const;
	void uniform(const std::string& name, const glm::vec2& value) const;
	void uniform(const std::string& name, const glm::vec3& value) const;
	void uniform(const std::string& name, const glm::vec4& value) const;
	void uniform(const std::string& name, const glm::mat3& value) const;
	void uniform(const std::string& name, const glm::mat3x2& value) const;
	void uniform(const std::string& name, const glm::mat4& value) const;

	void setColor(const ci::ColorA& color) const;

  protected:
	GLuint mProgram{0};
	GLuint mPipeline{0};
	Type   mType{Type::UNDEFINED};

	friend class ScopedShader;
};

class ScopedShader : public ci::Noncopyable {
	ci::gl::Context* mCtx = nullptr;
	ShaderRef		 mShader;

  public:
	//!
	explicit ScopedShader(Shader::Type type);
	//! Activates the solid color shader and sets the current color.
	explicit ScopedShader(const ci::ColorA& color, float opacity = 1);

	~ScopedShader();

	ScopedShader(const ScopedShader&)			 = delete;
	ScopedShader(ScopedShader&&)				 = delete;
	ScopedShader& operator=(const ScopedShader&) = delete;
	ScopedShader& operator=(ScopedShader&&)		 = delete;

	template <typename T>
	void uniform(const std::string& name, const T& value) {
		if (mShader) mShader->uniform(name, value);
	}

	void setColor(const ci::ColorA& color) const {
		if (mShader) mShader->setColor(color);
	}

	//! Only works for linear gradient, radial gradient and image shaders! TODO
	void setCoords(GLenum genMode = GL_PATH_OBJECT_BOUNDING_BOX_NV, const glm::mat3& transform = {}) const {
		// Sanity check.
		static_assert(sizeof(glm::mat3::value_type) == sizeof(GLfloat));

		if (mShader && (mShader->mType == Shader::Type::LINEAR_GRADIENT ||
						mShader->mType == Shader::Type::RADIAL_GRADIENT || mShader->mType == Shader::Type::IMAGE)) {
			const auto t = transpose(inverse(transform));
			glProgramPathFragmentInputGenNV(mShader->mProgram, 1, genMode, 2, value_ptr(t));
		}
	}

	//! Only works for linear gradient, radial gradient and image shaders! TODO
	void setCoords(GLenum genMode, const std::vector<float>& data) const {
		assert(data.size() == 6);

		if (mShader && (mShader->mType == Shader::Type::LINEAR_GRADIENT ||
						mShader->mType == Shader::Type::RADIAL_GRADIENT || mShader->mType == Shader::Type::IMAGE)) {
			glProgramPathFragmentInputGenNV(mShader->mProgram, 1, genMode, 2, data.data());
		}
	}
};

//! SVG Paint specification for fill or stroke, including solids and gradients
class Paint {
  public:
	enum Type : uint8_t { NONE, COLOR, LINEAR_GRADIENT, RADIAL_GRADIENT };

	struct Stop {
		Stop(float offset, const ci::ColorA8u& color)
		  : offset(offset)
		  , color(color) {}

		float		 offset;
		ci::ColorA8u color;

		bool operator==(const Stop& other) const;
		bool operator!=(const Stop& other) const { return !(*this == other); }
		bool operator<(const Stop& other) const { return offset < other.offset; }

		bool operator==(float value) const;
		bool operator!=(float value) const { return !(*this == value); }
		bool operator<(float value) const { return offset < value; }
	};

	static Paint color(const ci::ColorA8u& color, const std::string& id = {});
	static Paint linear(const std::string& id = {});
	static Paint radial(const std::string& id = {});

	Paint();
	explicit Paint(Type type, std::string id = {});
	explicit Paint(const ci::ColorA8u& color);
	explicit Paint(std::string url); // Marks Paint as "needs resolve".

	Paint(const Paint&)			   = default;
	Paint(Paint&&)				   = default;
	Paint& operator=(const Paint&) = default;
	Paint& operator=(Paint&&)	   = default;

	~Paint() = default;

	// static Paint parse(const char* value, bool* specified, const SvgNode* parentNode); // TODO parse in SVG code.

	bool			   isNone() const { return mType == NONE; }
	bool			   isColor() const { return mType == COLOR; }
	bool			   isLinearGradient() const { return mType == LINEAR_GRADIENT; }
	bool			   isRadialGradient() const { return mType == RADIAL_GRADIENT; }
	[[nodiscard]] bool isTransparent() const;
	bool			   needsResolve() const { return mNeedsResolve; }

	//! Returns the solid color. Returns black or fallback color for gradients.
	[[nodiscard]] const ci::ColorA8u& getColor() const;
	//! Replaces all existing colors (if any) with a single /a color.
	void setColor(const ci::ColorA8u& color);

	const ci::ColorA8u& getColor(size_t idx) const { return mStops.at(idx).color; }
	float				getOffset(size_t idx) const { return mStops.at(idx).offset; }

	void   clear() { mStops.clear(); }
	bool   empty() const { return mStops.empty(); }
	size_t getNumColors() const { return mStops.size(); }

	// only apply to gradients
	[[nodiscard]] glm::vec2 getCoords0() const { return mCoords0; } // (x1,y1) on linear, (cx,cy) on radial
	[[nodiscard]] glm::vec2 getCoords1() const { return mCoords1; } // (x2,y2) on linear, (fx,fy) on radial
	[[nodiscard]] float		getRadius0() const { return mRadius0; } // (r) on radial
	[[nodiscard]] float		getRadius1() const { return mRadius1; } // (fr) on radial

	bool useObjectBoundingBox() const { return mUseObjectBoundingBox; }
	void setUseObjectBoundingBox(bool enable) { mUseObjectBoundingBox = enable; }

	bool specifiesTransform() const { return mSpecifiesTransform; }
	void setTransform(const glm::mat3& m) {
		mTransform			= m;
		mSpecifiesTransform = true;
	}
	[[nodiscard]] glm::mat3 getTransform() const { return mTransform; }
	void					multiplyTransform(const glm::mat3& m) {
		   mTransform *= m;
		   mSpecifiesTransform = true;
	}

	bool		 specifiesSpreadMethod() const { return mSpecifiesSpreadMethod; }
	SpreadMethod getSpreadMethod() const { return mSpreadMethod; }
	void		 setSpreadMethod(SpreadMethod spreadMethod) {
		mSpreadMethod		   = spreadMethod;
		mSpecifiesSpreadMethod = true;
	}

	[[nodiscard]] CoordinateSpace getCoordinateSpace() const {
		return mUseObjectBoundingBox ? CoordinateSpace::OBJECT_BOUNDING_BOX : CoordinateSpace::USER_SPACE_ON_USE;
	}

	[[nodiscard]] const std::string& getId() const { return mId; }

	const std::shared_ptr<Paint>& fallback() const { return mFallback; }

	bool operator==(const Paint& rhs) const { return mId == rhs.mId && mType == rhs.mType && mStops == rhs.mStops; }
	bool operator!=(const Paint& rhs) const { return !(*this == rhs); }
	bool operator<(const Paint& rhs) const { return this < &rhs; }

	//! Sets a stop color.
	void set(float offset, const ci::ColorA8u& color);
	//! Sets (x1,y1) on linear, (cx,cy) on radial.
	void setCoords0(float a, float b) {
		mCoords0.x = a;
		mCoords0.y = b;
	}
	//! Sets (x1,y1) on linear, (cx,cy) on radial.
	void setCoords0(const glm::vec2& coords) { mCoords0 = coords; }
	//! Sets (x2,y2) on linear, (fx,fy) on radial.
	void setCoords1(float a, float b) {
		mCoords1.x = a;
		mCoords1.y = b;
	}
	//! Sets (x2,y2) on linear, (fx,fy) on radial.
	void setCoords1(const glm::vec2& coords) { mCoords1 = coords; }
	//! Sets (r) on radial.
	void setRadius0(float radius) { mRadius0 = radius; }
	//! Sets (fr) on radial.
	void setRadius1(float radius) { mRadius1 = radius; }

	//! Returns the (interpolated) color at position \a t.
	[[nodiscard]] ci::ColorA8u at(float t, bool preMultiply = false) const;

	//! Returns the nearest stop lower than position \a t.
	[[nodiscard]] const Stop& floor(float t) const;
	//! Returns the nearest stop higher than position \a t.
	[[nodiscard]] const Stop& ceil(float t) const;
	//! Returns all stops as a read-only vector.
	const std::vector<Stop>& getStops() const { return mStops; }
	//! Returns all stops as a writable vector.
	std::vector<Stop>& getStops() { return mStops; }

	[[nodiscard]] auto begin() const { return mStops.begin(); }
	[[nodiscard]] auto end() const { return mStops.end(); }

	[[nodiscard]] auto rbegin() const { return mStops.rbegin(); }
	[[nodiscard]] auto rend() const { return mStops.rend(); }

	//! Returns raw 8-bit RGBA data. You can use this to construct a Surface.
	[[nodiscard]] std::unique_ptr<uint8_t[]> data(int32_t width, int32_t height, bool preMultiply = false,
												  float from = 0.0f, float to = 1.0f) const;

	const std::shared_ptr<Paint>& getFallback() const { return mFallback; }
	void						  setFallback(const std::shared_ptr<Paint>& fallback) { mFallback = fallback; }

  protected:
	glm::vec2			   mCoords0{0, 0}, mCoords1{0, 1};	 //
	float				   mRadius0{0}, mRadius1{0};		 //
	glm::mat3			   mTransform{};					 //
	SpreadMethod		   mSpreadMethod{SpreadMethod::PAD}; //
	Type				   mType{NONE};						 //
	std::vector<Stop>	   mStops;							 //
	bool				   mSpecifiesTransform{false};		 //
	bool				   mUseObjectBoundingBox{true};		 // Used to default to false. Please check.
	bool				   mSpecifiesSpreadMethod{false};	 //
	bool				   mNeedsResolve{false};			 //
	std::string			   mId;								 //
	std::shared_ptr<Paint> mFallback;						 //
};

//! Stores multiple paints in a single texture for maximum performance.
class Paints {
  public:
	Paints()  = default;
	~Paints() = default;

	Paints(const Paints&)				 = delete;
	Paints(Paints&&) noexcept			 = default;
	Paints& operator=(const Paints&)	 = delete;
	Paints& operator=(Paints&&) noexcept = default;

	//!
	bool empty() const { return mPaints.empty(); }
	//!
	void clear() {
		mIndex = 0;
		mPaints.clear();
		mTexture.reset();
		mDirty = true;
	}
	//!
	size_t size() const { return mPaints.size(); }

	//! Returns whether the paint is known.
	bool contains(const std::string& paintId) const;
	//! Returns whether the paint is known.
	bool contains(const Paint& paint) const;
	//! Returns the coordinate of the paint in the texture.
	float index(const std::string& paintId) const;
	//! Returns the coordinate of the paint in the texture.
	float index(const Paint& paint) const;
	//! Sets or updates the paint.
	float set(const Paint& paint);

	//! Binds the paint texture to \a textureUnit. If the texture does not yet exist, or if paints have changed,
	//! it will be created.
	void bind(ci::gl::Context* ctx, uint8_t textureUnit = 0);
	//!
	void unbind(const ci::gl::Context* ctx) const;

	//! Applies the spread \a method to the paint texture.
	void setSpreadMethod(SpreadMethod method) const;

	//! Creates or activates a paint. Optionally prepares the correct shader as well.
	Shader::Type preparePaint(const Paint& paint, float opacity = 1, bool prepareShader = true);

  private:
	//!  Creates or activates a linear gradient. Optionally prepares the correct shader as well.
	Shader::Type prepareLinearGradient(const Paint& paint, float opacity = 1, bool prepareShader = true);
	//!  Creates or activates a radial gradient. Optionally prepares the correct shader as well.
	Shader::Type prepareRadialGradient(const Paint& paint, float opacity = 1, bool prepareShader = true);
	//!
	GLint textureSize() const;

	ci::gl::Context*					   mCtx = nullptr;	//
	size_t								   mIndex{0};		//
	std::unordered_map<std::string, Paint> mPaints;			//
	ci::gl::Texture2dRef				   mTexture{};		//
	uint8_t								   mTextureUnit{0}; //
	bool								   mDirty{true};	//
};

class ScopedPaints {
	Paints&			 mPaints;
	ci::gl::Context* mCtx = nullptr;

  public:
	explicit ScopedPaints(Paints& paints, uint8_t textureUnit = 0)
	  : mPaints(paints)
	  , mCtx(ci::gl::context()) {
		mPaints.bind(mCtx, textureUnit);
	}
	~ScopedPaints() { mPaints.unbind(mCtx); }

	ScopedPaints(const ScopedPaints&)			 = delete;
	ScopedPaints(ScopedPaints&&)				 = delete;
	ScopedPaints& operator=(const ScopedPaints&) = delete;
	ScopedPaints& operator=(ScopedPaints&&)		 = delete;
};

//! Path represents a vector shape stored efficiently on the GPU.
class Path {
  public:
	~Path();

	Path() = default;
	Path(const Path& other);
	Path(Path&& other) noexcept;
	Path& operator=(const Path& other);
	Path& operator=(Path&& other) noexcept;

	//! Construct a path from a Path2d. Note the correct winding order: points should be defined in counter clockwise
	//! order.
	Path(const ci::Path2d& path);
	//! Construct a path from a Shape2d. Note the correct winding order: holes should be defined in clockwise order.
	Path(const ci::Shape2d& shape);
	//! Construct a path from a PolyLine2. Note the correct winding order: points should be defined in counter clockwise
	//! order.
	Path(const ci::PolyLine2& polyLine);
	//! Construct a path from a \a path string. Accepts SVG or PostScript \a format.
	explicit Path(const std::string& path, PathFormat format = PathFormat::SVG);
	//! Construct a path from a vector of \a commands and 2D \a points.
	explicit Path(const std::vector<GLubyte>& commands, const std::vector<glm::vec2>& points);
	//! Construct a path from a vector of \a commands and 2D \a points.
	explicit Path(const std::vector<GLubyte>& commands, const std::vector<GLfloat>& points);
	//! Construct a path from the \a commands.
	explicit Path(const class PathHelper& commands);

	//! Creates a shallow clone of this path. Use with care.
	[[nodiscard]] PathRef clone() const { return std::make_shared<Path>(*this); }

	//! Returns the path's unique id number.
	GLuint getId() const { return mPathId; }

	//! Returns a point on the path at a given \a distance along the path, as well as the (unnormalized) tangent at that
	//! point. Returns false if the distance is out of bounds.
	bool getPointAlongPath(float distance, glm::vec2& point, glm::vec2& tangent) const;
	//! Returns the total length of the path.
	[[nodiscard]] float getLength() const;
	//! Returns the path's bounding box, calculated from the actual shape.
	[[nodiscard]] ci::Rectf getFillBounds() const;
	//! Returns the path's bounding box, calculated from the actual shape and adjusted for stroke width.
	[[nodiscard]] ci::Rectf getStrokeBounds() const;

	//! Returns the path's client length. Dash patterns use the client length to scale the dash pattern. Returns zero if
	//! not set.
	[[nodiscard]] float getClientLength() const;
	//! Sets the path's client length. Dash patterns use the client length to scale the dash pattern. Use zero to
	//! disable.
	void setClientLength(float length) const;

	//! Obtains the path's commands and coords.
	void getPath(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords) const;
	//! Set the path's commands and coords. This will overwrite any existing commands and coords.
	void setPath(const std::vector<GLubyte>& commands, const std::vector<GLfloat>& coords);
	//! Set the path's commands and coords using the provided \a svg string. This will overwrite any existing commands
	//! and coords.
	void setPath(const std::string& svg);

	//! Returns the number of segments defined for this path.
	[[nodiscard]] int getNumSegments() const;

	//! Resets the dash pattern.
	void resetDashPattern() const;
	//! Sets the dash pattern.
	void setDashPattern(const std::vector<float>& pattern) const;
	//! Sets the dash pattern, making sure it precisely fits an integer number of times on the path.
	void setDashPatternFitted(std::vector<float> pattern) const;
	//! Sets the dash pattern offset.
	void setDashOffset(float offset, PathStyle style = PathStyle::DEFAULT) const;
	//! Sets the caps for dashed strokes.
	void setDashCaps(CapsStyle caps) const;
	//! Sets the caps for dashed strokes.
	void setDashCaps(CapsStyle initialCap, CapsStyle terminalCap) const;
	//! Sets the path's end caps for strokes.
	void setEndCaps(CapsStyle caps) const;
	//! Sets the path's end caps for strokes.
	void setEndCaps(CapsStyle initialCap, CapsStyle terminalCap) const;
	//! Sets the join style for strokes.
	void setJoinStyle(JoinStyle joins) const;
	//! Sets the stroke width.
	void setStrokeWidth(float width) const;
	//! Sets the miter limit for stokes.
	void setMiterLimit(float limit) const;

	//! Sets multiple stroke parameters at once.
	void setStroke(CapsStyle caps, float strokeWidth) const { setStroke(caps, JoinStyle::DEFAULT, strokeWidth); }
	//! Sets multiple stroke parameters at once.
	void setStroke(JoinStyle join, float strokeWidth) const { setStroke(CapsStyle::DEFAULT, join, strokeWidth); }
	//! Sets multiple stroke parameters at once.
	void setStroke(CapsStyle caps, JoinStyle join, float strokeWidth) const;

	//! Renders the path to the stencil buffer but does not cover the path.
	//! Use the `stroke()` methods to stencil and cover the path in a single step.
	void stencilStroke(GLuint stencilMask = 0xFF) const;
	//! Renders the path to the stencil buffer but does not cover the path.
	//! Use the `fill()` methods to stencil and cover the path in a single step.
	void stencilFill(GLuint stencilMask = 0xFF, GLenum fillMode = GL_COUNT_UP_NV) const;

	//! Renders the path instances to the stencil buffer but does not cover the path instances.
	void stencilStrokeInstanced(const std::vector<glm::mat3x2>& transforms, GLuint stencilMask = 0xFF) const;
	//! Renders the path instances to the stencil buffer but does not cover the path instances. The \a paths vector
	//! contains offsets to the base path id.
	void stencilStrokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
								GLuint stencilMask = 0xFF) const;
	//! Renders the path instances to the stencil buffer but does not cover the path instances.
	void stencilFillInstanced(const std::vector<glm::mat3x2>& transforms, GLuint stencilMask = 0xFF,
							  GLenum fillMode = GL_COUNT_UP_NV) const;
	//! Renders the path instances to the stencil buffer but does not cover the path instances. The \a paths vector
	//! contains offsets to the base path id.
	void stencilFillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
							  GLuint stencilMask = 0xFF, GLenum fillMode = GL_COUNT_UP_NV) const;

	//!
	void clearStroke(GLuint stencilMask = 0xFF) const;
	//!
	void clearFill(GLuint stencilMask = 0xFF) const;

	//! Covers the paths that have already been rendered to the stencil buffer using a solid \a color. Clears the
	//! affected region of the stencil buffer by default, but this can be overridden. Use the `stroke()`
	//! method to stencil and cover the path in a single step.
	void coverStroke(const ci::ColorA& color, bool clearStencil = true) const;
	//! Covers the paths that have already been rendered to the stencil buffer using a solid \a color. Clears the
	//! affected region of the stencil buffer by default, but this can be overridden. Use the `fill()`
	//! method to stencil and cover the path in a single step.
	void coverFill(const ci::ColorA& color, float opacity = 1, bool clearStencil = true) const;
	//! Covers the paths that have already been rendered to the stencil buffer using a \a paint, which can be a
	//! gradient. Clears the affected region of the stencil buffer by default, but this can be overridden. Use the
	//! `fill()` method to stencil and cover the path in a single step.
	void coverFill(const Paint& paint, float opacity = 1, bool clearStencil = true) const;
	//! Covers the paths that have already been rendered to the stencil buffer using a \a paint, but using the \a
	//! texture for its colors. Clears the affected region of the stencil buffer by default, but this can be overridden.
	//! Use the `fill()` method to stencil and cover the path in a single step.
	void coverFill(const Paint& paint, const ci::gl::Texture2dRef& texture, float opacity = 1,
				   bool clearStencil = true) const;
	//! Covers the paths that have already been rendered to the stencil buffer using a solid \a color. Clears the
	//! affected region of the stencil buffer by default, but this can be overridden.
	void coverFill(const ci::ColorA& color, const ci::Rectf& bounds, bool clearStencil = true) const;
	//! Covers the paths that have already been rendered to the stencil buffer using the provided \a texture. Clears the
	//! affected region of the stencil buffer by default, but this can be overridden.
	void coverFill(const ci::gl::Texture2dRef& texture, const ci::Rectf& bounds, bool clearStencil = true) const;

	//! Strokes the path with a solid \a color.
	void stroke(const ci::ColorA& color, float opacity = 1, bool clearStencil = true) const;
	//! Strokes the path with a \a paint. Gradients are currently not supported.
	void stroke(const Paint& paint, float opacity = 1, bool clearStencil = true) const;

	//! Strokes the path instances with a solid \a color.
	void strokeInstanced(const std::vector<glm::mat3x2>& transforms, const ci::ColorA& color,
						 bool clearStencil = true) const;
	//! Strokes the path instances with a solid \a color.
	void strokeInstanced(const std::vector<glm::mat4x3>& transforms, const ci::ColorA& color,
						 bool clearStencil = true) const;
	//! Strokes the path instances with a solid \a color. The \a paths vector contains offsets to the base path id.
	void strokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
						 const ci::ColorA& color, bool clearStencil = true) const;
	//! Strokes the path instances with a solid \a color. The \a paths vector contains offsets to the base path id.
	void strokeInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat4x3>& transforms,
						 const ci::ColorA& color, bool clearStencil = true) const;

	//! Fills the path with a solid \a color.
	void fill(const ci::ColorA& color, float opacity = 1, bool clearStencil = true) const;
	//! Fills the path with a \a paint, which can be a gradient.
	void fill(const Paint& paint, float opacity = 1, bool clearStencil = true) const;
	//! Fills the path with a \a paint, but uses the \a texture for its colors.
	void fill(const Paint& paint, const ci::gl::TextureRef& texture, float opacity = 1, bool clearStencil = true) const;
	//! Fills the path with a \a texture, automatically centered within the path's bounding box.
	void fill(const ci::gl::TextureRef& texture, float opacity = 1, bool clearStencil = true) const {
		fill(texture, getFillBounds(), opacity, clearStencil);
	}
	//! Fills the path with a \a texture, automatically centered within the specified \a bounding box.
	void fill(const ci::gl::TextureRef& texture, const ci::Rectf& bounds, float opacity = 1,
			  bool clearStencil = true) const;
	//! Fills the path with a \a texture.
	void fill(const ci::gl::TextureRef& texture, const glm::vec2& upperLeftTexCoord,
			  const glm::vec2& lowerRightTexCoord, float opacity = 1, bool clearStencil = true) const;

	//! Fills the path instances with a solid \a color.
	void fillInstanced(const std::vector<glm::mat3x2>& transforms, const ci::ColorA& color,
					   bool clearStencil = true) const;
	//! Fills the path instances with a solid \a color.
	void fillInstanced(const std::vector<glm::mat4x3>& transforms, const ci::ColorA& color,
					   bool clearStencil = true) const;
	//! Fills the path instances with a solid \a color. The \a paths vector contains offsets to the base path id.
	void fillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat3x2>& transforms,
					   const ci::ColorA& color, bool clearStencil = true) const;
	//! Fills the path instances with a solid \a color. The \a paths vector contains offsets to the base path id.
	void fillInstanced(const std::vector<GLuint>& paths, const std::vector<glm::mat4x3>& transforms,
					   const ci::ColorA& color, bool clearStencil = true) const;

	//! Adds the \a other path to our path and returns the result as a new path.
	[[nodiscard]] Path operator+(const Path& other) const;
	//! Subtracts the \a other path from our path, creating a hole and returns the result as a new path.
	[[nodiscard]] Path operator-(const Path& other) const;

	//! Adds the \a other path to our path.
	Path& operator+=(const Path& other);
	//! Subtracts the \a other path from our path, creating a hole.
	Path& operator-=(const Path& other);

	//! Transforms the coordinates of our path.
	void transform(const glm::mat3x2& m) const;
	//! Transforms the coordinates of our path. Assumes matrix \a m is affine.
	void transform(const glm::mat3x3& m) const { transform(toMat3x2(m)); }
	//! Transforms the coordinates of our path. Assumes matrix \a m is affine.
	void transform(const glm::mat4x4& m) const { transform(toMat3x2(m)); }

	//! Returns the result of this path's transformation as a new path.
	[[nodiscard]] Path transformed(const glm::mat3x2& m) const;
	//! Returns the result of this path's transformation as a new path. Assumes matrix \a m is affine.
	[[nodiscard]] Path transformed(const glm::mat3x3& m) const { return transformed(toMat3x2(m)); }
	//! Returns the result of this path's transformation as a new path. Assumes matrix \a m is affine.
	[[nodiscard]] Path transformed(const glm::mat4x4& m) const { return transformed(toMat3x2(m)); }

	//! Translates the path by \a x and \a y.
	void translate(float x, float y) const;
	//! Translates the path by \a offset.
	void translate(const glm::vec2& offset) const { translate(offset.x, offset.y); }

	//! Returns the result of translating this path by \a x and \a y as a new path.
	[[nodiscard]] Path translated(float x, float y) const;
	//! Returns the result of translating this path by \a offset as a new path.
	[[nodiscard]] Path translated(const glm::vec2& offset) const { return translated(offset.x, offset.y); }

	//! Rotates the path \a radians around its origin.
	void rotate(float radians) const;
	//! Rotates the path \a radians around the specified \a center.
	void rotate(float radians, const glm::vec2& center) const;

	//! Returns the result of rotating this path \a radians around its origin as a new path.
	[[nodiscard]] Path rotated(float radians) const;
	//! Returns the result of rotating this path \a radians around the specified \a center as a new path.
	[[nodiscard]] Path rotated(float radians, const glm::vec2& center) const;

	//! Scales the path uniformly.
	void scale(float s) const { scale(s, s); }
	//! Scales the path uniformly around the specified \a center.
	void scale(float s, const glm::vec2& center) const { scale(s, s, center); }
	//! Scales the path.
	void scale(float sx, float sy) const;
	//! Scales the path around the specified \a center.
	void scale(float sx, float sy, const glm::vec2& center) const;
	//! Scales the path.
	void scale(const glm::vec2& s) const { scale(s.x, s.y); }
	//! Scales the path around the specified \a center.
	void scale(const glm::vec2& s, const glm::vec2& center) const;

	//! Returns the result of scaling this path uniformly as a new path.
	[[nodiscard]] Path scaled(float s) const { return scaled(s, s); }
	//! Returns the result of scaling this path as a new path.
	[[nodiscard]] Path scaled(float sx, float sy) const;
	//! Returns the result of scaling this path as a new path.
	[[nodiscard]] Path scaled(const glm::vec2& s) const { return scaled(s.x, s.y); }
	//! Returns the result of scaling this path around the specified \a center as a new path.
	[[nodiscard]] Path scaled(const glm::vec2& s, const glm::vec2& center) const;

	//! Skews the path by \a radiansX.
	void skewX(float radiansX) const;
	//! Skews the path by \a radiansY.
	void skewY(float radiansY) const;
	//! Skews the path by \a radiansX and \a radiansY.
	void skew(float radiansX, float radiansY) const;

	//! Returns the result of skewing this path by \a radiansX as a new path.
	[[nodiscard]] Path skewedX(float radiansX) const;
	//! Returns the result of skewing this path by \a radiansY as a new path.
	[[nodiscard]] Path skewedY(float radiansY) const;
	//! Returns the result of skewing this path by \a radiansX and \a radiansY as a new path.
	[[nodiscard]] Path skewed(float radiansX, float radiansY) const;

	////! Attempts to minimize the number of commands and coordinates used to represent the path.
	// void optimize() const;
	////! Attempts to minimize the number of commands and coordinates used to represent the path and returns the result
	/// as
	////! a new path.
	//[[nodiscard]] Path optimized() const;

	//! Reverses the order of the points and segments, effectively changing the winding.
	void reverse() const;
	//! Reverses the order of the points and segments, effectively changing the winding and returns the result as a new
	//! path.
	[[nodiscard]] Path reversed() const;

	//! Returns whether \a point is inside the fill path.
	bool fillContains(const ci::vec2& point) const;
	//! Returns whether \a point is inside the stroke path.
	bool strokeContains(const ci::vec2& point) const;

	//! Converts the path to Cinder's Shape2d.
	ci::Shape2d toShape2d() const;

  protected:
	friend class ScopedClipping;

	//! Returns the number of coordinates (floats) for the specified command.
	static GLint getCoordCount(GLubyte command);
	//! Returns the offset of the end coordinate (float) for the specified command.
	static GLint getCoordOffset(GLubyte command);

	static GLubyte toPathCommand(ci::Path2d::SegmentType type);

	static void optimizeImpl(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords);
	static void reverseImpl(std::vector<GLubyte>& commands, std::vector<GLfloat>& coords);

	//! Contains path id info for instanced rendering. Using lazy initialization to avoid a rare crash in Debug mode.
	static std::vector<GLuint>& sPaths() {
		thread_local static std::vector<GLuint> paths;
		return paths;
	}

	GLuint mPathId{0};
};

//! Can be used to construct paths from code. No validation is performed whatsoever.
class PathHelper {
	std::vector<GLubyte> mCommands;
	std::vector<GLfloat> mCoords;

  public:
	PathHelper()  = default;
	~PathHelper() = default;

	PathHelper(const PathHelper&)			 = default;
	PathHelper(PathHelper&&)				 = default;
	PathHelper& operator=(const PathHelper&) = default;
	PathHelper& operator=(PathHelper&&)		 = default;

	explicit PathHelper(const Path& path) { path.getPath(mCommands, mCoords); }

	//! Returns the path's commands.
	const std::vector<GLubyte>& getCommands() const { return mCommands; }
	//! Returns the path's coordinates.
	const std::vector<GLfloat>& getCoords() const { return mCoords; }

	ci::vec2 getCurrentPoint() const {
		assert(!mCoords.empty());
		return {mCoords[mCoords.size() - 2], mCoords[mCoords.size() - 1]};
	}

	//! Sets the path's commands.
	void setCommands(const std::vector<GLubyte>& commands) { mCommands = commands; }
	//! Sets the path's commands.
	void setCommands(const GLubyte* commands, size_t count) { mCommands.assign(commands, commands + count); }
	//! Sets the path's coordinates.
	void setCoords(const std::vector<GLfloat>& coords) { mCoords = coords; }
	//! Sets the path's coordinates.
	void setCoords(const GLfloat* coords, size_t count) { mCoords.assign(coords, coords + count); }
	//! Sets the path's commands and coordinates.
	void set(const Path& path);
	//! Removes the last command and its coordinates.
	void removeLastCommand();
	//! Returns whether any two segments of the path intersect.
	[[nodiscard]] bool isSelfIntersecting() const;
	//!
	[[nodiscard]] bool isIntersecting(const PathHelper& other) const;

	void moveTo(const ci::vec2& p) { moveTo(p.x, p.y); }
	void moveTo(float x, float y);
	void lineTo(const ci::vec2& p) { lineTo(p.x, p.y); }
	void lineTo(float x, float y);
	void quadTo(const ci::vec2& p1, const ci::vec2& p2) { quadTo(p1.x, p1.y, p2.x, p2.y); }
	void quadTo(float x1, float y1, float x2, float y2);
	void curveTo(const ci::vec2& p1, const ci::vec2& p2, const ci::vec2& p3) {
		curveTo(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	}
	void curveTo(float x1, float y1, float x2, float y2, float x3, float y3);
	void horizontalLineTo(float x);
	void verticalLineTo(float y);
	void smoothQuadTo(const ci::vec2& p2) { smoothQuadTo(p2.x, p2.y); }
	void smoothQuadTo(float x2, float y2);
	void smoothCurveTo(const ci::vec2& p2, const ci::vec2& p3) { smoothCurveTo(p2.x, p2.y, p3.x, p3.y); }
	void smoothCurveTo(float x2, float y2, float x3, float y3);
	void arcTo(float radiusX, float radiusY, float xAxisRotation, bool large, bool sweep, const ci::vec2& p) {
		arcTo(radiusX, radiusY, xAxisRotation, large, sweep, p.x, p.y);
	}
	void arcTo(float radiusX, float radiusY, float xAxisRotation, bool large, bool sweep, float x, float y);
	void close();

	////! Mimics Cinder's arc method. The arc is approximated by cubic Bezier curves.
	// void arc(const ci::vec2& center, float radius, float startRadians, float endRadians, bool forward = true) {
	//	arc(center.x, center.y, radius, startRadians, endRadians, forward);
	// }
	////! Mimics Cinder's arc method. The arc is approximated by cubic Bezier curves.
	// void arc(float centerX, float centerY, float radius, float startRadians, float endRadians, bool forward = true);
	////! Mimics Cinder's arc method. The arc is approximated by cubic Bezier curves.
	// void arcTo(const ci::vec2& p, const ci::vec2& t, float radius) { arcTo(p.x, p.y, t.x, t.y, radius); }
	////! Mimics Cinder's arc method. The arc is approximated by cubic Bezier curves.
	// void arcTo(float x, float y, float tanX, float tanY, float radius);

	//! Returns the number of coordinates (floats) for the specified command.
	static size_t getCoordCount(GLubyte command);
};

//! Stores shaders so they can be easily reused by other parts of your code.
class Cache {
	std::unordered_map<Shader::Type, ShaderRef> mShaders;

  public:
	static Cache& get() {
		thread_local static Cache instance;
		return instance;
	}

	static ShaderRef loadShader(Shader::Type type);

	static void clean();

	static void clear();

  private:
	Cache() = default;
};

//! Sets up a canvas to render NvPath graphics to.
class Canvas {
  public:
	Canvas(int samples = 8, int coverageSamples = 16, bool useFloats = false);
	Canvas(int width, int height, int samples = 8, int coverageSamples = 16, bool useFloats = false);

	explicit Canvas(const glm::ivec2& size, int samples = 8, int coverageSamples = 16, bool useFloats = false)
	  : Canvas(size.x, size.y, samples, useFloats) {}

	//! Binds the canvas. Please consider using ScopedCanvas instead.
	void bind() const;
	//! Unbinds the canvas. Please consider using ScopedCanvas instead.
	void unbind() const;
	//! Returns whether the canvas is currently bound.
	bool isBound() const { return mIsBound; }

	//! Draws the canvas.
	void draw() const;
	//! Draws the canvas.
	void draw(const ci::Rectf& bounds) const;

	//! Returns the width of the canvas in pixels.
	int32_t getWidth() const { return mWidth; }
	//! Returns the height of the canvas in pixels.
	int32_t getHeight() const { return mHeight; }
	//! Returns the size of the canvas in pixels.
	glm::ivec2 getSize() const { return {mWidth, mHeight}; }
	//! Returns the bounds of the canvas in pixels.
	ci::Area getBounds() const { return {0, 0, mWidth, mHeight}; }

	//! Returns the number of samples used per pixel.
	int getSamples() const { return getFboFormat().getSamples(); }

	//! Returns the canvas texture if it exists, otherwise returns nullptr.
	ci::gl::Texture2dRef getTexture(GLenum attachment = GL_COLOR_ATTACHMENT0) const;
	//! Returns the frame buffer format for this canvas.
	ci::gl::Fbo::Format getFboFormat() const;

	//! Sets the size of the canvas in pixels.
	void resize(const glm::ivec2& size);

  private:
	int32_t					 mWidth{640};		   //
	int32_t					 mHeight{480};		   //
	int						 mSamples{8};		   //
	int						 mCoverageSamples{16}; //
	bool					 mUseFloats{false};	   //
	mutable ci::gl::Context* mCtx = nullptr;	   //
	mutable ci::gl::FboRef	 mFbo;				   //
	mutable bool			 mIsBound = false;	   //
};

class ScopedCanvas {
	const Canvas& mCanvas;

  public:
	explicit ScopedCanvas(const Canvas& canvas)
	  : mCanvas(canvas) {
		mCanvas.bind();
	}
	~ScopedCanvas() { mCanvas.unbind(); }

	ScopedCanvas(const ScopedCanvas&)			 = delete;
	ScopedCanvas(ScopedCanvas&&)				 = delete;
	ScopedCanvas& operator=(const ScopedCanvas&) = delete;
	ScopedCanvas& operator=(ScopedCanvas&&)		 = delete;
};

class ScopedPathRendering {
	ci::gl::Context* mCtx = nullptr;
	ci::ColorAf		 mColor;

  public:
	ScopedPathRendering();
	[[deprecated]] explicit ScopedPathRendering(bool)
	  : ScopedPathRendering() {}
	~ScopedPathRendering();

	ScopedPathRendering(const ScopedPathRendering&)			   = delete;
	ScopedPathRendering(ScopedPathRendering&&)				   = delete;
	ScopedPathRendering& operator=(const ScopedPathRendering&) = delete;
	ScopedPathRendering& operator=(ScopedPathRendering&&)	   = delete;
};

//! Helper for enabling a clip path.
class ScopedClipPath {
  public:
	ScopedClipPath(const Path& mask, GLuint stencilMask = 0xFF, bool showMask = false);

	ScopedClipPath(const std::initializer_list<Path>& masks, GLuint stencilMask = 0xFF, bool showMask = false);

	~ScopedClipPath();

	ScopedClipPath(const ScopedClipPath&)			 = delete;
	ScopedClipPath(ScopedClipPath&&)				 = delete;
	ScopedClipPath& operator=(const ScopedClipPath&) = delete;
	ScopedClipPath& operator=(ScopedClipPath&&)		 = delete;

  private:
	size_t mPathCount;
};

//! Helper for setting up the stencil buffer for path rendering, taking clipping into account.
class ScopedStencilState {
	ci::gl::Context* mCtx = nullptr;
	GLuint			 mBitMask;

  public:
	ScopedStencilState(bool isPathRendering = true, bool invertMask = false)
	  : ScopedStencilState(sClipPaths().size(), isPathRendering, invertMask) {}
	ScopedStencilState(size_t clipCount, bool isPathRendering = true, bool invertMask = false);
	~ScopedStencilState();

	ScopedStencilState(const ScopedStencilState&)			 = delete;
	ScopedStencilState(ScopedStencilState&&)				 = delete;
	ScopedStencilState& operator=(const ScopedStencilState&) = delete;
	ScopedStencilState& operator=(ScopedStencilState&&)		 = delete;

	GLuint getBitMask() const { return mBitMask; }
};

//! Helper for setting up the stencil buffer for covering stenciled content.
class ScopedCover {
	ci::gl::Context* mCtx = nullptr;

  public:
	ScopedCover(bool clearStencil = true);
	~ScopedCover();

	ScopedCover(const ScopedCover&)			   = delete;
	ScopedCover(ScopedCover&&)				   = delete;
	ScopedCover& operator=(const ScopedCover&) = delete;
	ScopedCover& operator=(ScopedCover&&)	   = delete;
};

inline Path circle(float x, float y, float r) {
	std::stringstream ss;

	ss << 'M' << +(x + r) << ',' << y;
	ss << 'a' << r << ',' << r << ',' << 0 << ',' << false << ',' << true << ',' << -(r + r) << ',' << 0;
	ss << 'a' << r << ',' << r << ',' << 0 << ',' << false << ',' << true << ',' << +(r + r) << ',' << 0;
	ss << 'Z';

	return Path{ss.str()};
}

inline Path ellipse(float x, float y, float rx, float ry) {
	std::stringstream ss;

	ss << 'M' << +(x + rx) << ',' << y;
	ss << 'a' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << -(rx + rx) << ',' << 0;
	ss << 'a' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << +(rx + rx) << ',' << 0;
	ss << 'Z';

	return Path{ss.str()};
}

inline Path arc(float cx, float cy, float rx, float ry, float angle, bool closed = true) {
	angle = glm::sign(angle) * glm::min(glm::abs(angle), glm::radians(360.0f) - 1.0e-5f);

	std::stringstream ss;

	const float s	  = sin(angle);
	const float c	  = cos(angle);
	const bool	large = glm::abs(angle) > glm::pi<float>();
	const bool	sweep = angle > 0;

	ss << 'M' << +(cx + rx) << ',' << cy;
	ss << 'A' << rx << ',' << ry << ',' << 0 << ',' << large << ',' << sweep << ',' << +(cx + c * rx) << ','
	   << +(cy + s * ry);

	if (closed) {
		ss << 'L' << cx << ',' << cy;
		ss << 'Z';
	}

	return Path{ss.str()};
}

inline Path line(float x1, float y1, float x2, float y2) {
	std::stringstream ss;

	ss << 'M' << x1 << ',' << y1;
	ss << 'L' << x2 << ',' << y2;

	return Path{ss.str()};
}

inline Path line(const glm::vec2& a, const glm::vec2& b) {
	return line(a.x, a.y, b.x, b.y);
}

inline Path polygon(const glm::vec2* points, size_t count, bool closed) {
	std::stringstream ss;

	if (count > 1) {
		ss << 'M' << points[0].x << ',' << points[0].y;
		for (size_t i = 1; i < count; ++i)
			ss << 'L' << points[i].x << ',' << points[i].y;
		if (closed) ss << 'Z';
	}

	return Path{ss.str()};
}

inline Path polygon(const std::vector<glm::vec2>& points, bool closed) {
	return polygon(points.data(), points.size(), closed);
}

inline Path polygon(const std::vector<float>& points, bool closed) {
	return polygon(reinterpret_cast<const glm::vec2*>(points.data()), points.size() / 2, closed);
}

//! Returns a path with rounded corners. Define your path in CW order for this to work correctly.
inline Path roundedPolygon(const glm::vec2* points, size_t count, float r, bool closed) {
	std::stringstream ss;

	// See: https://stackoverflow.com/a/24780108/858219
	if (count > 1) {
		for (size_t i = 0; i < count; ++i) {
			size_t im = (i + count - 1) % count; // i - 1
			size_t ip = (i + 1) % count;		 // i + 1
			if (i == 0 && !closed) {
				// No rounding for the first segment.
				ss << 'M' << points[i].x << ',' << points[i].y;
			} else if (i + 1 < count || closed) {
				auto a = mix(points[i], points[im], 0.5f);
				auto b = points[i];
				auto c = mix(points[i], points[ip], 0.5f);

				auto angle	 = glm::atan(c.y - b.y, c.x - b.x) - glm::atan(a.y - b.y, a.x - b.x);
				auto radius	 = r;
				auto segment = radius / glm::abs(glm::tan(angle / 2.0f));

				auto la = distance(points[i], points[im]);
				auto lc = distance(points[i], points[ip]);
				if (segment > la || segment > lc) {
					segment = glm::min(la, lc);
					radius	= segment * glm::abs(glm::tan(angle / 2.0f));
				}

				a = mix(points[i], points[im], segment / la);
				c = mix(points[i], points[ip], segment / lc);

				ss << (i == 0 ? 'M' : 'L') << a.x << ',' << a.y;
				ss << 'A' << radius << ',' << radius << ',' << 0 << ',' << false << ',' << true << ',' << c.x << ','
				   << c.y;

			} else {
				// No rounding for the last segment.
				ss << 'L' << points[i].x << ',' << points[i].y;
			}
		}

		if (closed) ss << 'Z';
	}

	return Path{ss.str()};
}

inline Path roundedPolygon(const std::vector<glm::vec2>& points, float r, bool closed) {
	return roundedPolygon(points.data(), points.size(), r, closed);
}

inline Path roundedPolygon(const std::vector<float>& points, float r, bool closed) {
	return roundedPolygon(reinterpret_cast<const glm::vec2*>(points.data()), points.size() / 2, r, closed);
}

inline Path rectangle(float x, float y, float width, float height) {
	std::stringstream ss;

	ss << 'M' << x << ',' << y;
	ss << 'L' << x + width << ',' << y;
	ss << 'L' << x + width << ',' << y + height;
	ss << 'L' << x << ',' << y + height;
	ss << 'Z';

	return Path{ss.str()};
}

inline Path rectangle(const ci::Rectf& bounds) {
	return rectangle(bounds.x1, bounds.y1, bounds.getWidth(), bounds.getHeight());
}

inline Path roundedRectangle(float x, float y, float width, float height, float rx, float ry) {
	std::stringstream ss;

	ss << 'M' << x + rx << ',' << y;
	ss << 'L' << x + width - rx << ',' << y;
	ss << 'A' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << x + width << ',' << y + ry;
	ss << 'L' << x + width << ',' << y + height - ry;
	ss << 'A' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << x + width - rx << ','
	   << y + height;
	ss << 'L' << x + rx << ',' << y + height;
	ss << 'A' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << x << ',' << y + height - ry;
	ss << 'L' << x << ',' << y + ry;
	ss << 'A' << rx << ',' << ry << ',' << 0 << ',' << false << ',' << true << ',' << x + rx << ',' << y;
	ss << 'Z';

	return Path{ss.str()};
}

inline Path roundedRectangle(const ci::Rectf& bounds, float rx, float ry) {
	return roundedRectangle(bounds.x1, bounds.y1, bounds.getWidth(), bounds.getHeight(), rx, ry);
}

inline Path star(float cx, float cy, float rmax, float rmin, float points, float angle) {
	std::stringstream ss;

	const float step = glm::radians(180.0f / glm::round(points));
	for (float i = 0.f; i < glm::radians(360.0f);) {
		float x = cx + rmax * glm::sin(angle + i);
		float y = cy - rmax * glm::cos(angle + i);
		if (ss.str().empty())
			ss << 'M' << x << ',' << y;
		else
			ss << 'L' << x << ',' << y;
		i += step;
		x = cx + rmin * glm::sin(angle + i);
		y = cy - rmin * glm::cos(angle + i);
		ss << 'L' << x << ',' << y;
		i += step;
	}

	ss << 'Z';

	return Path{ss.str()};
}

inline Path arrow(const glm::vec2& p0, const glm::vec2& p1, float thickness, float width = 4, float length = 4,
				  float concavity = 0) {
	std::stringstream ss;

	const auto dist		 = glm::distance(p0, p1);
	const auto direction = (p1 - p0) / dist;
	const auto normal	 = glm::vec2(-direction.y, direction.x) * 0.5f * thickness;
	const auto base		 = p0 + direction * glm::max(0.0f, dist - thickness * length);
	const auto size		 = thickness * length * concavity * direction;

	ss << 'M' << p0.x - normal.x << ',' << p0.y - normal.y;
	ss << 'L' << base.x - normal.x + size.x << ',' << base.y - normal.y + size.y;
	ss << 'L' << base.x - normal.x * width << ',' << base.y - normal.y * width;
	ss << 'L' << p1.x << ',' << p1.y;
	ss << 'L' << base.x + normal.x * width << ',' << base.y + normal.y * width;
	ss << 'L' << base.x + normal.x + size.x << ',' << base.y + normal.y + size.y;
	ss << 'L' << p0.x + normal.x << ',' << p0.y + normal.y;
	ss << 'Z';

	return Path{ss.str()};
}

} // namespace nvpath