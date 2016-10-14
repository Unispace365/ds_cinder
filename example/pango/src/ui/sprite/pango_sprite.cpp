// CinderPango.cpp
// PangoBasic
//
// Created by Eric Mika on 1/6/16.
//

#include "pango_sprite.h"
#include <pango/pango-font.h>
#include <regex>

#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/service/pango_font_service.h"
#include <ds/debug/logger.h>

#if CAIRO_HAS_WIN32_SURFACE
#include <cairo-win32.h>
#endif

namespace ds {
namespace ui {

TextPango::TextPango(ds::ui::SpriteEngine& eng)
	: ds::ui::Sprite(eng)
	, mText("")
	, mProcessedText("")
	, mNeedsMarkupDetection(false)
	, mNeedsFontUpdate(false)
	, mNeedsMeasuring(false)
	, mNeedsTextRender(false)
	, mNeedsFontOptionUpdate(false)
	, mProbablyHasMarkup(false)
	, mDefaultTextColor(ci::ColorA::black())
	, mBackgroundColor(ci::ColorA::black())
	, mDefaultTextFont("Sans")
	, mDefaultTextSize(120.0)
	, mDefaultTextItalicsEnabled(false)
	, mDefaultTextSmallCapsEnabled(false)
	, mMinSize(ci::Vec2f(0, 0))
	, mMaxSize(ci::Vec2f(320, 240))
	, mSpacing(0)
	, mTextAlignment(TextAlignment::LEFT)
	, mDefaultTextWeight(TextWeight::NORMAL)
	, mTextAntialias(TextAntialias::SUBPIXEL)
	, mPixelWidth(-1)
	, mPixelHeight(-1)
	, fontDescription(nullptr)
	, pangoContext(nullptr)
	, pangoLayout(nullptr)
	, cairoSurface(nullptr)
	, cairoContext(nullptr)
	, cairoFontOptions(nullptr)

#ifdef CAIRO_HAS_WIN32_SURFACE
	, cairoImageSurface(nullptr)
#endif
{
//	std::cout << "Pango version: " << PANGO_VERSION_MAJOR << "." << PANGO_VERSION_MINOR << "." << PANGO_VERSION_MICRO << " " << PANGO_VERSION_STRING << std::endl;

	if(!mEngine.getPangoFontService().getPangoFontMap()) {
		DS_LOG_WARNING("Cannot create the pango font map, nothing will render for this pango text sprite.");
		return;
	}

	// Create Pango Context for reuse
	pangoContext = pango_font_map_create_context(mEngine.getPangoFontService().getPangoFontMap());
	if(nullptr == pangoContext) {
		DS_LOG_WARNING("Cannot create the pango font context.");
		return;
	}

	// Create Pango Layout for reuse
	pangoLayout = pango_layout_new(pangoContext);
	if(pangoLayout == nullptr) {
		DS_LOG_WARNING("Cannot create the pango layout.");
		return;
	}

	// Initialize Cairo surface and context, will be instantiated on demand
	cairoFontOptions = cairo_font_options_create();
	if(cairoFontOptions == nullptr) {
		DS_LOG_WARNING("Cannot create Cairo font options.");
		return;
	}

	// Generate the default font config
	mNeedsFontOptionUpdate = true;
	mNeedsFontUpdate = true;
	render();
}

TextPango::~TextPango() {
	// This causes crash on windows
	if(cairoContext != nullptr) {
		cairo_destroy(cairoContext);
	}

	if(fontDescription != nullptr) {
		pango_font_description_free(fontDescription);
	}

	if(cairoFontOptions != nullptr) {
		cairo_font_options_destroy(cairoFontOptions);
	}

#ifdef CAIRO_HAS_WIN32_SURFACE
	if(cairoImageSurface != nullptr) {
		cairo_surface_destroy(cairoImageSurface);
	}
#else
	// Crashes windows...
	if(cairoSurface != nullptr) {
		cairo_surface_destroy(cairoSurface);
	}
#endif

	// g_object_unref(pangoContext); // this one crashes Windows?
	//g_object_unref(fontMap);
	g_object_unref(pangoLayout);
}

//#pragma mark - Getters / Setters

const std::string TextPango::getText() {
	return mText;
}

void TextPango::setText(std::string text) {
	if(text != mText) {
		mText = text;
		mNeedsMarkupDetection = true;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;
	}
}

const ci::gl::TextureRef TextPango::getTexture() {
	return mTexture;
}

void TextPango::setDefaultTextStyle(std::string font, float size, ci::ColorA color, TextWeight weight, TextAlignment alignment) {
	setDefaultTextFont(font);
	setDefaultTextSize(size);
	setDefaultTextColor(color);
	setDefaultTextWeight(weight);
	setTextAlignment(alignment);
}

TextWeight TextPango::getDefaultTextWeight() {
	return mDefaultTextWeight;
}

void TextPango::setDefaultTextWeight(TextWeight weight) {
	if(mDefaultTextWeight != weight) {
		mDefaultTextWeight = weight;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;
	}
}

TextAlignment TextPango::getTextAlignment() {
	return mTextAlignment;
}

void TextPango::setTextAlignment(TextAlignment alignment) {
	if(mTextAlignment != alignment) {
		mTextAlignment = alignment;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;
	}
}

float TextPango::getSpacing() {
	return mSpacing;
}

void TextPango::setSpacing(float spacing) {
	if(mSpacing != spacing) {
		mSpacing = spacing;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;
	}
}

TextAntialias TextPango::getTextAntialias() {
	return mTextAntialias;
}

void TextPango::setTextAntialias(TextAntialias mode) {
	if(mTextAntialias != mode) {
		mTextAntialias = mode;
		mNeedsFontOptionUpdate = true;
		// TODO does this ever change metrics?
		mNeedsTextRender = true;
	}
}

ci::Vec2i TextPango::getMinSize() {
	return mMinSize;
}

void TextPango::setMinSize(int minWidth, int minHeight) {
	setMinSize(ci::Vec2i(minWidth, minHeight));
}

void TextPango::setMinSize(ci::Vec2i minSize) {
	if(mMinSize != minSize) {
		mMinSize = minSize;
		mNeedsMeasuring = true;
		// Might not need re-rendering
	}
}

ci::Vec2i TextPango::getMaxSize() {
	return mMaxSize;
}

void TextPango::setMaxSize(int maxWidth, int maxHeight) {
	setMaxSize(ci::Vec2i(maxWidth, maxHeight));
}

void TextPango::setMaxSize(ci::Vec2i maxSize) {
	if(mMaxSize != maxSize) {
		mMaxSize = maxSize;
		mNeedsMeasuring = true;
		// Might not need re-rendering
	}
}

ci::ColorA TextPango::getDefaultTextColor() {
	return mDefaultTextColor;
}

void TextPango::setDefaultTextColor(ci::ColorA color) {
	if(mDefaultTextColor != color) {
		mDefaultTextColor = color;
		mNeedsTextRender = true;
	}
}

ci::ColorA TextPango::getBackgroundColor() {
	return mBackgroundColor;
}

void TextPango::setBackgroundColor(ci::ColorA color) {
	if(mBackgroundColor != color) {
		mBackgroundColor = color;
		mNeedsTextRender = true;
	}
}

bool TextPango::getDefaultTextSmallCapsEnabled() {
	return mDefaultTextSmallCapsEnabled;
}

void TextPango::setDefaultTextSmallCapsEnabled(bool value) {
	if(mDefaultTextSmallCapsEnabled != value) {
		mDefaultTextSmallCapsEnabled = value;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;
	}
}

bool TextPango::getDefaultTextItalicsEnabled() {
	return mDefaultTextItalicsEnabled;
}

void TextPango::setDefaultTextItalicsEnabled(bool value) {
	if(mDefaultTextItalicsEnabled != value) {
		mDefaultTextItalicsEnabled = value;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;
	}
}

float TextPango::getDefaultTextSize() {
	return mDefaultTextSize;
}

void TextPango::setDefaultTextSize(float size) {
	if(mDefaultTextSize != size) {
		mDefaultTextSize = size;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;
	}
}

std::string TextPango::getDefaultTextFont() {
	return mDefaultTextFont;
}

void TextPango::setDefaultTextFont(std::string font) {
	if(mDefaultTextFont != font) {
		mDefaultTextFont = font;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;
	}
}

//#pragma mark - Pango Bridge

bool TextPango::render(bool force) {
	if(force || mNeedsFontUpdate || mNeedsMeasuring || mNeedsTextRender || mNeedsMarkupDetection) {

		if(force || mNeedsMarkupDetection) {

			// Pango doesn't support HTML-esque line-break tags, so
			// find break marks and replace with newlines, e.g. <br>, <BR>, <br />, <BR />
			std::regex e("<br\\s?/?>", std::regex_constants::icase);
			mProcessedText = std::regex_replace(mText, e, "\n");

			// Let's also decide and flag if there's markup in this string
			// Faster to use pango_layout_set_text than pango_layout_set_markup later on if
			// there's no markup to bother with.
			// Be pretty liberal, there's more harm in false-postives than false-negatives
			mProbablyHasMarkup = ((mProcessedText.find("<") != std::string::npos) && (mProcessedText.find(">") != std::string::npos));

			mNeedsMarkupDetection = false;
		}

		// First run, and then if the fonts change
		if(force || mNeedsFontOptionUpdate) {
			cairo_font_options_set_antialias(cairoFontOptions, static_cast<cairo_antialias_t>(mTextAntialias));

			// TODO, expose these?
			cairo_font_options_set_hint_style(cairoFontOptions, CAIRO_HINT_STYLE_FULL);
			cairo_font_options_set_hint_metrics(cairoFontOptions, CAIRO_HINT_METRICS_ON);
			//cairo_font_options_set_subpixel_order(cairoFontOptions, CAIRO_SUBPIXEL_ORDER_BGR);

			pango_cairo_context_set_font_options(pangoContext, cairoFontOptions);

			mNeedsFontOptionUpdate = false;
		}

		if(force || mNeedsFontUpdate) {
			if(fontDescription != nullptr) {
				pango_font_description_free(fontDescription);
			}

			fontDescription = pango_font_description_from_string(std::string(mDefaultTextFont + " " + std::to_string(mDefaultTextSize)).c_str());
			pango_font_description_set_weight(fontDescription, static_cast<PangoWeight>(mDefaultTextWeight));
			pango_font_description_set_style(fontDescription, mDefaultTextItalicsEnabled ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
			pango_font_description_set_variant(fontDescription, mDefaultTextSmallCapsEnabled ? PANGO_VARIANT_SMALL_CAPS : PANGO_VARIANT_NORMAL);
			pango_layout_set_font_description(pangoLayout, fontDescription);
			pango_font_map_load_font(mEngine.getPangoFontService().getPangoFontMap(), pangoContext, fontDescription);

			mNeedsFontUpdate = false;
		}

		bool needsSurfaceResize = false;

		// If the text or the bounds change
		if(force || mNeedsMeasuring) {

			const int lastPixelWidth = mPixelWidth;
			const int lastPixelHeight = mPixelHeight;

			pango_layout_set_width(pangoLayout, mMaxSize.x * PANGO_SCALE);
			pango_layout_set_height(pangoLayout, mMaxSize.y * PANGO_SCALE);

			// Pango separates alignment and justification... I prefer a simpler API here to handling certain edge cases.
			if(mTextAlignment == TextAlignment::JUSTIFY) {
				pango_layout_set_justify(pangoLayout, true);
				pango_layout_set_alignment(pangoLayout, static_cast<PangoAlignment>(TextAlignment::LEFT));
			} else {
				pango_layout_set_justify(pangoLayout, false);
				pango_layout_set_alignment(pangoLayout, static_cast<PangoAlignment>(mTextAlignment));
			}

			// pango_layout_set_wrap(pangoLayout, PANGO_WRAP_CHAR);
			pango_layout_set_spacing(pangoLayout, (int)mSpacing * PANGO_SCALE);

			// Set text, use the fastest method depending on what we found in the text
			if(mProbablyHasMarkup) {
				pango_layout_set_markup(pangoLayout, mProcessedText.c_str(), -1);
				//		pango_layout_set_markup_with_accel()
			} else {
				pango_layout_set_text(pangoLayout, mProcessedText.c_str(), -1);
			}

			// Measure text
			int newPixelWidth = 0;
			int newPixelHeight = 0;
			// use this instead: pango_layout_get_pixel_extents
			PangoRectangle inkRect;
			PangoRectangle extentRect;
			pango_layout_get_pixel_extents(pangoLayout, &inkRect, &extentRect);
			pango_layout_get_pixel_size(pangoLayout, &newPixelWidth, &newPixelHeight);

			// TODO: output a warning, and / or do a better job detecting and fixing issues or something
			if(newPixelWidth == 0 || newPixelHeight == 0){
				std::cout << "Uh oh, no size for pango layout." << std::endl;
			}

			//std::cout << "Ink rect: " << inkRect.x << " " << inkRect.y << " " << inkRect.width << " " << inkRect.height << std::endl;
			//std::cout << "Ext rect: " << extentRect.x << " " << extentRect.y << " " << extentRect.width << " " << extentRect.height << std::endl;

			// Some italics stuff extends beyond the normal widths
			mPixelWidth = extentRect.width + extentRect.x + 10; // newPixelWidth + 10;
			mPixelHeight = extentRect.height + extentRect.y; // newPixelHeight + 10;

			if(mPixelWidth < mMinSize.x) mPixelWidth = mMinSize.x;
			if(mPixelWidth > mMaxSize.x) mPixelWidth = mMaxSize.x;
			if(mPixelHeight < mMinSize.y) mPixelHeight = mMinSize.y;
			if(mPixelHeight > mMaxSize.y) mPixelHeight = mMaxSize.y;

			// Check for change, need to re-render if there's a change
			if((mPixelWidth != lastPixelWidth) || (mPixelHeight != lastPixelHeight)) {
				// Dimensions changed, re-draw text
				needsSurfaceResize = true;
			}

			mNeedsMeasuring = false;
		}


		// Create Cairo surface buffer to draw glyphs into
		// Force this is we need to render but don't have a surface yet
		bool freshCairoSurface = false;

		if(force || needsSurfaceResize || (mNeedsTextRender && (cairoSurface == nullptr))) {
			// Create appropriately sized cairo surface
			const bool grayscale = false; // Not really supported
			_cairo_format cairoFormat = grayscale ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32;

			// clean up any existing surfaces
			if(cairoSurface != nullptr) {
				cairo_surface_destroy(cairoSurface);
			}

#if CAIRO_HAS_WIN32_SURFACE
			cairoSurface = cairo_win32_surface_create_with_dib(cairoFormat, mPixelWidth, mPixelHeight);
#else
			cairoSurface = cairo_image_surface_create(cairoFormat, mPixelWidth, mPixelHeight);
#endif
			auto cairoSurfaceStatus = cairo_surface_status(cairoSurface);
			if(CAIRO_STATUS_SUCCESS != cairoSurfaceStatus) {
				DS_LOG_WARNING("Error creating Cairo surface.");
				return true;
			}

			// Create context
			/* create our cairo context object that tracks state. */
			if(cairoContext != nullptr) {
				cairo_destroy(cairoContext);
			}

			cairoContext = cairo_create(cairoSurface);

			auto cairoStatus = cairo_status(cairoContext);

			if(CAIRO_STATUS_NO_MEMORY == cairoStatus) {
				DS_LOG_WARNING("Out of memory, error creating Cairo context");
				return true;
			}

			if(CAIRO_STATUS_SUCCESS != cairoStatus){
				DS_LOG_WARNING("Error creating Cairo context " << cairoStatus);
				return true;
			}

			mNeedsTextRender = true;
			freshCairoSurface = true;
		}


		if(force || mNeedsTextRender) {
			// Render text

			if(!freshCairoSurface) {
				// Clear the context... if the background is clear and it's not a brand-new surface buffer
				cairo_save(cairoContext);
				cairo_set_operator(cairoContext, CAIRO_OPERATOR_CLEAR);
				cairo_paint(cairoContext);
				cairo_restore(cairoContext);
			}

			// Draw the text into the buffer
			cairo_set_source_rgba(cairoContext, mDefaultTextColor.r, mDefaultTextColor.g, mDefaultTextColor.b, mDefaultTextColor.a);
			pango_cairo_update_layout(cairoContext, pangoLayout);
			pango_cairo_show_layout(cairoContext, pangoLayout);

			//	cairo_surface_write_to_png(cairoSurface, "test_font.png");

			// Copy it out to a texture
#ifdef CAIRO_HAS_WIN32_SURFACE
			cairoImageSurface = cairo_win32_surface_get_image(cairoSurface);
			unsigned char *pixels = cairo_image_surface_get_data(cairoImageSurface);
#else
			unsigned char *pixels = cairo_image_surface_get_data(cairoSurface);
#endif


			// TODO!
			//if(mTexture == nullptr || (mTexture->getWidth() != mPixelWidth) || (mTexture->getHeight() != mPixelHeight)) {
			// Create a new texture if needed
			mTexture = ci::gl::Texture::create(pixels, GL_BGRA, mPixelWidth, mPixelHeight);
			mTexture->setMinFilter(GL_LINEAR);

			//	} else {
			// Update the existing texture?
			//	mTexture->update(pixels, GL_BGRA, GL_UNSIGNED_BYTE, 0, mPixelWidth, mPixelHeight);
			//}

			mNeedsTextRender = false;
		}

		return true;
	} else {
		return false;
	}
}

//#pragma mark - Static Methods

void TextPango::setTextRenderer(ds::ui::TextRenderer renderer) {
	std::string rendererName = "";

	switch(renderer) {
	case TextRenderer::PLATFORM_NATIVE:
#if defined(CINDER_MSW)
		rendererName = "win32";
#elif defined(CINDER_MAC)
		rendererName = "coretext";
#else
		DS_LOG_WARNING("Setting Pango text renderer not supported on this platform.");
#endif
		break;
	case TextRenderer::FREETYPE: {
		rendererName = "fontconfig";
	} break;
	}

	if(rendererName != "") {
#ifdef CINDER_MSW
		int status = _putenv_s("PANGOCAIRO_BACKEND", rendererName.c_str());
#else
		int status = setenv("PANGOCAIRO_BACKEND", rendererName.c_str(), 1); // this fixes some font issues on  mac
#endif
		if(status == 0) {
			DS_LOG_INFO("Set Pango Cairo backend renderer to: " << rendererName);
		} else {
			DS_LOG_WARNING("Error setting Pango Cairo backend environment variable.");
		}
	}
}

TextRenderer TextPango::getTextRenderer() {
	const char *rendererName = std::getenv("PANGOCAIRO_BACKEND");

	if(rendererName == nullptr) {
		DS_LOG_WARNING("Could not read Pango Cairo backend environment variable. Assuming native renderer.");
		return TextRenderer::PLATFORM_NATIVE;
	}

	std::string rendererNameString(rendererName);

	if((rendererNameString == "win32") || (rendererNameString == "coretext")) {
		return TextRenderer::PLATFORM_NATIVE;
	}

	if((rendererNameString == "fontconfig") || (rendererNameString == "fc")) {
		return TextRenderer::FREETYPE;
	}

	DS_LOG_WARNING("Unknown Pango Cairo backend environment variable: " << rendererNameString << ". Assuming native renderer.");
	return TextRenderer::PLATFORM_NATIVE;
}

} // namespace ui
} // namespace ds