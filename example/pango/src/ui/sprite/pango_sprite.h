#pragma once
#ifdef FUCKING_DISABLED
#ifndef DS_UI_SPRITE_PANGO_SPRITE
#define DS_UI_SPRITE_PANGO_SPRITE

#include "ds/ui/sprite/sprite.h"
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include <cinder/gl/Texture.h>

#include "cairo/cairo.h"
#include "fontconfig/fontconfig.h"
#include "pango-1.0/pango/pangocairo.h"

#include <vector>

namespace ds {
namespace ui {

enum class TextAlignment : int {
	LEFT,
	CENTER,
	RIGHT,
	JUSTIFY,
};

enum class TextRenderer {
	FREETYPE,
	PLATFORM_NATIVE,
};

enum class TextWeight : int {
	THIN = 100,
	ULTRALIGHT = 200,
	LIGHT = 300,
	SEMILIGHT = 350,
	BOOK = 380,
	NORMAL = 400,
	MEDIUM = 500,
	SEMIBOLD = 600,
	BOLD = 700,
	ULTRABOLD = 800,
	HEAVY = 900,
	ULTRAHEAVY = 1000
};

enum class TextAntialias : int {
	DEFAULT,
	NONE,
	GRAY,
	SUBPIXEL,
};

class TextPango : public ds::ui::Sprite {
public:
	TextPango(ds::ui::SpriteEngine& engine);
	~TextPango();

	// Globals
	static ds::ui::TextRenderer getTextRenderer();
	static void setTextRenderer(ds::ui::TextRenderer renderer);

	// Rendering

	const std::string getText();

	// setText can take inline markup to override the default text settings
	// See here for full list of supported tags:
	// https://developer.gnome.org/pango/stable/PangoMarkupFormat.html
	void setText(std::string text);

	// Text is rendered into this texture
	const ci::gl::TextureRef getTexture();

	// Text smaller than the min size will be clipped
	ci::Vec2i getMinSize();
	void setMinSize(int minWidth, int minHeight);
	void setMinSize(ci::Vec2i minSize);

	// Text can grow up to this size before a line breaks or clipping begins
	ci::Vec2i getMaxSize();
	void setMaxSize(int maxWidth, int maxHeight);
	void setMaxSize(ci::Vec2i maxSize);

	// Setting default font styles is more efficient than passing markup via the text string

	void setDefaultTextStyle(std::string font = "Sans", float size = 12.0, ci::ColorA color = ci::Color::black(), TextWeight weight = TextWeight::NORMAL,
							 TextAlignment alignment = TextAlignment::LEFT); // convenience

	ci::ColorA getDefaultTextColor();
	void setDefaultTextColor(ci::ColorA color);

	ci::ColorA getBackgroundColor();
	void setBackgroundColor(ci::ColorA color);

	float getDefaultTextSize();
	void setDefaultTextSize(float size);

	std::string getDefaultTextFont();
	void setDefaultTextFont(std::string font);

	TextWeight getDefaultTextWeight();
	void setDefaultTextWeight(TextWeight weight);

	TextAntialias getTextAntialias();
	void setTextAntialias(TextAntialias mode);

	TextAlignment getTextAlignment();
	void setTextAlignment(TextAlignment alignment);

	bool getDefaultTextSmallCapsEnabled();
	void setDefaultTextSmallCapsEnabled(bool value);

	bool getDefaultTextItalicsEnabled();
	void setDefaultTextItalicsEnabled(bool value);

	float getSpacing();
	void setSpacing(float spacing);

	// Renders text into the texture.
	// Returns true if the texture was actually pdated, false if nothing had to change
	// It's reasonable (and more efficient) to just run this in an update loop rather than calling it
	// explicitly after every change to the text state. It will coalesce all invalidations since the
	// last frame and only rebuild what needs to be rebuilt to render the diff.
	// Set force to true to render even if the system thinks state wasn't invalidated.
	bool render(bool force = false);


private:
	ci::gl::TextureRef mTexture;
	std::string mText;
	std::string mProcessedText; // stores text after newline filtering
	bool mProbablyHasMarkup;
	ci::Vec2i mMinSize;
	ci::Vec2i mMaxSize;

	// TODO wrap these up...
	std::string mDefaultTextFont;
	bool mDefaultTextItalicsEnabled;
	bool mDefaultTextSmallCapsEnabled;
	ci::ColorA mDefaultTextColor;
	ci::ColorA mBackgroundColor;
	float mDefaultTextSize;
	TextAlignment mTextAlignment;
	TextWeight mDefaultTextWeight;
	TextAntialias mTextAntialias;
	float mSpacing;

	// Internal flags for state invalidation
	// Used by render method
	bool mNeedsFontUpdate;
	bool mNeedsMeasuring;
	bool mNeedsTextRender;
	bool mNeedsFontOptionUpdate;
	bool mNeedsMarkupDetection;

	// simply stored to check for change across renders
	int mPixelWidth;
	int mPixelHeight;

	// Pango references
	PangoContext *pangoContext;
	PangoLayout *pangoLayout;
	PangoFontDescription *fontDescription;
	cairo_surface_t *cairoSurface;
	cairo_t *cairoContext;
	cairo_font_options_t *cairoFontOptions;

#ifdef CAIRO_HAS_WIN32_SURFACE
	cairo_surface_t *cairoImageSurface;
#endif
};
}
} // namespace kp::pango


#endif
#endif