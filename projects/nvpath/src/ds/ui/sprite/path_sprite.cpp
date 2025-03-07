#include "stdafx.h"

#include "path_sprite.h"

#include <ds/ui/effect/effect.h>

#include <ds/ui/service/load_image_service.h>
#include <ds/util/color_util.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/float_util.h>
#include <ds/util/string_util.h>

#include <cinder/Utilities.h>
#include <cinder/gl/Fbo.h>
#include <cinder/gl/Texture.h>

namespace {

// Add the 'path' sprite type so we can use it in our layout XML.
auto INIT = []() {
	ds::App::AddStartup("PathSprite", [](ds::Engine& e) {
		using namespace ds::ui;

		// Register our custom sprite(s).
		e.registerSpriteImporter("path", [](SpriteEngine& enginey) -> Sprite* { return new PathSprite(enginey); });

		// Register the properties for our custom sprites.
		e.registerSpritePropertySetter<PathSprite>(
			"d", [](PathSprite& path, const std::string& theValue, const std::string&) { path.setPath(theValue); });
		e.registerSpritePropertySetter<PathSprite>("shape", [](PathSprite& path, const std::string& theValue,
															   const std::string&) { path.setShape(theValue); });
		e.registerSpritePropertySetter<PathSprite>(
			"fill", [](PathSprite& path, const std::string& theValue, const std::string& referer) {
				if (ci::fs::path(theValue).has_extension()) {
					const auto filename = ds::filePathRelativeTo(referer, theValue);
					path.setFill(filename);
				} else
					path.setFill(theValue);
			});
		e.registerSpritePropertySetter<PathSprite>("stroke", [](PathSprite& path, const std::string& theValue,
																const std::string&) { path.setStroke(theValue); });
		e.registerSpritePropertySetter<PathSprite>(
			"stroke-width", [](PathSprite& path, const std::string& theValue, const std::string&) {
				path.setStrokeWidth(ds::string_to_float(theValue));
			});

		e.registerSpritePropertySetter<PathSprite>(
			"stroke-dasharray",
			[](PathSprite& path, const std::string& theValue, const std::string&) { path.setDashArray(theValue); });

		e.registerSpritePropertySetter<PathSprite>(
			"stroke-linecap",
			[](PathSprite& path, const std::string& theValue, const std::string&) { path.setLineCap(theValue); });

		e.registerSpritePropertySetter<PathSprite>(
			"stroke-linejoin",
			[](PathSprite& path, const std::string& theValue, const std::string&) { path.setLineJoin(theValue); });

		e.registerSpritePropertySetter<PathSprite>(
			"shadow", [](PathSprite& path, const std::string& theValue, const std::string&) {
				auto params = ds::parseVector(theValue);
				path.setShadow(params);
			});

		e.registerSpritePropertySetter<PathSprite>(
			"shadow-color", [](PathSprite& path, const std::string& theValue, const std::string&) {
				path.setShadowColor(ds::parseColor(theValue, path.getEngine()));
			});

		e.registerSpritePropertySetter<PathSprite>(
			"shadow-blur", [](PathSprite& path, const std::string& theValue, const std::string&) {
				auto params = ds::parseVector(theValue);
				path.setShadowBlur(params.x, params.y);
			});
	});
	return true;
}();

} // namespace

using namespace ci;

