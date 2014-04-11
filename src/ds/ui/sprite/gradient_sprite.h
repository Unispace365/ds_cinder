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
class GradientSprite : public ds::ui::Sprite {
public:

	GradientSprite(ds::ui::SpriteEngine&, 
		ci::ColorA tlColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
		ci::ColorA trColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
		ci::ColorA brColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f), 
		ci::ColorA blColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));

	void						setColorsH(ci::ColorA leftColor, ci::ColorA rightColor);
	void						setColorsV(ci::ColorA topColor, ci::ColorA botColor);
	void						setColorsAll(ci::ColorA tlColor, ci::ColorA trColor, ci::ColorA brColor, ci::ColorA blColor);

	virtual void				updateClient(const UpdateParams&);
	virtual void				updateServer(const UpdateParams&);


protected:
	virtual void				drawLocalClient();

private:
	ci::ColorA					mTLColor;
	ci::ColorA					mTRColor;
	ci::ColorA					mBLColor;
	ci::ColorA					mBRColor;

	typedef ds::ui::Sprite		inherited;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_GRADIENT_SPRITE_H_