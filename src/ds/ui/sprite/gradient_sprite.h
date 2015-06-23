#pragma once
#ifndef DS_UI_GRADIENT_SPRITE_H_
#define DS_UI_GRADIENT_SPRITE_H_

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/**
 \class Gradient
 \brief Draw a rectangle with up to 4 colors in a gradient.
 \author Gordon Nickel
 */
class Gradient : public ds::ui::Sprite {
public:

	/** Make a Gradient sprite with horizontal colors set for the gradient
	\param engine The Sprite engine for the app
	\param left The colors for the left side of the sprite (TL and BL colors will be the same)
	\param right The colors for the right side of the sprite (TR and BR colors will be the same)
	\param parent A sprite for the new gradient to be added to after construction for convenience.
	*/
	static Gradient&			makeH(SpriteEngine& engine, const ci::ColorA& left, const ci::ColorA& right, Sprite* parent = nullptr);

	/** Make a Gradient sprite with vertical colors set for the gradient
	\param engine The Sprite engine for the app
	\param top The colors for the top side of the sprite (TL and TR colors will be the same)
	\param bottom The colors for the bottom side of the sprite (BL and BR colors will be the same)
	\param parent A sprite for the new gradient to be added to after construction for convenience.
	*/
	static Gradient&			makeV(SpriteEngine& engine, const ci::ColorA& top, const ci::ColorA& bottom, Sprite* parent = nullptr);

	/** Make a Gradient sprite with vertical colors set for the gradient
	\param engine The Sprite engine for the app
	\param tlColor The top left color for the 4-color gradient
	\param trColor The top right color for the 4-color gradient
	\param brColor The bottom right color for the 4-color gradient
	\param blColor The bottom left color for the 4-color gradient
	*/
	Gradient(ds::ui::SpriteEngine& engine,
				const ci::ColorA& tlColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
				const ci::ColorA& trColor = ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f), 
				const ci::ColorA& brColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f), 
				const ci::ColorA& blColor = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));

	/** Set the horizontal gradient colors. TL and BL will be the same, and TR and BR will be the same color.
	\param left The colors for the left side of the sprite (TL and BL colors will be the same)
	\param right The colors for the right side of the sprite (TR and BR colors will be the same)
	*/
	void						setColorsH(const ci::ColorA& left, const ci::ColorA& right);

	/** Set the vertical gradient colors. TL and BL will be the same, and TR and BR will be the same color.
	\param top The colors for the top side of the sprite (TL and TR colors will be the same)
	\param bottom The colors for the bottom side of the sprite (BL and BR colors will be the same)
	*/
	void						setColorsV(const ci::ColorA& top, const ci::ColorA& bottom);

	/** Set all 4 colors of the graidnet
	\param tlColor The top left color for the 4-color gradient
	\param trColor The top right color for the 4-color gradient
	\param brColor The bottom right color for the 4-color gradient
	\param blColor The bottom left color for the 4-color gradient
	*/
	void						setColorsAll(const ci::ColorA& tlColor, const ci::ColorA& trColor, const ci::ColorA& brColor, const ci::ColorA& blColor);

	/** \return The Top Left color of the Gradient */
	ci::ColorA&					getColorTL();
	/** \return The Top Right color of the Gradient */
	ci::ColorA&					getColorTR();
	/** \return The Bottom Left color of the Gradient */
	ci::ColorA&					getColorBL();
	/** \return The Bottom Right color of the Gradient */
	ci::ColorA&					getColorBR();

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