namespace ds::ui {

PathSprite::PathSprite(SpriteEngine& engine)
  : Sprite(engine) {}

PathSprite::~PathSprite() {
	mEngine.getLoadImageService().release(mFilename, this);
}

bool PathSprite::contains(const vec3& point, float) const {
	const auto local = globalToLocal(point);
	return mBounds.contains(local);
}

bool PathSprite::isPointInsideFill(const vec3& point) const {
	const vec2 local = globalToLocal(point);
	return mPath.fillContains(local);
}

bool PathSprite::isPointInsideStroke(const vec3& point) const {
	const vec2 local = globalToLocal(point);
	return mPath.strokeContains(local);
}

void PathSprite::setPath(const nvpath::Path& path) {
	mPath = path;
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();

	mShadowDirty = true;
}

void PathSprite::setPath(nvpath::Path&& path) {
	mPath = std::move(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();

	mShadowDirty = true;
}

void PathSprite::setPath(const std::string& path) {
	mPath = nvpath::Path(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();

	mShadowDirty = true;
}

void PathSprite::setShape(const std::string& shape) {
	mShape		 = shape;
	mShadowDirty = true;

	try {
		const char* sInOut = shape.c_str();
		skipSpace(&sInOut);
		if (strncmp(sInOut, "circle", 6) == 0) {
			sInOut += 6;
			parseCircle(&sInOut);
		} else if (strncmp(sInOut, "ellipse", 7) == 0) {
			sInOut += 7;
			parseEllipse(&sInOut);
		} else if (strncmp(sInOut, "line", 4) == 0) {
			sInOut += 4;
			parseLine(&sInOut);
		} else if (strncmp(sInOut, "polygon", 7) == 0) {
			sInOut += 7;
			parsePolygon(&sInOut);
		} else if (strncmp(sInOut, "rectangle", 9) == 0) {
			sInOut += 9;
			parseRectangle(&sInOut);
		} else if (strncmp(sInOut, "star", 4) == 0) {
			sInOut += 4;
			parseStar(&sInOut);
		}
	} catch (const std::exception& exc) {
		DS_LOG_ERROR(exc.what());
	}
}

void PathSprite::setFill(const std::string& fill) {
	if (fs::is_regular_file(fill) && fs::exists(fill)) {
		mFill = {};
		loadImage(fill);
	} else if (!asciiCaseCmp(fill.c_str(), "none"))
		mFill = {};
	else
		mFill = nvpath::Paint::color(parseColor(fill, mEngine));
	onPaintChanged();
}

void PathSprite::setFill(const nvpath::Paint& fill) {
	mFill = fill;
	onPaintChanged();
}

void PathSprite::setFillColor(const ColorA8u& color) {
	mFill = nvpath::Paint::color(color);
	onPaintChanged();
}

void PathSprite::setStroke(const std::string& color) {
	if (!asciiCaseCmp(color.c_str(), "none"))
		mStroke = {};
	else
		mStroke = nvpath::Paint::color(parseColor(color, mEngine));
	onPaintChanged();
}

void PathSprite::setStroke(const nvpath::Paint& fill) {
	mStroke = fill;
	onPaintChanged();
}

void PathSprite::setStrokeColor(const ColorA8u& color) {
	mStroke = nvpath::Paint::color(color);
	onPaintChanged();
}

void PathSprite::setLineCap(std::string def) {
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

void PathSprite::setLineJoin(std::string def) {
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

void PathSprite::setShadow(const float offsetX, const float offsetY, const int scale) {
	if (!mShadow)
		mShadow = std::make_unique<PathSpriteShadow>(offsetX, offsetY, scale);
	else
		mShadow->setShadow(offsetX, offsetY, scale);
	mShadowDirty = true;
}

void PathSprite::setShadowColor(const ColorA& color) {
	if (!mShadow) mShadow = std::make_unique<PathSpriteShadow>(60.0f, 60.0f, 1);
	mShadow->setColor(color);
}

void PathSprite::setShadowBlur(double sigma, int kernelSize) {
	if (!mShadow) mShadow = std::make_unique<PathSpriteShadow>(60.0f, 60.0f, 1);
	mShadow->setBlur(sigma, kernelSize);
	mShadowDirty = true;
}

void PathSprite::drawLocalClient() {
	// Get render opacity.
	const auto opacity = getOpacity() * getDrawOpacity();
	if (approxZero(opacity)) return;

	// Enable path rendering.
	nvpath::ScopedPathRendering sp;

	// Render drop shadow, but only where there is no path.
	if (mShadowEnabled && mShadow) {
		if (mShadowDirty) mShadow->render(mPath);

		nvpath::ScopedClipPath	   scpClip(mPath);
		nvpath::ScopedStencilState scpStencilState(false, true);
		mShadow->draw(mPath.getStrokeBounds().getUpperLeft(), opacity);

		mShadowDirty = false;
	}

	// Render path.
	if (mTexture)
		mPath.fill(mTexture, mPath.getFillBounds(), opacity);
	else
		mPath.fill(mFill, opacity);
	mPath.stroke(mStroke, opacity);
}

void PathSprite::fitInsideArea(const Rectf& area) {
	if (!mPath.getId()) return;

	const auto fit = mFit.calcTransform(area, mBounds, false);
	setScale(fit[0][0], fit[1][1]);
	setPosition(fit[2]);
}

Rectf PathSprite::calcBounds() const {
	auto bounds = mStroke.isNone() ? mPath.getFillBounds() : mPath.getStrokeBounds();
	auto size	= vec2{glm::max(mTouchSize.x, bounds.getWidth()), glm::max(mTouchSize.y, bounds.getHeight())};
	bounds.inflate(0.5f * (size - bounds.getSize()));
	return bounds;
}

void PathSprite::loadImage(const std::string& filename, int flags) {
	if (mFilename == filename && mFlags == flags) return;

	setTransparent(true); // Hide while loading.

	mEngine.getLoadImageService().release(mFilename, this);

	if (filename.find("http") == 0)
		mFilename = filename;
	else
		mFilename = Environment::expand(filename);

	mFlags = flags;

	mEngine.getLoadImageService().acquire(
		filename, flags, this, [this](ci::gl::TextureRef tex, Rectf coords, bool error, const std::string& errorMsg) {
			mTexture = std::move(tex);
			if (error) {
				DS_LOG_WARNING("Failed to load PathSprite texture: " << errorMsg);
			}
			onPaintChanged();
		});
}

// Expects either "circle" or "circle( x, y, r )"
void PathSprite::parseCircle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);

	float		x, y, r;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		x = mPosition.x + 0.5f * getWidth();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		y = mPosition.y + 0.5f * getHeight();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		r = parseFloat(&s);
	else
		r = 0.5f * glm::min(getWidth(), getHeight());

	setPath(nvpath::circle(x, y, r));
}

// Expects either "ellipse" or "ellipse( x, y, rx, ry )"
void PathSprite::parseEllipse(const char** sInOut) {
	const auto params = fetchParameters(sInOut);

	float		x, y, rx, ry;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		x = mPosition.x + 0.5f * getWidth();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		y = mPosition.y + 0.5f * getHeight();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		rx = parseFloat(&s);
	else
		rx = 0.5f * getWidth();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		ry = parseFloat(&s);
	else
		ry = 0.5f * getHeight();

	setPath(nvpath::ellipse(x, y, rx, ry));
}

// Expects "line( x1, y1, x2, y2 )"
void PathSprite::parseLine(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x1, y1, x2, y2 )");

	float		x1, y1, x2, y2;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x1 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y1 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		x2 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y2 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");

	setPath(nvpath::line(x1, y1, x2, y2));
}

// Expects "polygon( x1, y1, x2, y2, ... )"
void PathSprite::parsePolygon(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x1, y1, x2, y2, ... )");

	std::vector<float> points;
	const char*		   s = params.c_str();
	while (*s) {
		if (isNumeric(*s))
			points.push_back(parseFloat(&s));
		else
			throw std::runtime_error("Expected ( x1, y1, x2, y2, ... )");
		skipSpaceOrComma(&s);
	}

	if (points.size() % 2 != 0) throw std::runtime_error("Expected ( x1, y1, x2, y2, ... )");

	setPath(nvpath::polygon(reinterpret_cast<const vec2*>(points.data()), points.size() / 2, true));
}

// Expects either "rectangle" or "rectangle( x, y, w, h, rx, ry )"
void PathSprite::parseRectangle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);

	float		x, y, w, h, rx, ry;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		x = mPosition.x;
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		y = mPosition.y;
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		w = parseFloat(&s);
	else
		w = getWidth();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		h = parseFloat(&s);
	else
		h = getHeight();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		rx = parseFloat(&s);
	else
		rx = getCornerRadius();
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		ry = parseFloat(&s);
	else
		ry = rx;

	if (approxZero(rx) || approxZero(ry))
		setPath(nvpath::rectangle(x, y, w, h));
	else
		setPath(nvpath::roundedRectangle(x, y, w, h, rx, ry));
}

