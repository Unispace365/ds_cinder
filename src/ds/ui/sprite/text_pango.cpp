#include "text_pango.h"

#include <pango/pango-font.h>
#include <regex>

#if CAIRO_HAS_WIN32_SURFACE
#include <cairo-win32.h>
#endif

#include "ds/data/font_list.h"
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/service/pango_font_service.h"
#include "ds/util/string_util.h"

namespace {
// Pango/cairo output is premultiplied colors, so rendering it with opacity fades like you'd expect with other sprites
// requires a custom shader that multiplies in the rest of the opacity setting
const std::string opacityFrag =
"uniform sampler2D tex0;\n"
"uniform float opaccy;\n"
"void main()\n"
"{\n"
"    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    color = texture2D( tex0, gl_TexCoord[0].st );\n"
"    color *= gl_Color;\n"
"    color *= opaccy;\n"
"    gl_FragColor = color;\n"
"}\n";

const std::string vertShader =
"void main()\n"
"{\n"
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
"  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"  gl_FrontColor = gl_Color;\n"
"}\n";

std::string shaderNameOpaccy = "pango_text_opacity";
}

//#define USE_PANGO_FBO

namespace ds {
namespace ui {


namespace {
char				BLOB_TYPE = 0;

const DirtyState&	FONT_DIRTY = INTERNAL_A_DIRTY;
const DirtyState&	TEXT_DIRTY = INTERNAL_B_DIRTY;
const DirtyState&	LAYOUT_DIRTY = INTERNAL_C_DIRTY;

const char			FONTNAME_ATT = 80;
const char			TEXT_ATT = 81;
const char			LAYOUT_ATT = 82;
}

void TextPango::installAsServer(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void TextPango::installAsClient(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<TextPango>(r); });
}

TextPango::TextPango(ds::ui::SpriteEngine& eng)
	: ds::ui::Sprite(eng)
	, mText(L"")
	, mProcessedText(L"")
	, mNeedsMarkupDetection(false)
	, mNeedsFontUpdate(false)
	, mNeedsMeasuring(false)
	, mNeedsTextRender(false)
	, mNeedsFontOptionUpdate(false)
	, mProbablyHasMarkup(false)
	, mTextFont("Sans")
	, mTextSize(120.0)
	, mTextColor(ci::Color::white())
	, mDefaultTextItalicsEnabled(false)
	, mDefaultTextSmallCapsEnabled(false)
	, mResizeLimitWidth(1000000.0f)
	, mResizeLimitHeight(1000000.0f)
	, mLeading(0.0f)
	, mTextAlignment(Alignment::kLeft)
	, mDefaultTextWeight(TextWeight::kNormal)
	, mPixelWidth(-1)
	, mPixelHeight(-1)
	, mOutputShader(vertShader, opacityFrag, shaderNameOpaccy)
	, mFontDescription(nullptr)
	, mPangoContext(nullptr)
	, mPangoLayout(nullptr)
	, mCairoSurface(nullptr)
	, mCairoContext(nullptr)
	, mCairoFontOptions(nullptr)

#ifdef CAIRO_HAS_WIN32_SURFACE
	, mCairoWinImageSurface(nullptr)
#endif
{
	mBlobType = BLOB_TYPE;

	mOutputShader.loadShaders();

	if(!mEngine.getPangoFontService().getPangoFontMap()) {
		DS_LOG_WARNING("Cannot create the pango font map, nothing will render for this pango text sprite.");
		return;
	}

	// Create Pango Context for reuse
	mPangoContext = pango_font_map_create_context(mEngine.getPangoFontService().getPangoFontMap());
	if(nullptr == mPangoContext) {
		DS_LOG_WARNING("Cannot create the pango font context.");
		return;
	}

	// Create Pango Layout for reuse
	mPangoLayout = pango_layout_new(mPangoContext);
	if(mPangoLayout == nullptr) {
		DS_LOG_WARNING("Cannot create the pango layout.");
		return;
	}

	// Initialize Cairo surface and context, will be instantiated on demand
	mCairoFontOptions = cairo_font_options_create();
	if(mCairoFontOptions == nullptr) {
		DS_LOG_WARNING("Cannot create Cairo font options.");
		return;
	}

	// Generate the default font config
	mNeedsFontOptionUpdate = true;
	//mNeedsFontUpdate = true;

	setTransparent(false);
}

TextPango::~TextPango() {
	// This causes crash on windows
	if(mCairoContext) {
		cairo_destroy(mCairoContext);
		mCairoContext = nullptr;
	}

	if(mFontDescription) {
		pango_font_description_free(mFontDescription);
		mFontDescription = nullptr;
	}

	if(mCairoFontOptions) {
		cairo_font_options_destroy(mCairoFontOptions);
		mCairoFontOptions = nullptr;
	}

#ifdef CAIRO_HAS_WIN32_SURFACE
	if(mCairoWinImageSurface) {
		cairo_surface_destroy(mCairoWinImageSurface);
		mCairoWinImageSurface = nullptr;
	}
#else
	// Crashes windows...
	if(mCairoSurface != nullptr) {
		cairo_surface_destroy(mCairoSurface);
	}
#endif

	g_object_unref(mPangoContext); // this one crashes Windows?
	g_object_unref(mPangoLayout);
}

std::string TextPango::getTextAsString() const{
	return ds::utf8_from_wstr(mText);
}

std::wstring TextPango::getText() const {
	return mText;
}

void TextPango::setText(std::string text) {
	setText(ds::wstr_from_utf8(text));
}

void TextPango::setText(std::wstring text) {
	if(text != mText) {
		mText = text;
		mNeedsMarkupDetection = true;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;

		markAsDirty(TEXT_DIRTY);
	}
}

const ci::gl::TextureRef TextPango::getTexture() {
	return mTexture;
}

void TextPango::setTextStyle(std::string font, float size, ci::ColorA color, TextWeight weight,	Alignment::Enum alignment) {
	setFont(font);
	setFontSize(size);
	setColor(color);
	setDefaultTextWeight(weight);
	setAlignment(alignment);
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

		markAsDirty(FONT_DIRTY);
	}
}

