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

	Gradient(	ds::ui::SpriteEngine&, 
				const ci::ColorA& tlColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
				const ci::ColorA& trColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
				const ci::ColorA& brColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f), 
				const ci::ColorA& blColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));

	void						setColorsH(const ci::ColorA& left, const ci::ColorA& right);
	void						setColorsV(const ci::ColorA& top, const ci::ColorA& bottom);
	void						setColorsAll(const ci::ColorA& tl, const ci::ColorA& tr, const ci::ColorA& br, const ci::ColorA& bl);

protected:
	virtual void				drawLocalClient();
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:
	void						setGradientColor(const DirtyState&, const ci::ColorA& src, ci::ColorA& dst);
	void						writeGradientColor(const DirtyState&, const ci::ColorA& src, const char att, ds::DataBuffer&) const;

	typedef ds::ui::Sprite		inherited;
	ci::ColorA					mTLColor;
	ci::ColorA					mTRColor;
	ci::ColorA					mBLColor;
	ci::ColorA					mBRColor;

	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

typedef Gradient GradientSprite;

} // namespace ui
} // namespace ds

#endif // DS_UI_GRADIENT_SPRITE_H_