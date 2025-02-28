#pragma once

#include <ds/ui/sprite/sprite.h>

#include <nvpath/NvPath.h>
#include <nvpath/NvPathSvg.h>

namespace ds::ui {

class PathSpriteShadow;

class PathSprite : public Sprite {
  public:
	PathSprite(SpriteEngine& engine);

	~PathSprite() override;

	PathSprite(const PathSprite&)			 = delete;
	PathSprite(PathSprite&&)				 = delete;
	PathSprite& operator=(const PathSprite&) = delete;
	PathSprite& operator=(PathSprite&&)		 = delete;

	bool contains(const ci::vec3& point, float pad) const override;

	/// Returns whether global coordinate \a point is inside the fill bounds.
	bool isPointInsideFill(const ci::vec3& point) const;
	/// Returns whether the global coordinate \a point is inside the stroke bounds.
	bool isPointInsideStroke(const ci::vec3& point) const;

	/// Returns the minimum size of the bounding box. If set to 0, the bounding box will be calculated based on the
	/// path.
	const ci::vec2& getTouchSize() const { return mTouchSize; }
	/// Sets the minimum touch size. If set to 0, the bounding box will be calculated based on the
	/// path.
	virtual void setTouchSize(float size) { mTouchSize.x = mTouchSize.y = size; }
	/// Sets the minimum touch size. If set to 0, the bounding box will be calculated based on the
	/// path.
	virtual void setTouchSize(const ci::vec2& size) { mTouchSize = size; }

	/// Clones the provided \a path.
	virtual void setPath(const nvpath::Path& path);
	/// Accepts the provided \a path.
	virtual void setPath(nvpath::Path&& path);
	/// Accepts an SVG \a path definition.
	virtual void setPath(const std::string& path);
	/// Accepts a shape definition, e.g. "circle( 50, 50, 20 )".
	virtual void setShape(const std::string& shape);
	/// Sets the fill color. Accepts a color like "#ff0000", a name like "white" or a file name like
	/// "%APP%/data/images/texture.png".
	virtual void setFill(const std::string& fill);
	/// Sets the fill color or gradient.
	virtual void setFill(const nvpath::Paint& fill);
	/// Sets the fill color.
	virtual void setFillColor(const ci::ColorA8u& color);
	/// Sets the stroke color. Accepts a color like "#ff0000" or a name like "white".
	virtual void setStroke(const std::string& color);
	/// Sets the stroke color or gradient.
	virtual void setStroke(const nvpath::Paint& fill);
	/// Sets the stroke color.
	virtual void setStrokeColor(const ci::ColorA8u& color);
	/// Returns the stroke width in pixels.
	float getStrokeWidth() const { return mStrokeWidth; }
	/// Sets the stroke width in pixels.
	virtual void setStrokeWidth(float width) {
		mStrokeWidth = width;
		if (mPath.getId()) mPath.setStrokeWidth(width);
	}
	/// Sets the dash caps style.
	virtual void setDashCaps(nvpath::CapsStyle caps) {
		mDashCapsInitial = mDashCapsTerminal = caps;
		if (mPath.getId()) mPath.setDashCaps(caps);
	}
	/// Sets the dash caps style separately for the \a initial and \a terminal caps.
	virtual void setDashCaps(nvpath::CapsStyle initial, nvpath::CapsStyle terminal) {
		mDashCapsInitial  = initial;
		mDashCapsTerminal = terminal;
		if (mPath.getId()) mPath.setDashCaps(initial, terminal);
	}
	/// Sets the end caps style.
	virtual void setEndCaps(nvpath::CapsStyle caps) {
		mEndCapsInitial = mEndCapsTerminal = caps;
		if (mPath.getId()) mPath.setEndCaps(caps);
	}
	/// Sets the end caps style separately for the \a initial and \a terminal caps.
	virtual void setEndCaps(nvpath::CapsStyle initial, nvpath::CapsStyle terminal) {
		mEndCapsInitial	 = initial;
		mEndCapsTerminal = terminal;
		if (mPath.getId()) mPath.setEndCaps(initial, terminal);
	}
	/// Sets the line caps style. Accepts a string like "butt", "round" or "square".
	virtual void setLineCap(std::string def) {
		to_lowercase(def);
		if (def == "round") {
			setDashCaps(nvpath::CapsStyle::ROUND);
			setEndCaps(nvpath::CapsStyle::ROUND);
		} else if (def == "square") {
			setDashCaps(nvpath::CapsStyle::SQUARE);
			setEndCaps(nvpath::CapsStyle::SQUARE);
		} else {
			setDashCaps(nvpath::CapsStyle::DEFAULT);
			setEndCaps(nvpath::CapsStyle::DEFAULT);
		}
	}
	/// Sets the line join style.
	virtual void setJoinStyle(nvpath::JoinStyle joins) {
		mJoinStyle = joins;
		if (mPath.getId()) mPath.setJoinStyle(joins);
	}
	/// Sets the line join style. Accepts a string like "miter", "miter-clip", "round" or "bevel".
	virtual void setLineJoin(std::string def) {
		to_lowercase(def);
		if (def == "miter") {
			setJoinStyle(nvpath::JoinStyle::MITER_REVERT);
		} else if (def == "miter-clip") {
			setJoinStyle(nvpath::JoinStyle::MITER_TRUNCATE);
		} else if (def == "round") {
			setJoinStyle(nvpath::JoinStyle::ROUND);
		} else if (def == "bevel") {
			setJoinStyle(nvpath::JoinStyle::BEVEL);
		} else {
			setJoinStyle(nvpath::JoinStyle::DEFAULT);
		}
	}
	/// Sets the dash pattern. Accepts a string of floats, e.g. "2.0 1.5".
	virtual void setDashArray(const std::string& def) {
		const char* sInOut = def.c_str();
		parseDashArray(&sInOut);
	}
	/// Sets the dash pattern.
	virtual void setDashPattern(const std::vector<float>& pattern) {
		if (mPath.getId()) mPath.setDashPattern(pattern);
	}