Alignment::Enum TextPango::getAlignment() {
	return mTextAlignment;
}

void TextPango::setAlignment(Alignment::Enum alignment) {
	if(mTextAlignment != alignment) {
		mTextAlignment = alignment;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;
		
		markAsDirty(FONT_DIRTY);
	}
}

float TextPango::getLeading() const {
	return mLeading;
}

TextPango& TextPango::setLeading(const float leading) {
	if(mLeading != leading) {
		mLeading = leading;
		mNeedsMeasuring = true;
		mNeedsTextRender = true;

		markAsDirty(FONT_DIRTY);
	}
	return *this;
}

float TextPango::getResizeLimitWidth() const {
	return mResizeLimitWidth;
}

float TextPango::getResizeLimitHeight() const {
	return mResizeLimitHeight;
}

TextPango& TextPango::setResizeLimit(const float maxWidth, const float maxHeight) {
	if(mResizeLimitWidth != maxWidth || mResizeLimitHeight != maxHeight){
		mResizeLimitWidth = maxWidth;
		mResizeLimitHeight = maxHeight;

		if(mResizeLimitWidth < 1){
			mResizeLimitWidth = 1000000.0f;
		}

		if(mResizeLimitHeight < 1){
			mResizeLimitHeight = 1000000.0f;
		}
		mNeedsMeasuring = true;

		markAsDirty(LAYOUT_DIRTY);
	}

	return *this;
}

void TextPango::setTextColor(const ci::Color& color) {
	if(mTextColor != color) {
		mTextColor = color;
		mNeedsTextRender = true;

		markAsDirty(FONT_DIRTY);
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

		markAsDirty(FONT_DIRTY);
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

		markAsDirty(FONT_DIRTY);
	}
}

void TextPango::setFontSize(float size) {
	if(mTextSize != size) {
		mTextSize = size;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;

		markAsDirty(FONT_DIRTY);
	}
}

TextPango& TextPango::setFont(const std::string& font, const float fontSize) {
	if(mTextFont != font || mTextSize != fontSize) {
		mTextFont = mEngine.getFonts().getFileNameFromName(font);
	//	mTextFont = font;
		mTextSize = fontSize;
		mNeedsFontUpdate = true;
		mNeedsMeasuring = true;

		markAsDirty(FONT_DIRTY);

		if(!mEngine.getPangoFontService().getFamilyExists(mTextFont) && !mEngine.getPangoFontService().getFaceExists(mTextFont)){
			DS_LOG_WARNING("TextPango: Family or face not found: " << mTextFont);
		}
	}
	return *this;
}