void PathSprite::parseStar(const char** sInOut) {
	const auto innerRadius = [](float m, float n) -> float {
		const auto f = glm::pi<float>() / n;
		const auto s = m * f;
		return glm::cos(s) / glm::cos(s - f);
	};

	float points = 5;
	float x		 = 0.5f * getWidth();
	float y		 = 0.5f * getHeight();
	float r1	 = 0.5f * glm::max(getWidth(), getHeight());
	float r2	 = r1 * innerRadius(2, points);
	float angle	 = 0;

	const auto params = fetchParameters(sInOut);

	const char* s	   = params.c_str();
	const auto	values = fetchFloats(&s);
	switch (values.size()) {
	case 0:
		break;
	case 1:
		points = values[0];
		break;
	case 2:
		points = values[0];
		angle  = values[1];
		break;
	case 4:
		x	   = values[0];
		y	   = values[1];
		r1	   = values[2];
		points = values[3];
		r2	   = r1 * innerRadius(2, points);
		break;
	case 5:
		x	   = values[0];
		y	   = values[1];
		r1	   = values[2];
		points = values[3];
		angle  = values[4];
		r2	   = r1 * innerRadius(2, points);
		break;
	case 6:
		x	   = values[0];
		y	   = values[1];
		r1	   = values[2];
		r2	   = values[3];
		points = values[4];
		angle  = values[5];
		break;
	default:
		throw std::runtime_error("Expected ( x, y, r1, r2, points, angle ), (x, y, radius, points, angle), (x, y, "
								 "radius, points), ( points, angle ), ( points ) or ()");
	}

	setPath(nvpath::star(x, y, r1, r2, points, angle));
}

