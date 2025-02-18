#include "stdafx.h"

#include "path_sprite.h"

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
		e.registerSpriteImporter(
			"path", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* { return new PathSprite(enginey); });

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
	});
	return true;
}();

} // namespace

using namespace ci;

namespace ds::ui {

PathSprite::PathSprite(ds::ui::SpriteEngine& engine)
  : Sprite(engine) {}

PathSprite::~PathSprite() {
	mEngine.getLoadImageService().release(mFilename, this);
}

bool PathSprite::contains(const vec3& point, float) const {
	const auto local = globalToLocal(point);
	return mBounds.contains(local);
}

bool PathSprite::isPointInsideFill(const ci::vec3& point) const {
	const vec2 local = globalToLocal(point);
	return mPath.fillContains(local);
}

bool PathSprite::isPointInsideStroke(const ci::vec3& point) const {
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
}

void PathSprite::setPath(nvpath::Path&& path) {
	mPath = std::move(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();
	setSize(mBounds.getSize());
}

void PathSprite::setPath(const std::string& path) {
	mPath = nvpath::Path(path);
	mPath.setStrokeWidth(mStrokeWidth);
	mPath.setDashCaps(mDashCapsInitial, mDashCapsTerminal);
	mPath.setEndCaps(mEndCapsInitial, mEndCapsTerminal);
	mPath.setJoinStyle(mJoinStyle);

	mBounds = calcBounds();
	setSize(mBounds.getSize());
}

void PathSprite::setShape(const std::string& shape) {
	try {
		const char* sInOut = shape.c_str();
		ds::skipSpace(&sInOut);
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
		mFill = nvpath::Paint::color(ds::parseColor(fill, mEngine));

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::setFill(const nvpath::Paint& fill) {
	mFill = fill;

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::setFillColor(const ColorA8u& color) {
	mFill = nvpath::Paint::color(color);

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::setStroke(const std::string& color) {
	if (!asciiCaseCmp(color.c_str(), "none"))
		mStroke = {};
	else
		mStroke = nvpath::Paint::color(ds::parseColor(color, mEngine));

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::setStroke(const nvpath::Paint& fill) {
	mStroke = fill;

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::setStrokeColor(const ColorA8u& color) {
	mStroke = nvpath::Paint::color(color);

	setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
}

void PathSprite::drawLocalClient() {
	// Get render opacity.
	const auto opacity = mOpacity * getDrawOpacity();
	if (ds::approxZero(opacity)) return;

	// Render path.
	nvpath::ScopedPathRendering sp;
	if (mTexture)
		mPath.fill(mTexture, mPath.getFillBounds(), opacity);
	else
		mPath.fill(mFill, opacity);
	mPath.stroke(mStroke, opacity);
}

void PathSprite::fitInsideArea(const ci::Rectf& area) {
	if (!mPath.getId()) return;

	const auto fit = mFit.calcTransform(area, mBounds, false);
	setScale(fit[0][0], fit[1][1]);
	setPosition(fit[2]);
}

Rectf PathSprite::calcBounds() const {
	auto bounds = mStroke.isNone() ? mPath.getFillBounds() : mPath.getStrokeBounds();
	auto size	= glm::vec2{glm::max(mTouchSize.x, bounds.getWidth()), glm::max(mTouchSize.y, bounds.getHeight())};
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

			setTransparent(mStroke.isNone() && mFill.isNone() && !mTexture);
		});
}

void PathSprite::parseCircle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, r )");

	float		x, y, r;
	const char* s = params.c_str();
	if (*s && ds::isNumeric(*s))
		x = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		r = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, r )");

	setPath(nvpath::circle(x, y, r));
}

void PathSprite::parseEllipse(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, rx, ry )");

	float		x, y, rx, ry;
	const char* s = params.c_str();
	if (*s && ds::isNumeric(*s))
		x = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		rx = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		ry = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rx, ry )");

	setPath(nvpath::ellipse(x, y, rx, ry));
}

void PathSprite::parseLine(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x1, y1, x2, y2 )");

	float		x1, y1, x2, y2;
	const char* s = params.c_str();
	if (*s && ds::isNumeric(*s))
		x1 = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y1 = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		x2 = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x1, y1, x2, y2 )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y2 = ds::parseFloat(&s);
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
		if (ds::isNumeric(*s))
			points.push_back(ds::parseFloat(&s));
		else
			throw std::runtime_error("Expected ( x1, y1, x2, y2, ... )");
		ds::skipSpaceOrComma(&s);
	}

	if (points.size() % 2 != 0) throw std::runtime_error("Expected ( x1, y1, x2, y2, ... )");

	setPath(nvpath::polygon(reinterpret_cast<const vec2*>(points.data()), points.size() / 2, true));
}

void PathSprite::parseRectangle(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");

	float		x, y, w, h, rx{0}, ry{0};
	const char* s = params.c_str();
	if (*s && ds::isNumeric(*s))
		x = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		w = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		h = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, w, h, (rx), (ry) )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s)) rx = ds::parseFloat(&s);
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s)) ry = ds::parseFloat(&s);

	if (ds::approxZero(rx)) {
		if (ds::approxZero(ry))
			setPath(nvpath::rectangle(x, y, w, h));
		else
			setPath(nvpath::roundedRectangle(x, y, w, h, ry, ry));
	} else if (ds::approxZero(ry)) {
		setPath(nvpath::roundedRectangle(x, y, w, h, rx, rx));
	} else
		setPath(nvpath::roundedRectangle(x, y, w, h, rx, ry));
}

void PathSprite::parseStar(const char** sInOut) {
	const auto params = fetchParameters(sInOut);
	if (params.empty()) throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");

	float		x, y, r1, r2, points, angle;
	const char* s = params.c_str();
	if (*s && ds::isNumeric(*s))
		x = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		y = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		r1 = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		r2 = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		points = ds::parseFloat(&s);
	else
		throw std::runtime_error("Expected ( x, y, rmax, rmin, points, angle )");
	ds::skipSpaceOrComma(&s);
	if (*s && ds::isNumeric(*s))
		angle = ds::parseFloat(&s);
	else
		angle = 0;

	setPath(nvpath::star(x, y, r1, r2, points, angle));
}

void PathSprite::parseDashArray(const char** sInOut) {
	std::vector<float> pattern;
	pattern.reserve(16);
	while (**sInOut) {
		ds::skipSpaceOrComma(sInOut);
		while (**sInOut && !ds::isNumeric(**sInOut))
			++(*sInOut);
		if (**sInOut && ds::isNumeric(**sInOut)) pattern.push_back(ds::parseFloat(sInOut));
	}
	setDashPattern(pattern);
}

std::string PathSprite::fetchParameters(const char** sInOut) {
	std::string params;

	ds::skipSpace(sInOut);
	if (**sInOut == '(') {
		++*sInOut;
		ds::skipSpace(sInOut);
		params = ds::fetchUntil(sInOut, ')');
		if (**sInOut) ++*sInOut;
	}

	return trim(params);
}

} // namespace ds::ui