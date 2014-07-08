#pragma once
#ifndef DS_UI_GRADIENT_SPRITE_H_
#define DS_UI_GRADIENT_SPRITE_H_

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/**
 * \class ds::ui::sprite::GradientSprite
 * Draw a rectangle with 2-color top-to-bottom gradient
 */
class Gradient : public ds::ui::Sprite {
public:
	static Gradient&			makeH(SpriteEngine&, const ci::ColorA& left, const ci::ColorA& right, Sprite* parent = nullptr);
	static Gradient&			makeV(SpriteEngine&, const ci::ColorA& top, const ci::ColorA& bottom, Sprite* parent = nullptr);

	Gradient(ds::ui::SpriteEngine&, 
		ci::ColorA tlColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
		ci::ColorA trColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
		ci::ColorA brColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f), 
		ci::ColorA blColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));

	void						setColorsH(const ci::ColorA& left, const ci::ColorA& right);
	void						setColorsV(const ci::ColorA& top, const ci::ColorA& bottom);
	void						setColorsAll(const ci::ColorA& tl, const ci::ColorA& tr, const ci::ColorA& br, const ci::ColorA& bl);

protected:
	virtual void				drawLocalClient();

private:
	ci::ColorA					mTLColor;
	ci::ColorA					mTRColor;
	ci::ColorA					mBLColor;
	ci::ColorA					mBRColor;

	typedef ds::ui::Sprite		inherited;
};

typedef Gradient GradientSprite;

} // namespace ui
} // namespace ds

#endif // DS_UI_GRADIENT_SPRITE_H_