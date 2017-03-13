#pragma once
#ifndef DS_UI_SPRITE_PANGO_SPRITE
#define DS_UI_SPRITE_PANGO_SPRITE

#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/text_defs.h"
#include <cinder/gl/Texture.h>
#include "ds/ui/sprite/fbo/fbo.h"
#include "ds/ui/sprite/shader/sprite_shader.h"


#include "cairo/cairo.h"
#include "fontconfig/fontconfig.h"
#include "pango/pangocairo.h"

namespace ds {
namespace ui {


/**
* \class ds::ui::TextPango
*	A sprite for displaying text. 
*	This is similar to the Text class, except this uses Pango and Cairo for layout and rendering instead of OGLFT
*	The current implementation requires fonts to be installed system-wide. (there seems to be a bug in fontconfig/pango loading a font from a file)
*	You can specify some simple markup for pango text. See this page: http://www.gtk.org/api/2.6/pango/PangoMarkupFormat.html
*		Basics: <markup> root document node (optional)
				<span foreground='blue' style='italic'>Some blue italic text</span> (NOTE: you must use single 'quotes' and not double "quotes" like the documentation. Double quotes = nothing shows up)
					Span attributes: 
						font_desc (such as 'Sans Italic 12')
						font_family or face (such as 'Arial' or 'Sans')
						size ('xx-small', 'x-small', 'small', 'medium', 'large', 'x-large', 'xx-large' or 'smaller' or 'larger')
						style (One of 'normal', 'oblique', 'italic')
						weight (One of 'ultralight', 'light', 'normal', 'bold', 'ultrabold', 'heavy', or a numeric weight like 400 for normal or 700 for bold)
						variant ('normal' or 'smallcaps')
						stretch (One of 'ultracondensed', 'extracondensed', 'condensed', 'semicondensed', 'normal', 'semiexpanded', 'expanded', 'extraexpanded', 'ultraexpanded')
						foreground (An RGB color specification such as '#00FF00' or a color name such as 'red')
						background (An RGB color specification such as '#00FF00' or a color name such as 'red')
						underline (One of 'single', 'double', 'low', 'none')
						underline_color (The color of underlines; an RGB color specification such as '#00FF00' or a color name such as 'red')
						rise (Vertical displacement, in 10000ths of an em. Can be negative for subscript, positive for superscript.)
						strikethrough ('true' or 'false' whether to strike through the text)
						strikethrough_color (The color of strikethrough lines; an RGB color specification such as '#00FF00' or a color name such as 'red')
						fallback ('true' or 'false' whether to enable fallback. If disabled, then characters will only be used from the closest matching font on the system. No fallback will be done to other fonts on the system that might contain the characters in the text. Fallback is enabled by default. Most applications should not disable fallback.)
						lang (A language code, indicating the text language)
						letter_spacing (in 1024ths of a point)
				<b> bold
				<big> bigger
				<i> italic
				<s> strikethrough
				<sub> Subscript
				<sup> Superscript
				<small> smaller
				<tt> monospace font
				<u> underline
*/

class TextPango : public ds::ui::Sprite {
public:
	TextPango(ds::ui::SpriteEngine& engine);
	~TextPango();

	std::string					getTextAsString() const;
	std::wstring				getText() const;
	bool						hasText() const;

	/// Can be plain text or use markup (see TextPango header)
	void						setText(std::string text);
	void						setText(std::wstring text);


	/// Resize limit is the amount of width the text will wrap at and the height that text will no longer be rendered
	float						getResizeLimitWidth() const;
	float						getResizeLimitHeight() const;
	TextPango&					setResizeLimit(const float width = 0, const float height = 0);

	/// This is a convenience that can set all the text attributes at once
	void						setTextStyle(std::string font = "Sans", float size = 12.0f, ci::ColorA color = ci::Color::black(), TextWeight weight = TextWeight::kNormal, Alignment::Enum alignment = Alignment::kLeft); 

	/// Sets the default color of the text for rendering.
	/// You can also set the color of the sprite, which works like a "multiply" effect on the outputted text color
	void						setTextColor(const ci::Color&);
	ci::Color					getTextColor(){	return mTextColor; };
	/** Set the display color of this Sprite. Implementation can vary by Sprite type. */
	virtual void				setColor(const ci::Color&) override;
	/** Set the display color of this Sprite. Implementation can vary by Sprite type.
	\param r Red component of the color from 0.0 to 1.0
	\param g Green component of the color from 0.0 to 1.0
	\param b Blue component of the color from 0.0 to 1.0 */
	virtual void				setColor(float r, float g, float b) override;

	virtual void				setColorA(const ci::ColorA&) override;

	/// The font name must match a family or face name installed on the system. Will warn and use a default if the desired font is not installed
	TextPango&					setFont(const std::string& name);
	TextPango&					setFont(const std::string& name, const float fontSize); // provided for compatibility with the old api
	std::string					getFont(){ return mTextFont; };

