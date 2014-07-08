#include "gradient_sprite.h"

#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {

/**
 * \class ds::ui::sprite::GradientSprite
 */
Gradient& Gradient::makeH(SpriteEngine& e, const ci::ColorA& x1, const ci::ColorA& x2, Sprite* parent) {
	return makeAlloc<ds::ui::Gradient>([&e, &x1, &x2]()->ds::ui::Gradient*{
		Gradient* s = new ds::ui::Gradient(e);
		if (s) s->setColorsH(x1, x2);
		return s;
	}, parent);
}

Gradient& Gradient::makeV(SpriteEngine& e, const ci::ColorA& y1, const ci::ColorA& y2, Sprite* parent) {
	return makeAlloc<ds::ui::Gradient>([&e, &y1, &y2]()->ds::ui::Gradient*{
		Gradient* s = new ds::ui::Gradient(e);
		if (s) s->setColorsV(y1, y2);
		return s;
	}, parent);
}

Gradient::Gradient(ds::ui::SpriteEngine& e, ci::ColorA tlColor, ci::ColorA trColor, ci::ColorA brColor, ci::ColorA blColor)
		: inherited(e)
		, mTLColor(tlColor)
		, mTRColor(trColor)
		, mBRColor(brColor)
		, mBLColor(blColor)
{
	setTransparent(false);
}

void Gradient::drawLocalClient() {
	// the magic!
	ci::gl::begin(GL_QUADS);
	ci::gl::color(mTLColor.r, mTLColor.g, mTLColor.b, mTLColor.a * mDrawOpacityHack);
	ci::gl::vertex(0, 0);
	ci::gl::color(mTRColor.r, mTRColor.g, mTRColor.b, mTRColor.a * mDrawOpacityHack);
	ci::gl::vertex(getWidth(), 0.0f);
	ci::gl::color(mBRColor.r, mBRColor.g, mBRColor.b, mBRColor.a * mDrawOpacityHack);
	ci::gl::vertex(getWidth(), getHeight());
	ci::gl::color(mBLColor.r, mBLColor.g, mBLColor.b, mBLColor.a * mDrawOpacityHack);
	ci::gl::vertex(0.0f, getHeight());
	ci::gl::end();
}

void Gradient::setColorsH(const ci::ColorA& leftColor, const ci::ColorA& rightColor ){
	mTLColor = leftColor;
	mBLColor = leftColor;
	mTRColor = rightColor;
	mBRColor = rightColor;
}

void Gradient::setColorsV(const ci::ColorA& topColor, const ci::ColorA& botColor ){
	mTLColor = topColor;
	mTRColor = topColor;
	mBLColor = botColor;
	mBRColor = botColor;
}

void Gradient::setColorsAll(const ci::ColorA& tlColor, const ci::ColorA& trColor,
							const ci::ColorA& brColor, const ci::ColorA& blColor) {
	mTLColor = tlColor;
	mTRColor = trColor;
	mBLColor = blColor;
	mBRColor = brColor;
}

} // using namespace ui
} // using namespace ds
