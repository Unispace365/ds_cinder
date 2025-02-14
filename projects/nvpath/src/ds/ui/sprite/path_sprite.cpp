#include "stdafx.h"

#include "ds/ui/sprite/path_sprite.h"

namespace {

// Add the 'svg' sprite type so we can use it in our layout XML.
auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
		// Register our custom sprite(s).
		e.registerSpriteImporter(
			"path", [](ds::ui::SpriteEngine& engine) -> ds::ui::Sprite* { return new ds::ui::PathSprite(engine); });

		// Register the properties for our custom sprites.
		e.registerSpritePropertySetter<ds::ui::PathSprite>("d",
														   [](ds::ui::PathSprite& sprite, const std::string& theValue,
															  const std::string&) { sprite.setPath(theValue); });
		e.registerSpritePropertySetter<ds::ui::PathSprite>("fill",
														   [](ds::ui::PathSprite& sprite, const std::string& theValue,
															  const std::string&) { sprite.setFill(theValue); });
		e.registerSpritePropertySetter<ds::ui::PathSprite>("stroke",
														   [](ds::ui::PathSprite& sprite, const std::string& theValue,
															  const std::string&) { sprite.setStroke(theValue); });
	});
	return true;
}();

} // namespace

namespace ds::ui {

PathSprite::PathSprite(ds::ui::SpriteEngine& engine)
	: Sprite(engine) {
}

void PathSprite::setPath(const std::string& pathDef) {
	mPath	= nvpath::Path(pathDef, nvpath::PathFormat::SVG);
	mBounds = mPath.getFillBounds();
	mBounds.include(mPath.getStrokeBounds());
	setSize(mBounds.getSize());
	setTransparent(!(mFillColor.a > 0 || mStrokeColor.a > 0));
}

void PathSprite::setFill(const std::string& fillColor) {
	mFillColor = mEngine.getColors().getColorFromName(fillColor);
	setTransparent(!(mFillColor.a > 0 || mStrokeColor.a > 0));
}

void PathSprite::setStroke(const std::string& strokeColor) {
	mStrokeColor = mEngine.getColors().getColorFromName(strokeColor);
	setTransparent(!(mFillColor.a > 0 || mStrokeColor.a > 0));
}

void PathSprite::drawLocalClient() {
	ci::gl::ScopedModelMatrix sm;
	ci::gl::translate(-mBounds.getUpperLeft());

	nvpath::ScopedPathRendering sp;

	if (mFillColor.a > 0) mPath.fill(mFillColor);
	if (mStrokeColor.a > 0) mPath.stroke(mStrokeColor);
}

} // namespace ds::ui