	float						getFontSize(){ return mTextSize; }
	void						setFontSize(float fontSize);

	/// If using a font family, will set the desired weight for the whole sprite. You can use markup to specifically select portions of text to be different weights
	TextWeight					getDefaultTextWeight();
	void						setDefaultTextWeight(TextWeight weight);

	Alignment::Enum				getAlignment();
	void						setAlignment(Alignment::Enum alignment);

	bool						getDefaultTextSmallCapsEnabled();
	void						setDefaultTextSmallCapsEnabled(bool value);

	bool						getDefaultTextItalicsEnabled();
	void						setDefaultTextItalicsEnabled(bool value);

	float						getLeading() const;
	TextPango&					setLeading(const float);

	virtual float				getWidth() const;
	virtual float				getHeight() const;

	/// Whether to add ellipses to the text if it doesn't fit inside the resize limit
	/// If the resize limit is set to 0 or -1, no text wrapping will happen and no ellipses will be added
	void						setEllipsizeMode(EllipsizeMode theMode);
	EllipsizeMode				getEllipsizeMode();

	/// If ellipsize mode is none and there's a resize width > 0 and the text had to wrap at all, returns true. otherwise false
	bool						getTextWrapped();

	/// The number of lines in the text layout
	int							getNumberOfLines();


	/// Returns the 2-d position of the character in the current text string
	/// Will return 0,0 if the string is blank or the index is out-of-bounds
	/// Note that this position is fudged by 25% vertically to get the top-left corner of most characters in most fonts. 
	/// This is good for cursors. If you need the exact bounds of a character, use getRectForCharacterIndex()
	ci::vec2					getPositionForCharacterIndex(const int characterIndex);

	/// Returns the bounds of the character at the supplied index
	/// Returns 0,0,0,0 if the string is blank of the index is out of bounds
	/// Note that this box is likely taller than the actual character
	/// This is just a wrapper around pango_layout_index_to_pos(), so see that documentation for full details
	ci::Rectf					getRectForCharacterIndex(const int characterIndex);


	/// Returns the index of the character of the current text string for the position supplied
	/// Gracefully handles out-of-bounds points (will always return a valid index, assuming the current text string isn't empty)
	int							getCharacterIndexForPosition(const ci::vec2& localPosition);

	// TODO: make these function?
	void						setConfigName(const std::string& cfgName){ mCfgName = cfgName; }
	const std::string			getConfigName(){ return mCfgName; }
	const std::string			getFontFileName(){ return mTextFont; }

	virtual void				updateClient(const UpdateParams &updateParams);
	virtual void				updateServer(const UpdateParams&);
	void						drawLocalClient();

	// Renders text into the texture.
	// Returns true if the texture was actually updated, false if nothing had to change
	// It's reasonable (and more efficient) to just run this in an update loop rather than calling it
	// explicitly after every change to the text state. It will coalesce all invalidations since the
	// last frame and only rebuild what needs to be rebuilt to render the diff.
	// Set force to true to render even if the system thinks state wasn't invalidated.
	bool render(bool force = false);

	/// Text is rendered into this texture
	/// Note: this texture has pre-multiplied alpha
	const ci::gl::TextureRef	getTexture();


	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);

	virtual	void				buildRenderBatch() override;
private:
	ci::gl::TextureRef			mTexture;

	std::string					mCfgName;

	std::wstring				mText;
	std::wstring				mProcessedText; // stores text after newline filtering
	bool						mProbablyHasMarkup;
	float						mResizeLimitWidth,
								mResizeLimitHeight;

	float						mTextSize;
	std::string					mTextFont;
	ci::Color					mTextColor;
	bool						mDefaultTextItalicsEnabled;
	bool						mDefaultTextSmallCapsEnabled;
	Alignment::Enum				mTextAlignment;
	TextWeight					mDefaultTextWeight;
	EllipsizeMode				mEllipsizeMode;
	float						mLeading;

	// Info about the text layout
	bool						mWrappedText;
	int							mNumberOfLines;

	// Internal flags for state invalidation
	// Used by render method
	bool						mNeedsFontUpdate;
	bool 						mNeedsMeasuring;
	bool 						mNeedsTextRender;
	bool 						mNeedsFontOptionUpdate;
	bool 						mNeedsMarkupDetection;

	// simply stored to check for change across renders
	int 						mPixelWidth;
	int							mPixelHeight;

	// Pango references
	PangoContext*				mPangoContext;
	PangoLayout*				mPangoLayout;
	PangoFontDescription*		mFontDescription;
	cairo_surface_t*			mCairoSurface;
	cairo_t*					mCairoContext;
	cairo_font_options_t*		mCairoFontOptions;

#ifdef CAIRO_HAS_WIN32_SURFACE
	cairo_surface_t*			mCairoWinImageSurface;
#endif
};
}
} // namespace kp::pango


#endif