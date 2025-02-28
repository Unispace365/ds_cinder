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
	setSize(mBounds.getSize());

	if (mShadow) mShadow->render(mPath);
}

void PathSprite::setPath(nvpath::Path&& path) {
	mPath = std::move(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();
	setSize(mBounds.getSize());

	if (mShadow) mShadow->render(mPath);
}

void PathSprite::setPath(const std::string& path) {
	mPath = nvpath::Path(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();
	setSize(mBounds.getSize());

	if (mShadow) mShadow->render(mPath);
}

void PathSprite::setShape(const std::string& shape) {
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

void PathSprite::setShadow(const float offsetX, const float offsetY, const int scale) {
	if (!mShadow)
		mShadow = std::make_unique<PathSpriteShadow>(offsetX, offsetY, scale);
	else
		mShadow->setShadow(offsetX, offsetY, scale);

	mShadow->render(mPath);
}

void PathSprite::setShadowColor(const ColorA& color) {
	if (!mShadow) mShadow = std::make_unique<PathSpriteShadow>(60.0f, 60.0f, 1);
	mShadow->setColor(color);

	mShadow->render(mPath);
}

void PathSprite::setShadowBlur(double sigma, int kernelSize) {
	if (!mShadow) mShadow = std::make_unique<PathSpriteShadow>(60.0f, 60.0f, 1);
	mShadow->setBlur(sigma, kernelSize);

	mShadow->render(mPath);
}

void PathSprite::drawLocalClient() {
	// Get render opacity.
	const auto opacity = mOpacity * getDrawOpacity();
	if (approxZero(opacity)) return;

	// Render drop shadow.
	if (mShadow) mShadow->draw(mPath.getStrokeBounds().getUpperLeft(), opacity);

	// Render path.
	nvpath::ScopedPathRendering sp;
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

void PathSprite::parseCircle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, r )");

	float		x, y, r;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		r = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");

	setPath(nvpath::circle(x, y, r));
}

void PathSprite::parseEllipse(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, rx, ry )");

	float		x, y, rx, ry;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		rx = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		ry = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");

	setPath(nvpath::ellipse(x, y, rx, ry));
}

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

void PathSprite::parseRectangle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");

	float		x, y, w, h, rx{0}, ry{0};
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		w = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		h = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s)) rx = parseFloat(&s);
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s)) ry = parseFloat(&s);

	if (approxZero(rx)) {
		if (approxZero(ry))
			setPath(nvpath::rectangle(x, y, w, h));
		else
			setPath(nvpath::roundedRectangle(x, y, w, h, ry, ry));
	} else if (approxZero(ry)) {
		setPath(nvpath::roundedRectangle(x, y, w, h, rx, rx));
	} else
		setPath(nvpath::roundedRectangle(x, y, w, h, rx, ry));
}

void PathSprite::parseStar(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");

	float		x, y, r1, r2, points, angle;
	const char* s = params.c_str();
	if (*s && isNumeric(*s))
		x = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		y = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		r1 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		r2 = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		points = parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	skipSpaceOrComma(&s);
	if (*s && isNumeric(*s))
		angle = parseFloat(&s);
	else
		angle = 0;

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
											"    fragColor *= texture( uInput, vertTexCoord ).r;"
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
	mSigma		= sigma;
	mKernelSize = kernelSize;
}

void PathSpriteShadow::render(const nvpath::Path& path) {
	if (!path.getId()) return;

	// Construct drop shadow.
	mPadding = ivec2(2 * static_cast<int>(mSigma));

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

		nvpath::ScopedPathRendering sp;
		path.fill(Color::white());
		path.stroke(Color::white());
	}

	const EffectBlur blur{mSigma, mKernelSize};
	blur.applyEffect(mTexture);

	// Make sure the matrices are restored for path rendering.
	ci::gl::matrixLoadfEXT(GL_MODELVIEW, value_ptr(ci::gl::getModelView()));
	ci::gl::matrixLoadfEXT(GL_PROJECTION, value_ptr(ci::gl::getProjectionMatrix()));

	// Load the shadow shader.
	if (!sShadowShader) {
		sShadowShader = ci::gl::GlslProg::create(sVertShader, sFragShader);
	}
}

void PathSpriteShadow::draw(const vec2& offset, float opacity) const {
	ci::gl::ScopedBlendPremult sb;

	if (mTexture && sShadowShader) {
		ci::gl::ScopedColor		  sc(ColorA(mColor, opacity));
		ci::gl::ScopedTextureBind st(mTexture, 0);
		ci::gl::ScopedGlslProg	  sg(sShadowShader);
		sShadowShader->uniform("uInput", 0);

		ci::gl::ScopedModelMatrix sm;
		ci::gl::translate(offset + mOffset);
		ci::gl::scale(ivec2(mScale));
		ci::gl::translate(-vec2(mPadding));
		ci::gl::drawSolidRect(mTexture->getBounds());
	}
}

} // namespace ds::ui