void PathSprite::parseDashArray(const char** sInOut) {
	std::vector<float> pattern;
	pattern.reserve(16);
	while (**sInOut) {
		skipSpaceOrComma(sInOut);
		while (**sInOut && !isNumeric(**sInOut))
			++(*sInOut);
		if (**sInOut && isNumeric(**sInOut)) pattern.push_back(parseFloat(sInOut));
	}
	setDashPattern(pattern);
}

void PathSprite::onPositionChanged() {
	setShape(mShape);
}

void PathSprite::onSizeChanged() {
	setShape(mShape);
}

std::string PathSprite::fetchParameters(const char** sInOut) {
	std::string params;

	skipSpace(sInOut);
	if (**sInOut == '(') {
		++*sInOut;
		skipSpace(sInOut);
		params = fetchUntil(sInOut, ')');
		if (**sInOut) ++*sInOut;
	}

	return trim(params);
}

std::vector<float> PathSprite::fetchFloats(const char** sInOut) {
	std::vector<float> floats;
	while (**sInOut) {
		skipSpaceOrComma(sInOut);
		if (**sInOut && isNumeric(**sInOut))
			floats.push_back(parseFloat(sInOut));
		else
			break;
	}
	return floats;
}

const char* PathSpriteShadow::sVertShader = "#version 150\n"
											"uniform mat4 ciModelViewProjection;"
											"in vec4 ciPosition;"
											"in vec4 ciColor;"
											"in vec2 ciTexCoord0;"
											"out vec2 vertTexCoord;"
											"out vec4 vertColor;"
											"void main(void) {"
											"    vertTexCoord = ciTexCoord0;"
											"    vertColor = ciColor;"
											"    gl_Position = ciModelViewProjection * ciPosition;"
											"}";