TextPango& TextPango::setFont(const std::string& name){
	return setFont(name, mTextSize);
}

float TextPango::getWidth() const {
	if(mNeedsMeasuring) {
		(const_cast<TextPango*>(this))->render();
	}
	return mWidth;
}

float TextPango::getHeight() const {
	if(mNeedsMeasuring) {
		(const_cast<TextPango*>(this))->render();
	}
	return mHeight;
}

void TextPango::drawLocalClient(){
	if(mTexture){

		// The true flag is for premultiplied alpha, which this texture is
		ci::gl::enableAlphaBlending(true);		
		ci::gl::GlslProg& shaderBase = mOutputShader.getShader();
		if(shaderBase) {
			shaderBase.bind();
			shaderBase.uniform("tex0", 0);
			shaderBase.uniform("opaccy", mDrawOpacity);
			mUniform.applyTo(shaderBase);
		}

		mTexture->bind();
		if(getPerspective()){
			ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mTexture->getHeight()), static_cast<float>(mTexture->getWidth()), 0.0f));
		} else {
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture->getWidth()), static_cast<float>(mTexture->getHeight())));
		}
		mTexture->unbind();		

		if(shaderBase){
			shaderBase.unbind();
		}
	}
}

int TextPango::getCharacterIndexForPosition(const ci::Vec2f& lp){
	if(!mPangoLayout){
		render();
	}

	int outputIndex = 0;
	if(mPangoLayout){
		int trailing = 0;
		auto success = pango_layout_xy_to_index(mPangoLayout, (int)lp.x * PANGO_SCALE, (int)lp.y * PANGO_SCALE, &outputIndex, &trailing);
		outputIndex += trailing;
		// the "trailing" is if the xy is more than halfway to the next character. this is required to be added for the cursor to be able to be placed after the last character

	}	
	return outputIndex;
}

ci::Vec2f TextPango::getPositionForCharacterIndex(const int characterIndex){
	if(!mPangoLayout){
		render();
	}

	ci::Vec2f outputPos = ci::Vec2f::zero();
	if(mPangoLayout){
		PangoRectangle outputRectangle;
		pango_layout_index_to_pos(mPangoLayout, characterIndex, &outputRectangle);

		//std::cout << "position for index: " << characterIndex << " " << outputRectangle.x << " " << outputRectangle.y << std::endl;

		outputPos.x = (float)outputRectangle.x / (float)PANGO_SCALE;
		outputPos.y = (float)outputRectangle.y / (float)PANGO_SCALE;
	}
	return outputPos;
	
}

void TextPango::updateClient(const UpdateParams&){
	render();
}

void TextPango::updateServer(const UpdateParams&){
	render();
}