	/// Enables shadow rendering.
	void setShadow(float offsetX, float offsetY, int scale);
	/// Enables shadow rendering and sets the shadow parameters: offsetX, offsetY, scale.
	void setShadow(const ci::vec3& params) { setShadow(params.x, params.y, static_cast<int>(params.z)); }
	/// Enables shadow rendering (using default settings if not yet enabled) and sets the shadow color.
	void setShadowColor(const ci::ColorA& color);
	/// Enables shadow rendering (using default settings if not yet enabled) and sets the blur parameters: standard
	/// deviation sigma, kernel size.
	void setShadowBlur(double sigma, int kernelSize);

	void drawLocalClient() override;

	void fitInsideArea(const ci::Rectf& area) override;

  protected:
	// Calculates the bounds of the path.
	ci::Rectf calcBounds() const;

	// Uses the LoadImageService to load the image.
	void loadImage(const std::string& filename, int flags = Image::IMG_CACHE_F | Image::IMG_ENABLE_MIPMAP_F);

	// Accepts a string like: "circle( x, y, r )".
	void parseCircle(const char** sInOut);
	// Accepts a string like: "ellipse( x, y, rx, ry )".
	void parseEllipse(const char** sInOut);
	// Accepts a string like: "line( x1, y1, x2, y2 )".
	void parseLine(const char** sInOut);
	// Accepts a string like: "polygon( x1, y1, x2, y2, ... )".
	void parsePolygon(const char** sInOut);
	// Accepts a string like: "rectangle( x, y, width, height )", "rectangle( x, y, width, height, r )" or
	// "rectangle( x, y, width, height, rx, ry )".
	void parseRectangle(const char** sInOut);
	// Accepts a string like: "star( x, y, rmax, rmin, points, angle )".
	void parseStar(const char** sInOut);
	// Accepts a string of floats, separated by white space or comma's.
	void parseDashArray(const char** sInOut);

	//
	void onPaintChanged() { setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture); }

	//
	static std::string fetchParameters(const char** sInOut);

	nvpath::Path					  mPath;										 //
	nvpath::CapsStyle				  mDashCapsInitial{nvpath::CapsStyle::DEFAULT};	 //
	nvpath::CapsStyle				  mDashCapsTerminal{nvpath::CapsStyle::DEFAULT}; //
	nvpath::CapsStyle				  mEndCapsInitial{nvpath::CapsStyle::DEFAULT};	 //
	nvpath::CapsStyle				  mEndCapsTerminal{nvpath::CapsStyle::DEFAULT};	 //
	nvpath::JoinStyle				  mJoinStyle{nvpath::JoinStyle::DEFAULT};		 //
	nvpath::Paint					  mFill{nvpath::Paint::NONE};					 //
	nvpath::Paint					  mStroke{nvpath::Paint::NONE};					 //
	float							  mStrokeWidth{1};								 //
	std::string						  mFilename;									 //
	ci::gl::TextureRef				  mTexture;										 // Image used to fill the path.
	ci::vec2						  mTouchSize{0}; // Extra padding for touch detection.
	ci::Rectf						  mBounds;		 // Cached bounds.
	int								  mFlags{0};	 // Image loading flags.
	std::unique_ptr<PathSpriteShadow> mShadow;		 //
};

class PathSpriteShadow {
  public:
	PathSpriteShadow()
	  : PathSpriteShadow(60, 60) {}

	PathSpriteShadow(float offsetX, float offsetY, int softness = 1);

	void setShadow(float offsetX, float offsetY, int softness);

	void setShadow(const ci::vec3& params) { setShadow(params.x, params.y, static_cast<int>(params.z)); }

	void setColor(const ci::ColorA& color);

	void setBlur(double sigma, int kernelSize);

	void render(const nvpath::Path& path);

	void draw(const ci::vec2& offset, float opacity = 1) const;

  private:
	ci::gl::TextureRef mTexture;			  //
	ci::ColorA		   mColor{0, 0, 0, 0.5f}; //
	ci::vec2		   mOffset{60, 60};		  //
	ci::ivec2		   mPadding{0};			  //
	int				   mScale{1};			  // Higher values result in blurrier shadows and improved memory usage.
	int mKernelSize{0}; // Must be an odd number. If less than or equal to 0, it is calculated based on the
						// standard deviation.
	double									mSigma{6};	   // Standard deviation.
	thread_local static ci::gl::GlslProgRef sShadowShader; //

	static const char* sVertShader;
	static const char* sFragShader;
};

} // namespace ds::ui