const char* PathSpriteShadow::sFragShader = "#version 150\n"
											"uniform sampler2D uInput;"
											"in vec2 vertTexCoord;"
											"in vec4 vertColor;"
											"out vec4 fragColor;"
											"void main(void) {"
											"    fragColor = vertColor;"
											"    const float gamma = 1.5;\n"
											"    fragColor *= pow( texture( uInput, vertTexCoord ).r, gamma );\n"
											"}";

thread_local ci::gl::GlslProgRef PathSpriteShadow::sShadowShader;

PathSpriteShadow::PathSpriteShadow(float offsetX, float offsetY, int softness) {
	mOffset.x = offsetX;
	mOffset.y = offsetY;
	mScale	  = glm::max(1, softness);
}

void PathSpriteShadow::setShadow(float offsetX, float offsetY, int softness) {
	mOffset.x = offsetX;
	mOffset.y = offsetY;
	mScale	  = glm::max(1, softness);
}

void PathSpriteShadow::setColor(const ColorA& color) {
	mColor = color;
}

void PathSpriteShadow::setBlur(double sigma, int kernelSize) {
	mBlur.setSigma(sigma, kernelSize);
}

void PathSpriteShadow::render(const nvpath::Path& path) {
	if (!path.getId()) return;

	// Construct drop shadow.
	mPadding = ivec2(mBlur.getKernelSize());

	const auto dimensions	 = ivec2(path.getStrokeBounds().getSize()) / mScale + 2 * mPadding;
	const auto textureFormat = ci::gl::Texture::Format().internalFormat(GL_RED);
	mTexture				 = ci::gl::Texture::create(dimensions.x, dimensions.y, textureFormat);

	const auto fboFormat =
		ci::gl::Fbo::Format().attachment(GL_COLOR_ATTACHMENT0, mTexture).samples(0).stencilBuffer().disableDepth();
	const auto fbo = ci::gl::Fbo::create(dimensions.x, dimensions.y, fboFormat);

	{
		// Render path to shadow texture.
		ci::gl::ScopedFramebuffer sf(fbo);
		ci::gl::ScopedViewport	  sv(dimensions);
		ci::gl::ScopedMatrices	  sm;
		ci::gl::setMatricesWindow(dimensions);

		// Make sure the matrices are updated for path rendering.
		ci::gl::clearColor(ColorA(0, 0, 0, 0));
		ci::gl::clear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Assumes mask and clear value have been set.

		ci::gl::translate(mPadding);
		ci::gl::scale(vec2(1.0f / static_cast<float>(mScale)));
		ci::gl::translate(-ivec2(path.getStrokeBounds().getUpperLeft()));

		// Also: don't clear the stencil buffer afterwards.
		nvpath::ScopedPathRendering sp;
		path.stencilFill();
		path.stencilStroke();
		path.coverStroke(ci::Color::white(), false);
	}

	mBlur.applyEffect(mTexture);

	// Make sure the matrices are restored for path rendering.
	ci::gl::matrixLoadfEXT(GL_MODELVIEW, value_ptr(ci::gl::getModelView()));
	ci::gl::matrixLoadfEXT(GL_PROJECTION, value_ptr(ci::gl::getProjectionMatrix()));

	// Load the shadow shader.
	if (!sShadowShader) {
		sShadowShader = ci::gl::GlslProg::create(sVertShader, sFragShader);
	}
}

void PathSpriteShadow::draw(const vec2& offset, float opacity) const {
	if (mTexture && sShadowShader) {
		ci::gl::ScopedBlendPremult sb;
		ci::gl::ScopedColor		   sc(mColor);
		ci::gl::ScopedTextureBind  st(mTexture, 0);
		ci::gl::ScopedGlslProg	   sg(sShadowShader);
		sShadowShader->uniform("uInput", 0);

		ci::gl::ScopedModelMatrix sm;
		ci::gl::translate(offset + mOffset);
		ci::gl::scale(ivec2(mScale));
		ci::gl::translate(-vec2(mPadding));
		ci::gl::drawSolidRect(mTexture->getBounds());
	}
}

} // namespace ds::ui