bool TextPango::render(bool force) {
	if(force || mNeedsFontUpdate || mNeedsMeasuring || mNeedsTextRender || mNeedsMarkupDetection) {

		if(force || mNeedsMarkupDetection) {

			// Pango doesn't support HTML-esque line-break tags, so
			// find break marks and replace with newlines, e.g. <br>, <BR>, <br />, <BR />
			// TODO
			//std::regex e(L"<br\\s?/?>", std::regex_constants::icase);
			//mProcessedText = std::regex_replace(mText, e, L"\n");
			mProcessedText = mText;

			// Let's also decide and flag if there's markup in this string
			// Faster to use pango_layout_set_text than pango_layout_set_markup later on if
			// there's no markup to bother with.
			// Be pretty liberal, there's more harm in false-postives than false-negatives
			mProbablyHasMarkup = ((mProcessedText.find(L"<") != std::wstring::npos) && (mProcessedText.find(L">") != std::wstring::npos));

			mNeedsMarkupDetection = false;
		}

		// First run, and then if the fonts change
		if(force || mNeedsFontOptionUpdate) {
			// TODO, expose these?
			
			cairo_font_options_set_antialias(mCairoFontOptions, CAIRO_ANTIALIAS_DEFAULT);
			cairo_font_options_set_hint_style(mCairoFontOptions, CAIRO_HINT_STYLE_DEFAULT);
			cairo_font_options_set_hint_metrics(mCairoFontOptions, CAIRO_HINT_METRICS_ON);
			//cairo_font_options_set_subpixel_order(mCairoFontOptions, CAIRO_SUBPIXEL_ORDER_RGB);

			pango_cairo_context_set_font_options(mPangoContext, mCairoFontOptions);

			mNeedsFontOptionUpdate = false;
		}

		if(force || mNeedsFontUpdate) {
			if(mFontDescription != nullptr) {
				pango_font_description_free(mFontDescription);
			}

			mFontDescription = pango_font_description_from_string(std::string(mTextFont + " " + std::to_string(mTextSize)).c_str());
		//	pango_font_description_set_weight(fontDescription, static_cast<PangoWeight>(mDefaultTextWeight));
		//	pango_font_description_set_style(mFontDescription, PANGO_STYLE_ITALIC);// mDefaultTextItalicsEnabled ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
		//	pango_font_description_set_variant(fontDescription, mDefaultTextSmallCapsEnabled ? PANGO_VARIANT_SMALL_CAPS : PANGO_VARIANT_NORMAL);
			pango_layout_set_font_description(mPangoLayout, mFontDescription);
			pango_font_map_load_font(mEngine.getPangoFontService().getPangoFontMap(), mPangoContext, mFontDescription);

		//	std::cout << pango_font_description_to_string(mFontDescription) << std::endl;

			mNeedsFontUpdate = false;
		}

		bool needsSurfaceResize = false;

		// If the text or the bounds change
		if(force || mNeedsMeasuring) {

			const int lastPixelWidth = mPixelWidth;
			const int lastPixelHeight = mPixelHeight;

			pango_layout_set_width(mPangoLayout, (int)mResizeLimitWidth * PANGO_SCALE);
			pango_layout_set_height(mPangoLayout, (int)mResizeLimitHeight * PANGO_SCALE);

			// Pango separates alignment and justification... I prefer a simpler API here to handling certain edge cases.
			if(mTextAlignment == Alignment::kJustify) {
				pango_layout_set_justify(mPangoLayout, true);
				pango_layout_set_alignment(mPangoLayout, PANGO_ALIGN_LEFT);
			} else {
				PangoAlignment aligny = PANGO_ALIGN_LEFT;
				if(mTextAlignment == Alignment::kCenter){
					aligny = PANGO_ALIGN_CENTER;
				} else if(mTextAlignment == Alignment::kRight){
					aligny = PANGO_ALIGN_RIGHT;
				} else if(mTextAlignment == Alignment::kJustify){ // handled above, but just to be safe
					aligny = PANGO_ALIGN_LEFT;
				}

				pango_layout_set_justify(mPangoLayout, false);
				pango_layout_set_alignment(mPangoLayout, aligny);
			}

			// pango_layout_set_wrap(pangoLayout, PANGO_WRAP_CHAR);
			pango_layout_set_spacing(mPangoLayout, (int)mLeading * PANGO_SCALE);

			// Set text, use the fastest method depending on what we found in the text
			if(mProbablyHasMarkup) {
				pango_layout_set_markup(mPangoLayout, ds::utf8_from_wstr(mProcessedText).c_str(), -1);
			} else {
				pango_layout_set_text(mPangoLayout, ds::utf8_from_wstr(mProcessedText).c_str(), -1);
			}

			// Measure text
			int newPixelWidth = 0;
			int newPixelHeight = 0;
			// use this instead: pango_layout_get_pixel_extents
			PangoRectangle inkRect;
			PangoRectangle extentRect;
			pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
			pango_layout_get_pixel_size(mPangoLayout, &newPixelWidth, &newPixelHeight);

			// TODO: output a warning, and / or do a better job detecting and fixing issues or something
			if((newPixelWidth == 0 || newPixelHeight == 0) && !mText.empty()){
				DS_LOG_WARNING("No size detected for pango text size. Font not detected or invalid markup are likely causes.");
			}

			//std::cout << "Ink rect: " << inkRect.x << " " << inkRect.y << " " << inkRect.width << " " << inkRect.height << std::endl;
			//std::cout << "Ext rect: " << extentRect.x << " " << extentRect.y << " " << extentRect.width << " " << extentRect.height << std::endl;

			// Some italics stuff extends beyond the normal widths
			mPixelWidth = extentRect.width + extentRect.x + 10;
			mPixelHeight = extentRect.height + extentRect.y;

			setSize((float)mPixelWidth, (float)mPixelHeight);

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

		if(force || needsSurfaceResize || (mNeedsTextRender && (mCairoSurface == nullptr))) {
			// Create appropriately sized cairo surface
			const bool grayscale = false; // Not really supported
			_cairo_format cairoFormat = grayscale ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32;

			// clean up any existing surfaces
			if(mCairoSurface != nullptr) {
				cairo_surface_destroy(mCairoSurface);
			}

#if CAIRO_HAS_WIN32_SURFACE
			mCairoSurface = cairo_win32_surface_create_with_dib(cairoFormat, mPixelWidth, mPixelHeight);
#else
			mCairoSurface = cairo_image_surface_create(cairoFormat, mPixelWidth, mPixelHeight);
#endif
			auto cairoSurfaceStatus = cairo_surface_status(mCairoSurface);
			if(CAIRO_STATUS_SUCCESS != cairoSurfaceStatus) {
				DS_LOG_WARNING("Error creating Cairo surface. " << mPixelWidth << " " << mPixelHeight);
				return true;
			}

			// Create context
			/* create our cairo context object that tracks state. */
			if(mCairoContext) {
				cairo_destroy(mCairoContext);
			}

			mCairoContext = cairo_create(mCairoSurface);

			auto cairoStatus = cairo_status(mCairoContext);

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


		if((force || mNeedsTextRender) && mCairoContext) {
			// Render text
			if(!freshCairoSurface) {
				// Clear the context... if the background is clear and it's not a brand-new surface buffer
				cairo_save(mCairoContext);
				cairo_set_operator(mCairoContext, CAIRO_OPERATOR_CLEAR);
				cairo_paint(mCairoContext);
				cairo_restore(mCairoContext);
			}

			// Draw the text into the buffer
			cairo_set_source_rgb(mCairoContext, mTextColor.r, mTextColor.g, mTextColor.b);// , getDrawOpacity());
			pango_cairo_update_layout(mCairoContext, mPangoLayout);
			pango_cairo_show_layout(mCairoContext, mPangoLayout);

			//	cairo_surface_write_to_png(cairoSurface, "test_font.png");

			// Copy it out to a texture
#ifdef CAIRO_HAS_WIN32_SURFACE
			mCairoWinImageSurface = cairo_win32_surface_get_image(mCairoSurface);
			unsigned char *pixels = cairo_image_surface_get_data(mCairoWinImageSurface);
#else
			unsigned char *pixels = cairo_image_surface_get_data(mCairoSurface);
#endif

			// Here we're copying the pixels into an intermediate texture, then drawing into an fbo
			// This is done entirely so the final output is not premultiplied alpha so it draws well with blend modes, opacity, etc.
			ci::gl::Texture::Format format;
			format.setMagFilter(GL_LINEAR);
			format.setMinFilter(GL_LINEAR);
			mTexture = ci::gl::Texture::create(pixels, GL_BGRA, mPixelWidth, mPixelHeight, format);
			mNeedsTextRender = false;
		}

		return true;
	} else {
		return false;
	}
}


void TextPango::writeAttributesTo(ds::DataBuffer& buf){
	ds::ui::Sprite::writeAttributesTo(buf);

	if(mDirty.has(TEXT_DIRTY)){
		buf.add(TEXT_ATT);
		buf.add(mText);
	}

	if(mDirty.has(FONT_DIRTY)) {
		buf.add(FONTNAME_ATT);
		buf.add(mTextFont);
		buf.add(mTextSize);
		buf.add(mLeading);
		buf.add(mTextColor);
		buf.add((int)mTextAlignment);
	}
	if(mDirty.has(LAYOUT_DIRTY)) {
		buf.add(LAYOUT_ATT);
		buf.add(mResizeLimitWidth);
		buf.add(mResizeLimitHeight);
	}
}

void TextPango::readAttributeFrom(const char attributeId, ds::DataBuffer& buf){
	if(attributeId == TEXT_ATT) {
		std::wstring theText = buf.read<std::wstring>();
		setText(theText);
	} else if(attributeId == FONTNAME_ATT) {

		std::string fontName = buf.read<std::string>();
		float fontSize = buf.read<float>();
		float leading = buf.read<float>();
		ci::Color fontColor = buf.read<ci::Color>();
		auto alignment = (ds::ui::Alignment::Enum)(buf.read<int>());

		setFont(fontName, fontSize);
		setLeading(leading);
		setTextColor(fontColor);
		setAlignment(alignment);

	} else if(attributeId == LAYOUT_ATT) {
		float rsw = buf.read<float>();
		float rsh = buf.read<float>();
		setResizeLimit(rsw, rsh);

	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

} // namespace ui
} // namespace ds