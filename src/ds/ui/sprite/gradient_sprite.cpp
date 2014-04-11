#include "gradient_sprite.h"

#include <ds/app/app.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {


/**
 * \class ds::ui::sprite::GradientSprite
 */
GradientSprite::GradientSprite(ds::ui::SpriteEngine& e, ci::ColorA tlColor, ci::ColorA trColor, ci::ColorA brColor, ci::ColorA blColor)
		: inherited(e)
		, mTLColor(tlColor)
		, mTRColor(trColor)
		, mBRColor(brColor)
		, mBLColor(blColor)
{
	setTransparent(false);
}


void GradientSprite::updateClient(const UpdateParams& p) {
	inherited::updateClient(p);
}

void GradientSprite::updateServer(const UpdateParams& p) {
	inherited::updateServer(p);
	
}

void GradientSprite::drawLocalClient() {
	// the magic!
	ci::gl::begin(GL_QUADS);
	ci::gl::color(mTLColor.r, mTLColor.g, mTLColor.b, mTLColor.a * mOpacity);
	ci::gl::vertex(0, 0);
	ci::gl::color(mTRColor.r, mTRColor.g, mTRColor.b, mTRColor.a * mOpacity);
	ci::gl::vertex(getWidth(), 0.0f);
	ci::gl::color(mBRColor.r, mBRColor.g, mBRColor.b, mBRColor.a * mOpacity);
	ci::gl::vertex(getWidth(), getHeight());
	ci::gl::color(mBLColor.r, mBLColor.g, mBLColor.b, mBLColor.a * mOpacity);
	ci::gl::vertex(0.0f, getHeight());
	ci::gl::end();
}

void GradientSprite::setColorsH( ci::ColorA leftColor, ci::ColorA rightColor ){
	mTLColor = leftColor;
	mBLColor = leftColor;
	mTRColor = rightColor;
	mBRColor = rightColor;
}

void GradientSprite::setColorsV( ci::ColorA topColor, ci::ColorA botColor ){
	mTLColor = topColor;
	mTRColor = topColor;
	mBLColor = botColor;
	mBRColor = botColor;

}

void GradientSprite::setColorsAll( ci::ColorA tlColor, ci::ColorA trColor, ci::ColorA brColor, ci::ColorA blColor ){
	mTLColor = tlColor;
	mTRColor = trColor;
	mBLColor = blColor;
	mBRColor = brColor;
}




} // using namespace ui
} // using namespace ds
