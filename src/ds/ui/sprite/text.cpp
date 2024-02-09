#include "stdafx.h"

#ifdef _WIN32
// Put a manifest dependency to the gtk/ directory so we can keep GStreamer dlls in their own directory
#pragma comment(linker, "/manifestdependency:\"name='gtk' version='1.0.0.0' type='win32'\"")
#endif

#include "text.h"

#include "cairo/cairo.h"
#include "cinder/CinderMath.h"
#include "fontconfig/fontconfig.h"
#include "pango/pangocairo.h"

#include <pango/pango-font.h>
#include <regex>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/font_list.h"
#include "ds/debug/logger.h"
#include "ds/ui/service/pango_font_service.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/float_util.h"
#include "ds/util/string_util.h"
#include <Poco/Stopwatch.h>

#ifdef _WIN32
// Put a manifest dependency to the gtk/ directory so we can keep Pango/GTK dlls in their own directory
#pragma comment(linker, "/manifestdependency:\"name='gtk' version='1.0.0.0' type='win32'\"")
#endif


namespace {
// Pango/cairo output is premultiplied colors, so rendering it with opacity fades like you'd expect with other sprites
// requires a custom shader that multiplies in the rest of the opacity setting
const std::string opacityPremultFrag = R"PREFRAG(
uniform sampler2D tex0;
uniform bool	  useTexture;  // dummy, Engine always sends this anyway
uniform bool	  preMultiply; // dummy, Engine always sends this anyway

in vec4	 Color;
in vec2	 TexCoord0;
out vec4 oColor;

void main() {

	oColor = vec4(1.0, 1.0, 1.0, 1.0);
	if (useTexture) {
		oColor = texture2D(tex0, vec2(TexCoord0.x, 1.0 - TexCoord0.y));
	}
	// Undo the pango premultiplication
	oColor.rgb /= oColor.a;
	// Now do the normal colorize/optional premultiplication
	oColor *= Color;
	if (preMultiply) oColor.rgb *= oColor.a;
}

)PREFRAG";


const std::string opacityFrag = R"FRAG(
uniform sampler2D tex0;

in vec4	 Color;
in vec2	 TexCoord0;
out vec4 oColor;

void main() {
	oColor	 = vec4(1.0, 1.0, 1.0, 1.0);
	oColor	 = texture2D(tex0, vec2(TexCoord0.x, 1.0 - TexCoord0.y));
	oColor.a = oColor.r;
	oColor.r = Color.r;
	oColor.g = Color.g;
	oColor.b = Color.b;
	oColor.a *= Color.a;
}
)FRAG";

const std::string vertShader = R"VERT(
uniform mat4 ciModelMatrix;
uniform mat4 ciModelViewProjection;
uniform vec4 uClipPlane0;
uniform vec4 uClipPlane1;
uniform vec4 uClipPlane2;
uniform vec4 uClipPlane3;

in vec4 ciPosition;
in vec4 ciColor;
in vec2 ciTexCoord0;

out vec2 TexCoord0;
out vec4 Color;

void main() {
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0	= ciTexCoord0;
	Color		= ciColor;

	gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);
	gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);
	gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);
	gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);
}
)VERT";

std::string shaderNameOpaccy = "pango_text_opacity";
std::string shaderNamePreser = "pango_text_preserve_colors";
} // namespace

namespace ds::ui {


namespace {
	char BLOB_TYPE = 0;

	const DirtyState& FONT_DIRTY   = INTERNAL_A_DIRTY;
	const DirtyState& TEXT_DIRTY   = INTERNAL_B_DIRTY;
	const DirtyState& LAYOUT_DIRTY = INTERNAL_C_DIRTY;

	const char FONTNAME_ATT = 80;
	const char TEXT_ATT		= 81;
	const char LAYOUT_ATT	= 82;
} // namespace


void Text::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
}

void Text::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<Text>(r); });
}

Text::Text(ds::ui::SpriteEngine& eng)
  : ds::ui::Sprite(eng)
  , mPangoContext(nullptr)
  , mPangoLayout(nullptr)
  , mText("")
  , mProcessedText("")
  , mProbablyHasMarkup(false)
  , mAllowMarkup(true)
  , mPreserveSpanColors(false)
  , mTrimWhiteSpace(false)
  , mEllipsizeMode(EllipsizeMode::kEllipsizeNone)
  , mWrapMode(WrapMode::kWrapModeWordChar)
  , mEngineFontScale(4.0 / 3.0)
  , mShrinkToBounds(false)
  , mResizeLimitWidth(-1.0f)
  , mResizeLimitHeight(-1.0f)
  , mFitToResizeLimit(false)
  , mNeedsMaxResizeFontSizeUpdate(false)
  , mNeedsRefit(false)
  , mFitCurrentTextSize(0)
  , mWrappedText(false)
  , mNumberOfLines(0)
  , mHasLists(false)
  , mNeedsFontUpdate(false)
  , mNeedsFontSizeUpdate(false)
  , mNeedsMeasuring(false)
  , mNeedsTextRender(false)
  , mNeedsFontOptionUpdate(false)
  , mNeedsMarkupDetection(false)
  , mPixelWidth(-1)
  , mPixelHeight(-1)
  , mPixelOffsetX(0)
  , mPixelOffsetY(0)
  , mCairoFontOptions(nullptr) {
	mBlobType = BLOB_TYPE;

	mEngineFontScale = mEngine.getEngineSettings().getFloat("font_scale", 0, 4.0f / 3.0f);

	if (!mEngine.getPangoFontService().getPangoFontMap()) {
		DS_LOG_WARNING("Cannot create the pango font map, nothing will render for this pango text sprite.");
		return;
	}

	// Create Pango Context for reuse
	mPangoContext = pango_font_map_create_context(mEngine.getPangoFontService().getPangoFontMap());
	if (nullptr == mPangoContext) {
		DS_LOG_WARNING("Cannot create the pango font context.");
		return;
	}

	// Create Pango Layout for reuse
	mPangoLayout = pango_layout_new(mPangoContext);
	if (mPangoLayout == nullptr) {
		DS_LOG_WARNING("Cannot create the pango layout.");
		return;
	}

	// Initialize Cairo surface and context, will be instantiated on demand
	mCairoFontOptions = cairo_font_options_create();
	if (mCairoFontOptions == nullptr) {
		DS_LOG_WARNING("Cannot create Cairo font options.");
		return;
	}

	// Generate the default font config
	mNeedsFontOptionUpdate = true;
	// mNeedsFontUpdate = true;

	setTransparent(false);
	setUseShaderTexture(true);
	mSpriteShader.setShaders(vertShader, opacityFrag, shaderNameOpaccy);
}

Text::~Text() {
	if (mCairoFontOptions) {
		cairo_font_options_destroy(mCairoFontOptions);
		mCairoFontOptions = nullptr;
	}

	g_object_unref(mPangoContext); // this one crashes Windows?
	g_object_unref(mPangoLayout);
}

std::string Text::getTextAsString() const {
	return mText;
}

std::wstring Text::getText() const {
	return ds::wstr_from_utf8(mText);
}

void Text::setText(std::string text) {
	if (text != mText) {
		if (!mText.empty()) {
			g_object_unref(mPangoLayout);
			mPangoLayout	 = pango_layout_new(mPangoContext);
			mNeedsFontUpdate = true;
		}

		mText						  = text;
		mNeedsMarkupDetection		  = true;
		mNeedsMeasuring				  = true;
		mNeedsTextRender			  = true;
		mNeedsRefit					  = true;
		mNeedsMaxResizeFontSizeUpdate = true;

		markAsDirty(TEXT_DIRTY);
	}
}

void Text::setText(std::wstring text) {
	setText(ds::utf8_from_wstr(text));
}

const ci::gl::TextureRef Text::getTexture() {
	return mTexture;
}

void Text::setTextStyle(std::string font, double size, ci::ColorA color, Alignment::Enum alignment) {
	setFont(font);
	setFontSize(size);
	setColor(color);
	setAlignment(alignment);
}

void Text::setTextStyle(ds::ui::TextStyle theStyle) {
	mStyle.mName = theStyle.mName;
	setFont(theStyle.mFont);
	setFontSize(theStyle.mSize);
	setFitFontSizes(theStyle.mFitSizes);
	setFitMaxFontSize(theStyle.mFitMaxTextSize);
	setFitMinFontSize(theStyle.mFitMinTextSize);
	setLeading(theStyle.mLeading);
	setLetterSpacing(theStyle.mLetterSpacing);
	setColorA(theStyle.mColor);
	setAlignment(theStyle.mAlignment);
}

void Text::setTextStyle(std::string styleName) {
	setTextStyle(mEngine.getEngineCfg().getTextStyle(styleName));
}

Alignment::Enum Text::getAlignment() {
	return mStyle.mAlignment;
}

void Text::setAlignment(Alignment::Enum alignment) {
	if (mStyle.mAlignment != alignment) {
		mStyle.mAlignment = alignment;
		mNeedsMeasuring	  = true;
		mNeedsTextRender  = true;
		mNeedsRefit		  = true;
		markAsDirty(FONT_DIRTY);
	}
}

double Text::getLeading() const {
	return mStyle.mLeading;
}

Text& Text::setLeading(const double leading) {
	if (mStyle.mLeading != leading) {
		mStyle.mLeading	 = leading;
		mNeedsMeasuring	 = true;
		mNeedsTextRender = true;
		mNeedsRefit		 = true;
		markAsDirty(FONT_DIRTY);
	}
	return *this;
}

double Text::getLetterSpacing() const {
	return mStyle.mLetterSpacing;
}

Text& Text::setLetterSpacing(const double letterSpacing) {
	if (mStyle.mLetterSpacing != letterSpacing) {
		mStyle.mLetterSpacing = letterSpacing;
		mNeedsMeasuring		  = true;
		mNeedsTextRender	  = true;
		mNeedsRefit			  = true;
		markAsDirty(FONT_DIRTY);
	}
	return *this;
}

float Text::getResizeLimitWidth() const {
	return mResizeLimitWidth;
}

float Text::getResizeLimitHeight() const {
	return mResizeLimitHeight;
}

Text& Text::setResizeLimit(const float maxWidth, const float maxHeight) {
	if (mResizeLimitWidth != maxWidth || mResizeLimitHeight != maxHeight) {
		mResizeLimitWidth  = maxWidth;
		mResizeLimitHeight = maxHeight;

		if (mResizeLimitWidth < 1) {
			mResizeLimitWidth = -1.0f; // negative one turns off text wrapping
		}

		/// prevent a situation where a default value of 0.0f only shows 1 line of text
		if (mResizeLimitHeight == 0.0f) {
			mResizeLimitHeight = -1.0f;
		}
		mNeedsMeasuring = true;
		mNeedsRefit		= true;
		markAsDirty(LAYOUT_DIRTY);
	}

	return *this;
}

Text& Text::setFitFontSizes(std::vector<double> font_sizes) {
	mStyle.mFitSizes = font_sizes;
	mNeedsRefit		 = true;
	mNeedsMeasuring	 = true;
	return *this;
}

Text& Text::setFitToResizeLimit(const bool fitToResize) {
	if (mFitToResizeLimit != fitToResize) {

		mNeedsFontSizeUpdate = true;
		mNeedsMeasuring		 = true;
		mFitToResizeLimit	 = fitToResize;
		mNeedsRefit			 = true;

		mNeedsMaxResizeFontSizeUpdate = true;
		markAsDirty(LAYOUT_DIRTY);
	}

	return *this;
}

bool Text::getShrinkToBounds() const {
	return mShrinkToBounds;
}

void Text::setShrinkToBounds(const bool shrinkToBounds /* = false */) {
	if (mShrinkToBounds == shrinkToBounds) return;

	mShrinkToBounds = shrinkToBounds;
	mNeedsMeasuring = true;

	markAsDirty(LAYOUT_DIRTY);
}

void Text::setTextColor(const ci::Color& color) {
	if (mStyle.mColor != color) {
		mStyle.mColor	 = color;
		mColor			 = color;
		mNeedsTextRender = true;

		markAsDirty(FONT_DIRTY);
	}
}

void Text::setFontSize(double size) {
	if (mStyle.mSize != size) {
		mStyle.mSize		 = size;
		mNeedsFontSizeUpdate = true;
		mNeedsMeasuring		 = true;

		markAsDirty(FONT_DIRTY);
	}
}

void Text::setFitMaxFontSize(double fontSize) {
	if (mStyle.mFitMaxTextSize != fontSize) {
		mStyle.mFitMaxTextSize = fontSize;
		mNeedsMeasuring		   = true;
		mNeedsRefit			   = true;
	}
}

void Text::setFitMinFontSize(double fontSize) {
	if (mStyle.mFitMinTextSize != fontSize) {
		mStyle.mFitMinTextSize = fontSize;
		mNeedsMeasuring		   = true;
		mNeedsRefit			   = true;
	}
}

void Text::setColor(const ci::Color& c) {
	setTextColor(c);
}

void Text::setColor(float r, float g, float b) {
	setTextColor(ci::Color(r, g, b));
}

void Text::setColorA(const ci::ColorA& c) {
	setTextColor(ci::Color(c));
	setOpacity(c.a);
}

Text& Text::setFont(const std::string& font) {
	if (mStyle.mFont != font) {
		mStyle.mFont				  = mEngine.getFonts().getFontNameForShortName(font);
		mNeedsFontUpdate			  = true;
		mNeedsMeasuring				  = true;
		mNeedsRefit					  = true;
		mNeedsMaxResizeFontSizeUpdate = true;
		markAsDirty(FONT_DIRTY);
	}
	return *this;
}

std::string Text::getFont() {
	return mStyle.mFont;
}

float Text::getWidth() const {
	if (mNeedsMeasuring) {
		(const_cast<Text*>(this))->measurePangoText();
	}
	return mWidth;
}

float Text::getHeight() const {
	if (mNeedsMeasuring) {
		(const_cast<Text*>(this))->measurePangoText();
	}
	return mHeight;
}

void Text::setEllipsizeMode(EllipsizeMode theMode) {
	if (theMode == mEllipsizeMode) return;

	mEllipsizeMode	= theMode;
	mNeedsMeasuring = true;
	mNeedsRefit		= true;
	markAsDirty(LAYOUT_DIRTY);
}

EllipsizeMode Text::getEllipsizeMode() {
	return mEllipsizeMode;
}

void Text::setWrapMode(WrapMode theMode) {
	if (theMode == mWrapMode) return;

	mWrapMode		= theMode;
	mNeedsMeasuring = true;
	mNeedsRefit		= true;
	markAsDirty(LAYOUT_DIRTY);
}

WrapMode Text::getWrapMode() {
	return mWrapMode;
}

void Text::onBuildRenderBatch() {
	float preWidth	= 0.0f;
	float preHeight = 0.0f;
	if (mTexture) {
		preWidth  = static_cast<float>(mTexture->getWidth());
		preHeight = static_cast<float>(mTexture->getHeight());
	}

	renderPangoText();


	if (!mTexture) {
		mRenderBatch = nullptr;
		return;
	}

	// if we already have a batch of this size, don't rebuild it
	if (mRenderBatch && preHeight == mTexture->getHeight() && preWidth == mTexture->getWidth()) {
		mNeedsBatchUpdate = false;
		return;
	}

	auto drawRect =
		ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture->getWidth()), static_cast<float>(mTexture->getHeight()));
	if (getPerspective()) {
		drawRect =
			ci::Rectf(0.0f, static_cast<float>(mTexture->getHeight()), static_cast<float>(mTexture->getWidth()), 0.0f);
	}
	auto theGeom = ci::geom::Rect(drawRect);
	if (mRenderBatch) {
		mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
	} else {
		mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
	}
}

void Text::setFlexboxAutoSizes() {
	measurePangoText();
	Sprite::setFlexboxAutoSizes();
}

void Text::drawLocalClient() {
	if (mTexture && !mText.empty()) {
		ci::gl::color(mStyle.mColor.r, mStyle.mColor.g, mStyle.mColor.b, mDrawOpacity);
		ci::gl::ScopedTextureBind scopedTexture(mTexture);

		ci::gl::ScopedModelMatrix scopedMat;
		ci::gl::translate(mRenderOffset);

		if (mRenderBatch) {
			if (getRevealTweenIsRunning()) {
				ci::gl::ScopedGlslProg scopedGlsl(mRenderBatch->getGlslProg());

				// Render only part of each line.
				const auto numLines = getNumberOfLines();
				if (numLines > 0) {
					const float durationPerLine = 1.0f / numLines;
					const float lineHeight		= mTexture->getHeight() / numLines;

					for (int i = 0; i < numLines; ++i) {
						float t = glm::clamp((getReveal() - i * durationPerLine) / durationPerLine, 0.0f, 1.0f);
						t		= ci::easeInOutSine(t);

						float vy0;
						float vy1;
						getLineRange(i, vy0, vy1);

						const float vx0 = 0;
						const float vx1 = t * float(mTexture->getWidth());
						const float tx0 = 0;
						const float tx1 = t;
						const float ty0 = 1.0f - vy0 / float(mTexture->getHeight());
						const float ty1 = 1.0f - vy1 / float(mTexture->getHeight());

						ci::gl::begin(GL_TRIANGLE_STRIP);
						ci::gl::texCoord(tx0, ty0);
						ci::gl::vertex(vx0, vy0);
						ci::gl::texCoord(tx1, ty0);
						ci::gl::vertex(vx1, vy0);
						ci::gl::texCoord(tx0, ty1);
						ci::gl::vertex(vx0, vy1);
						ci::gl::texCoord(tx1, ty1);
						ci::gl::vertex(vx1, vy1);
						ci::gl::end();
					}
				}
			} else
				mRenderBatch->draw();
		} else {
			ci::Rectf bounds = mTexture->getBounds();
			if (getRevealTweenIsRunning()) { // UNTESTED
				ci::gl::begin(GL_TRIANGLE_STRIP);
				ci::gl::texCoord(0, 1);
				ci::gl::vertex(0, 0);
				ci::gl::texCoord(getReveal(), 1);
				ci::gl::vertex(getReveal() * bounds.getWidth(), 0);
				ci::gl::texCoord(0, 0);
				ci::gl::vertex(0, bounds.getHeight());
				ci::gl::texCoord(getReveal(), 0);
				ci::gl::vertex(getReveal() * bounds.getWidth(), bounds.getHeight());
				ci::gl::end();
			} else if (getPerspective()) {
				ci::gl::drawSolidRect(ci::Rectf(0.0f, bounds.getHeight(), bounds.getWidth(), 0.0f));
			} else {
				ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, bounds.getWidth(), bounds.getHeight()));
			}
		}
		if (mFitToResizeLimit && mEngine.getEngineSettings().getBool("font:debug_fit_to_resize", 0)) {
			ci::gl::drawString(ci::toString(mFitCurrentTextSize), ci::vec2(0, 0), ci::ColorA(1, 0, 0, 1));
			ci::gl::ScopedColor sc(ci::ColorA(1, 0, 0, 1));
			ci::gl::drawStrokedRect(ci::Rectf(0, 0, mResizeLimitWidth, mResizeLimitHeight));
		}
	}
}

int Text::getCharacterIndexForPosition(const ci::vec2& lp) {
	measurePangoText();

	int outputIndex = 0;
	if (mPangoLayout) {
		int	 trailing = 0;
		auto success  = pango_layout_xy_to_index(mPangoLayout, (int)lp.x * PANGO_SCALE, (int)lp.y * PANGO_SCALE,
												 &outputIndex, &trailing);
		// the "trailing" is if the xy is more than halfway to the next character. this is required to be added for
		// the cursor to be able to be placed after the last character
		outputIndex += trailing;

		// std::cout << "Selected index: " << outputIndex << " " << ds::utf8_from_wstr(mText) << " " << mText.size()
		// << std::endl;
	}
	return outputIndex;
}
ci::vec2 Text::getPositionForCharacterIndex(const int characterIndex) {
	measurePangoText();

	ci::vec2 outputPos = ci::vec2();
	if (mPangoLayout) { // && !mText.empty()){
		PangoRectangle outputRectangle;
		pango_layout_index_to_pos(mPangoLayout, characterIndex, &outputRectangle);

		outputPos.x = (float)outputRectangle.x / (float)PANGO_SCALE;
		// Note: the rectangle returned is to the very top of the very tallest possible character (I think), which
		// makes it a good distance above the top of most characters So I fudged the output of this for a reasonable
		// position for the 'start' of each character from the top-left The exact output for the rectangle of each
		// character can be got from getRectForCharacterIndex()
		outputPos.y =
			(float)outputRectangle.y / (float)PANGO_SCALE + (float)outputRectangle.height / (float)PANGO_SCALE / 4.0f;
	}
	return outputPos;
}

ci::Rectf Text::getRectForCharacterIndex(const int characterIndex) {
	measurePangoText();

	ci::Rectf outputRect = ci::Rectf();
	if (mPangoLayout && !mText.empty()) {
		PangoRectangle outputRectangle;
		pango_layout_index_to_pos(mPangoLayout, characterIndex, &outputRectangle);

		float xx = (float)outputRectangle.x / (float)PANGO_SCALE;
		float yy = (float)outputRectangle.y / (float)PANGO_SCALE;
		outputRect.set(xx, yy, xx + (float)outputRectangle.width / (float)PANGO_SCALE,
					   yy + (float)outputRectangle.height / (float)PANGO_SCALE);
	}
	return outputRect;
}

float Text::getBaseline() {
	measurePangoText();

	if (mPangoLayout && !mText.empty()) {
		int baseline = pango_layout_get_baseline(mPangoLayout);
		return float(baseline) / float(PANGO_SCALE);
	} else {
		return 0.f;
	}
}

bool Text::setAvailableSize(const ci::vec2& size) {
	// Adjust resize limits.
	setResizeLimit(size.x, size.y);
	setFitToResizeLimit(true);

	// Measure minimum required space.
	bool hasChanged = false;
	if (!mMinWidth.isDefined() || !approxEqual(mMinWidth.asUser(this, css::Value::HORIZONTAL), getWidth())) {
		mMinWidth  = css::Value(getWidth(), css::Value::PIXELS);
		hasChanged = true;
	}
	if (!mMinHeight.isDefined() || !approxEqual(mMinHeight.asUser(this, css::Value::VERTICAL), getHeight())) {
		mMinHeight = css::Value(getHeight(), css::Value::PIXELS);
		hasChanged = true;
	}

	// Notify layout if settings have changed.
	return hasChanged;
}

void Text::fitInsideArea(const ci::Rectf& area) {
	setResizeLimit(area.getWidth(), area.getHeight());
	setFitToResizeLimit(true);

	Sprite::fitInsideArea(area);
}

bool Text::getTextWrapped() {
	// calculate current state if needed
	measurePangoText();
	return mWrappedText;
}

int Text::getNumberOfLines() {
	// calculate current state if needed
	measurePangoText();
	return mNumberOfLines;
}

float Text::getBaseLine(int index) {
	// calculate current state if needed
	measurePangoText();

	PangoLayoutIter* iter = pango_layout_get_iter(mPangoLayout);
	for (int i = 0; i < index && !pango_layout_iter_at_last_line(iter); ++i)
		pango_layout_iter_next_line(iter);
	int baseLine = pango_layout_iter_get_baseline(iter);

	pango_layout_iter_free(iter);

	return float(baseLine) / (float)PANGO_SCALE;
}

void Text::getLineRange(int index, float& y0, float& y1) {
	// calculate current state if needed
	measurePangoText();

	PangoLayoutIter* iter = pango_layout_get_iter(mPangoLayout);
	for (int i = 0; i < index && !pango_layout_iter_at_last_line(iter); ++i)
		pango_layout_iter_next_line(iter);

	int _y0;
	int _y1;
	pango_layout_iter_get_line_yrange(iter, &_y0, &_y1);

	pango_layout_iter_free(iter);

	y0 = float(_y0) / (float)PANGO_SCALE;
	y1 = float(_y1) / (float)PANGO_SCALE;
}

bool Text::getHasLists() {
	measurePangoText();
	return mHasLists;
}


void Text::setAllowMarkup(const bool allow) {
	mAllowMarkup				  = allow;
	mNeedsMarkupDetection		  = true;
	mNeedsMeasuring				  = true;
	mNeedsTextRender			  = true;
	mNeedsRefit					  = true;
	mNeedsMaxResizeFontSizeUpdate = true;
	markAsDirty(TEXT_DIRTY);
}

void Text::setPreserveSpanColors(const bool preserve) {
	mPreserveSpanColors = preserve;
	if (mPreserveSpanColors) {
		mSpriteShader.setShaders(vertShader, opacityPremultFrag, shaderNamePreser);
	} else {
		mSpriteShader.setShaders(vertShader, opacityFrag, shaderNameOpaccy);
	}
	mSpriteShader.loadShaders();
}

void Text::setTrimWhiteSpace(const bool trim) {
	if (mTrimWhiteSpace != trim) {
		mTrimWhiteSpace = trim;
		mNeedsMeasuring = true;
		markAsDirty(LAYOUT_DIRTY);
	}
}

void Text::onUpdateClient(const UpdateParams&) {
	measurePangoText();
}

void Text::onUpdateServer(const UpdateParams&) {
	measurePangoText();
	YGNodeMarkDirty(mYogaNode);
}


void Text::findFitFontSize() {

	if (mFitToResizeLimit && mNeedsRefit) {
		if (mStyle.mFitSizes.size() > 0) {
			findFitFontSizeFromArray();
			return;
		}
		//------------------------------------
		double		   fs		  = 5; // the starting font size.
		double		   set_fs	  = 5;
		double		   increment  = 1;
		PangoRectangle extentRect = PangoRectangle();
		PangoRectangle inkRect	  = PangoRectangle();

		auto				  constFontDescription = pango_layout_get_font_description(mPangoLayout);
		PangoFontDescription* fontDescription	   = nullptr;
		if (constFontDescription) {
			fontDescription = pango_font_description_copy(constFontDescription);
		}

		if (fontDescription) {
			auto _setFontSize = [this, fontDescription](double size) {
				pango_font_description_set_absolute_size(fontDescription, size * mEngineFontScale * 1024.0);
				pango_layout_set_font_description(mPangoLayout, fontDescription);
				pango_layout_set_spacing(mPangoLayout, (int)(size * (mStyle.mLeading - 1.0f)) * PANGO_SCALE);
			};
			// handle height;

			// set the height to a big as it goes so we can measure accurately.
			pango_layout_set_height(mPangoLayout, INT_MAX);

			// set the starting font size;
			_setFontSize(fs);
			set_fs = fs;

			// inital height measurement
			pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
			double h = std::max(extentRect.height, inkRect.height);

			while (h < mResizeLimitHeight) {

				// we are not over the limit so we set the font to the current value plus our increment
				_setFontSize(fs + increment);
				set_fs = fs + increment;


				// get the height
				pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
				h = std::max(extentRect.height, inkRect.height);

				// check if we are over the limit now and out last increment was greater than 1.
				// if both of those are true, we reset the increment to one, and try again from the last working
				// font size. this means the while check should only fail if the increment was 1.
				if (h >= mResizeLimitHeight && increment > 1) {
					// reset the increment
					increment = 1;

					// reset the font size to fs (without the increment)
					_setFontSize(fs);
					set_fs = fs;

					// remeasure the height
					pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
					h = std::max(extentRect.height, inkRect.height);

					continue;
				}

				// if we are still below the height (or the increment was 1)
				fs = fs + increment;
				increment *= 2;
			}

			auto height_fs = set_fs - 1.5;
			height_fs	   = mStyle.mFitMaxTextSize > 0 ? std::min(mStyle.mFitMaxTextSize, height_fs) : height_fs;
			height_fs	   = std::max(mStyle.mFitMinTextSize, height_fs);
			fs			   = height_fs;
			_setFontSize(fs);

			// handle width;
			if (mWrapMode == WrapMode::kWrapModeOff || mWrapMode == WrapMode::kWrapModeWord) {
				fs		  = 5;
				increment = 1;
				_setFontSize(fs);

				pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
				double w = std::max(extentRect.width, inkRect.width);
				while (w < mResizeLimitWidth) {

					// set font
					_setFontSize(fs + increment);

					// get height
					pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
					w = std::max(extentRect.width, inkRect.width);


					if (w >= mResizeLimitWidth && increment > 1) {
						increment = 1;
						// setFontSize(fs);
						_setFontSize(fs);
						// h = getHeight();
						pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
						w = std::max(extentRect.width, inkRect.width);
						continue;
					}
					fs = fs + increment;
					increment *= 2;
				}
				fs = fs - 1.5;

				// pick the smaller one;
				fs = std::min(height_fs, fs);
				fs = mStyle.mFitMaxTextSize > 0 ? std::min(mStyle.mFitMaxTextSize, fs) : fs;
				fs = std::max(mStyle.mFitMinTextSize, fs);

				_setFontSize(fs);
			}
			pango_font_description_free(fontDescription);
			pango_layout_set_height(mPangoLayout, (int)mResizeLimitHeight * PANGO_SCALE);

			mFitCurrentTextSize			  = fs;
			mNeedsFontUpdate			  = true;
			mNeedsRefit					  = false;
			mNeedsMaxResizeFontSizeUpdate = false;
			mNeedsTextRender			  = true;
			mNeedsMeasuring				  = true;
		}
		//-----------------------------------------------------------
	}
}

void Text::findFitFontSizeFromArray() {
	// DS_LOG_INFO("****** Pick Array font size: ");
	if (mFitToResizeLimit && mNeedsRefit) {

		if (mStyle.mFitSizes.empty()) return;

		//------------------------------------
		double		   fs		  = 5;
		int			   idx		  = 0;
		PangoRectangle extentRect = PangoRectangle();
		PangoRectangle inkRect	  = PangoRectangle();

		auto				  constFontDescription = pango_layout_get_font_description(mPangoLayout);
		PangoFontDescription* fontDescription	   = nullptr;
		if (constFontDescription) {
			fontDescription = pango_font_description_copy(constFontDescription);
		}


		if (fontDescription) {
			auto _setFontSize = [this, fontDescription](double size) {
				pango_font_description_set_absolute_size(fontDescription, size * mEngineFontScale * 1024.0);
				pango_layout_set_font_description(mPangoLayout, fontDescription);
				pango_layout_set_spacing(mPangoLayout, (int)(size * (mStyle.mLeading - 1.0f)) * PANGO_SCALE);
			};

			// set the height to a big as it goes so we can measure accurately.
			pango_layout_set_height(mPangoLayout, INT_MAX);

			// sort the font sizes (default small to large)
			std::sort(mStyle.mFitSizes.begin(), mStyle.mFitSizes.end());

			// handle height;
			fs = mStyle.mFitSizes[idx];
			_setFontSize(fs);
			// DS_LOG_INFO("Start At font size: " << mStyle.mFitSizes[idx]);
			pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);

			auto offsety = inkRect.y;

			// DS_LOG_INFO("offset: "<<offsety);
			double h	   = std::max(extentRect.height, inkRect.height) + offsety;
			double limit_h = mResizeLimitHeight;
			while (h < limit_h) {
				if (idx >= mStyle.mFitSizes.size() - 1) {
					// DS_LOG_INFO("Run out of sizes. ");
					break;
				}

				fs = mStyle.mFitSizes[++idx];
				// set font
				_setFontSize(fs);
				// DS_LOG_INFO("At font size: " << fs);

				// get height
				pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
				h = std::max(extentRect.height, inkRect.height) + offsety;

				if (h >= limit_h) {
					// DS_LOG_INFO("Over limit rewinding idx");
					idx--;
					break;

				} else {
					// DS_LOG_INFO("Good With: h of " << h <<" and mResizeLimitHeight of " << mResizeLimitHeight);
				}
			}

			// DS_LOG_INFO("Picked height font size: " << mStyle.mFitSizes[idx]);

			// fs = getFontSize() - 1.5;
			auto height_fs = mStyle.mFitSizes[idx];
			height_fs	   = mStyle.mFitMaxTextSize > 0 ? std::min(mStyle.mFitMaxTextSize, height_fs) : height_fs;
			height_fs	   = std::max(mStyle.mFitMinTextSize, height_fs);

			_setFontSize(height_fs);
			fs = height_fs;
			if (mWrapMode == WrapMode::kWrapModeOff || mWrapMode == WrapMode::kWrapModeWord) {
				// handle width;
				idx = 0;
				fs	= mStyle.mFitSizes[idx];

				_setFontSize(fs);

				pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
				double w = std::max(extentRect.width, inkRect.width);
				while (w < mResizeLimitWidth) {

					if (idx >= mStyle.mFitSizes.size() - 1) {
						break;
					}

					fs = mStyle.mFitSizes[++idx];
					// set font
					_setFontSize(fs);

					// get height
					pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);
					w = std::max(extentRect.width, inkRect.width);

					if (w > mResizeLimitWidth) {
						idx--;
						break;
					}
				}
				fs = mStyle.mFitSizes[idx];
				// pick the smaller one;
				fs = std::min(height_fs, fs);
				fs = mStyle.mFitMaxTextSize > 0 ? std::min(mStyle.mFitMaxTextSize, fs) : fs;
				fs = std::max(mStyle.mFitMinTextSize, fs);

				_setFontSize(fs);
			}
			// DS_LOG_INFO("Picked width font size: " << mStyle.mFitSizes[idx]);
			pango_font_description_free(fontDescription);
			pango_layout_set_height(mPangoLayout, (int)mResizeLimitHeight * PANGO_SCALE);
			mFitCurrentTextSize			  = fs;
			mNeedsFontUpdate			  = true;
			mNeedsRefit					  = false;
			mNeedsMaxResizeFontSizeUpdate = false;
			mNeedsTextRender			  = true;
			mNeedsMeasuring				  = true;
		}
		//-----------------------------------------------------------
	}
}

bool Text::parseLists() {
	if (mProcessedText.empty()) {
		return false;
	}

	bool isOrdered = true;

	auto firstInstance = mProcessedText.find("<ol>");

	if (firstInstance == std::string::npos) {
		isOrdered	  = false;
		firstInstance = mProcessedText.find("<ul>");
	}

	if (firstInstance == std::string::npos) {
		return false;
	}

	size_t listEnd = std::string::npos;

	if (isOrdered) {
		listEnd = mProcessedText.find("</ol>");
	} else {
		listEnd = mProcessedText.find("</ul>");
	}

	if (listEnd == std::string::npos) {
		DS_LOG_WARNING("Text: Found an opening ordered list tag but no closing tag!");
		return false;
	}

	std::string beforeText = mProcessedText.substr(0, firstInstance);
	std::string theList =
		mProcessedText.substr(firstInstance + 4, listEnd - firstInstance - 4); // the minus four is to remove the tags
	std::string afterText = mProcessedText.substr(listEnd + 5);

	if (isOrdered) {
		int listNumber = 1;

		size_t itemPos = theList.find("<li>");
		while (itemPos != std::string::npos) {
			theList = theList.replace(itemPos, 4, std::to_string(listNumber) + ". ");
			listNumber++;
			ds::replace(theList, "</li>", "");
			itemPos = theList.find("<li>");
		}

	} else {
		ds::replace(theList, "<li>", ds::utf8_from_wstr(L"• "));
		ds::replace(theList, "</li>", "");
	}

	mProcessedText = beforeText + theList + afterText;
	return true;
}

bool Text::measurePangoText() {
	if (mNeedsFontUpdate || mNeedsMeasuring || mNeedsMarkupDetection) {


		if (mText.empty() || mStyle.mSize <= 0.0f) {
			if (mWidth > 0.0f || mHeight > 0.0f) {
				setSize(0.0f, 0.0f);
			}
			mNeedsMarkupDetection = false;
			mNeedsMeasuring		  = false;
			mNeedsBatchUpdate	  = true;
			return false;
		}

		double textSize = mStyle.mSize;
		if (mFitToResizeLimit && mFitCurrentTextSize > 0) {
			textSize = mFitCurrentTextSize;
		}

		mNeedsTextRender = true;
		bool hadMarkup	 = mProbablyHasMarkup;

		if (mNeedsMarkupDetection) {

			// Pango doesn't support HTML-esque line-break tags, so
			// find break marks and replace with newlines, e.g. <br>, <BR>, <br />, <BR />
			std::regex e("<br\\s?/?>", std::regex_constants::icase);
			mProcessedText = std::regex_replace(mText, e, "\n");

			if (mAllowMarkup) {
				// Let's also decide and flag if there's markup in this string
				// Faster to use pango_layout_set_text than pango_layout_set_markup later on if
				// there's no markup to bother with.
				// Be pretty liberal, there's more harm in false-postives than false-negatives
				bool hasAmps	   = mProcessedText.find("&amp;") != std::string::npos;
				mProbablyHasMarkup = ((mProcessedText.find("<") != std::string::npos) &&
									  (mProcessedText.find(">") != std::string::npos)) ||
									 hasAmps;

				// parse any lists
				if (mProbablyHasMarkup) {
					mHasLists		  = false;
					bool hasMoreLists = true;
					while (hasMoreLists) {
						hasMoreLists = parseLists();
						if (hasMoreLists) {
							mHasLists = true;
						}
					}

					if (!hasAmps && mProcessedText.find("&") != std::string::npos) {
						ds::replace(mProcessedText, "&", "&amp;");
					}
				}
			} else {
				hadMarkup		   = false;
				mProbablyHasMarkup = false;
			}

			mNeedsMarkupDetection = false;
		}

		// First run, and then if the fonts change
		if (mNeedsFontOptionUpdate) {
			// TODO, expose these?

			cairo_font_options_set_antialias(mCairoFontOptions, CAIRO_ANTIALIAS_SUBPIXEL);
			cairo_font_options_set_hint_style(mCairoFontOptions, CAIRO_HINT_STYLE_DEFAULT);
			cairo_font_options_set_hint_metrics(mCairoFontOptions, CAIRO_HINT_METRICS_ON);
			cairo_font_options_set_subpixel_order(mCairoFontOptions, CAIRO_SUBPIXEL_ORDER_BGR);

			pango_cairo_context_set_font_options(mPangoContext, mCairoFontOptions);

			mNeedsFontOptionUpdate = false;
		}

		if (mNeedsFontUpdate || mNeedsFontSizeUpdate) {

			PangoFontDescription* fontDescription =
				pango_font_description_from_string(mStyle.mFont.c_str()); // +" " + std::to_string(textSize)).c_str());
			pango_font_description_set_absolute_size(fontDescription, textSize * mEngineFontScale * 1024.0);
			pango_layout_set_font_description(mPangoLayout, fontDescription);
			if (mNeedsFontUpdate) {
				pango_font_map_load_font(mEngine.getPangoFontService().getPangoFontMap(), mPangoContext,
										 fontDescription);
			}
			pango_font_description_free(fontDescription);

			mNeedsFontUpdate	 = false;
			mNeedsFontSizeUpdate = false;
		}

		// If the text or the bounds change
		if (mNeedsMeasuring) {
			pango_layout_set_width(mPangoLayout, (int)mResizeLimitWidth * PANGO_SCALE);
			if (mWrapMode == WrapMode::kWrapModeOff) {
				if (mEllipsizeMode == EllipsizeMode::kEllipsizeNone) {
					pango_layout_set_width(mPangoLayout, -1);
				}
				pango_layout_set_height(mPangoLayout, (int)0);
			} else if (mResizeLimitHeight < 0) {
				pango_layout_set_height(mPangoLayout, (int)mResizeLimitHeight);
			} else {
				pango_layout_set_height(mPangoLayout, (int)mResizeLimitHeight * PANGO_SCALE);
			}

			// Pango separates alignment and justification... I prefer a simpler API here to handling certain edge
			// cases.
			if (mStyle.mAlignment == Alignment::kJustify) {
				pango_layout_set_justify(mPangoLayout, true);
				pango_layout_set_alignment(mPangoLayout, PANGO_ALIGN_LEFT);
			} else {
				PangoAlignment aligny = PANGO_ALIGN_LEFT;
				if (mStyle.mAlignment == Alignment::kCenter) {
					aligny = PANGO_ALIGN_CENTER;
				} else if (mStyle.mAlignment == Alignment::kRight) {
					aligny = PANGO_ALIGN_RIGHT;
				} else if (mStyle.mAlignment == Alignment::kJustify) { // handled above, but just to be safe
					aligny = PANGO_ALIGN_LEFT;
				}

				pango_layout_set_justify(mPangoLayout, false);
				pango_layout_set_alignment(mPangoLayout, aligny);
			}

			if (mWrapMode == WrapMode::kWrapModeChar) {
				pango_layout_set_wrap(mPangoLayout, PANGO_WRAP_CHAR);
			} else if (mWrapMode == WrapMode::kWrapModeWord) {
				pango_layout_set_wrap(mPangoLayout, PANGO_WRAP_WORD);
			} else {
				pango_layout_set_wrap(mPangoLayout, PANGO_WRAP_WORD_CHAR);
			}

			PangoEllipsizeMode elipsizeMode = PANGO_ELLIPSIZE_NONE;
			if (mEllipsizeMode == EllipsizeMode::kEllipsizeEnd) {
				elipsizeMode = PANGO_ELLIPSIZE_END;
			} else if (mEllipsizeMode == EllipsizeMode::kEllipsizeMiddle) {
				elipsizeMode = PANGO_ELLIPSIZE_MIDDLE;
			} else if (mEllipsizeMode == EllipsizeMode::kEllipsizeStart) {
				elipsizeMode = PANGO_ELLIPSIZE_START;
			}

			pango_layout_set_ellipsize(mPangoLayout, elipsizeMode);
			pango_layout_set_spacing(mPangoLayout, (int)(textSize * (mStyle.mLeading - 1.0f)) * PANGO_SCALE);

			// Set text, use the fastest method depending on what we found in the text
			int newPixelWidth  = 0;
			int newPixelHeight = 0;
			if (mProbablyHasMarkup) {
				pango_layout_set_markup(mPangoLayout, mProcessedText.c_str(), static_cast<int>(mProcessedText.size()));

				// check the pixel size, if it's empty, then we can try again without markup
				pango_layout_get_pixel_size(mPangoLayout, &newPixelWidth, &newPixelHeight);
			}

			if (!mProbablyHasMarkup || newPixelWidth < 1) {
				if (hadMarkup) {
					pango_layout_set_markup(mPangoLayout, mProcessedText.c_str(),
											static_cast<int>(mProcessedText.size()));
				}

				pango_layout_set_text(mPangoLayout, mProcessedText.c_str(), -1);
			}

			if (mStyle.mLetterSpacing != 0.0f) {
				auto attrs		= pango_layout_get_attributes(mPangoLayout);
				bool createdNew = false;
				if (attrs == nullptr) {
					attrs	   = pango_attr_list_new();
					createdNew = true;
				}

				// Set letter spacing: 0.0f=normal; 1.0f = 1pt extra spacing;
				pango_attr_list_insert(attrs,
									   pango_attr_letter_spacing_new((int)(mStyle.mLetterSpacing * PANGO_SCALE)));

				// Enable ligatures, kerning, and auto-conversion of simple fractions to a single character
				// representation
				// pango_attr_list_insert(attrs, pango_attr_font_features_new("liga=1, -kern, afrc on, frac on"));

				pango_layout_set_attributes(mPangoLayout, attrs);

				if (createdNew) {
					pango_attr_list_unref(attrs);
				}
			}

			// If we are sizing for limits we do that logic here after all the attributes have be set.
			// at the end of this only the font size should be changed.
			findFitFontSize();


			mWrappedText   = pango_layout_is_wrapped(mPangoLayout) != FALSE;
			mNumberOfLines = pango_layout_get_line_count(mPangoLayout);


			// use this instead: pango_layout_get_pixel_extents
			PangoRectangle extentRect = PangoRectangle();
			PangoRectangle inkRect	  = PangoRectangle();
			pango_layout_get_pixel_extents(mPangoLayout, &inkRect, &extentRect);

			// The offset for rendering to the cairo surface
			mPixelOffsetX = -extentRect.x;
			mPixelOffsetY = -extentRect.y;

			// Instead of making the image textue larger, we will offset the drawing to the correct position
			mRenderOffset = ci::vec2(extentRect.x, extentRect.y);

			// To account for the case where the inkRect goes outside of the extentRect:
			//   move the cairo & render offsets appropriately by opposite amounts
			if (inkRect.x < extentRect.x) {
				mRenderOffset.x -= extentRect.x - inkRect.x;
				mPixelOffsetX += extentRect.x - inkRect.x;
			}

			if (inkRect.y < extentRect.y) {
				mRenderOffset.y -= extentRect.y - inkRect.y;
				mPixelOffsetY += extentRect.y - inkRect.y;
			}

			if ((extentRect.width == 0 || extentRect.height == 0) && !mText.empty()) {
				DS_LOG_WARNING("No size detected for pango text size. Font not detected or invalid markup are "
							   "likely causes. Text: "
							   << getTextAsString());
			}

			// DS_LOG_INFO("the Text: " << getTextAsString());
			// DS_LOG_INFO("Ink rect: " << inkRect.x << " " << inkRect.y << " " << inkRect.width << " " <<
			// inkRect.height); DS_LOG_INFO("Ext rect: " << extentRect.x << " " << extentRect.y << " " <<
			// extentRect.width << " " << extentRect.height << "\n");

			// Set the final width/height for the texture, handling the case where inkRect is larger than extentRect
			mPixelWidth	 = std::max(extentRect.width, inkRect.width);
			mPixelHeight = std::max(extentRect.height, inkRect.height);

			// Adjust size and render offset when trimming white space
			if (mTrimWhiteSpace) {
				switch (mStyle.mAlignment) {
				case Alignment::kCenter:
					mRenderOffset -=
						ci::vec2(inkRect.x + inkRect.width / 2 - (extentRect.x + extentRect.width / 2), inkRect.y);
					break;
				case Alignment::kRight:
					mRenderOffset -= ci::vec2(inkRect.x + inkRect.width - (extentRect.x + extentRect.width), inkRect.y);
					break;
				case Alignment::kLeft:
				case Alignment::kJustify:
					mRenderOffset -= ci::vec2(inkRect.x, inkRect.y);
					break;
				}
			}

			const auto pixelWidth  = float(mTrimWhiteSpace ? inkRect.width : mPixelWidth);
			const auto pixelHeight = float(mTrimWhiteSpace ? inkRect.height : mPixelHeight);

			// This is required to not break combinations of layout align & text align
			if (extentRect.width < (int)mResizeLimitWidth) {
				if (!mShrinkToBounds) {
					setSize(mResizeLimitWidth, pixelHeight);
				} else {
					mRenderOffset.x -= extentRect.x;
					setSize(pixelWidth, pixelHeight);
				}
			} else {
				setSize(pixelWidth, pixelHeight);
			}
			YGNodeMarkDirty(mYogaNode);

			mNeedsMeasuring = false;
		}

		mNeedsBatchUpdate = true;
		return true;
	} else {
		return false;
	}
}

void Text::renderPangoText() {
	if (mNeedsTextRender && mPixelWidth > 0 && mPixelHeight > 0) {

		_cairo_format cairoFormat = mPreserveSpanColors ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_A8;

		cairo_surface_t* cairoSurface = cairo_image_surface_create(cairoFormat, mPixelWidth, mPixelHeight);

		auto cairoSurfaceStatus = cairo_surface_status(cairoSurface);
		if (CAIRO_STATUS_SUCCESS != cairoSurfaceStatus) {
			DS_LOG_WARNING("Error creating Cairo surface. Status:" << cairoSurfaceStatus << " w:" << mPixelWidth
																   << " h:" << mPixelHeight << " text:" << mText);
			// make sure we don't render garbage
			if (mTexture) {
				mTexture = nullptr;
			}
			return;
		}

		cairo_t* cairoContext = nullptr;
		if (cairoSurface) {
			// Create context
			cairoContext = cairo_create(cairoSurface);

			auto cairoStatus = cairo_status(cairoContext);

			if (CAIRO_STATUS_NO_MEMORY == cairoStatus) {
				DS_LOG_WARNING("Out of memory, error creating Cairo context");
				cairo_surface_destroy(cairoSurface);
				return;
			}

			if (CAIRO_STATUS_SUCCESS != cairoStatus) {
				DS_LOG_WARNING("Error creating Cairo context " << cairoStatus);
				cairo_surface_destroy(cairoSurface);
				return;
			}
		}

		if (cairoContext) {

			// Draw the text into the buffer
			cairo_set_source_rgb(cairoContext, mStyle.mColor.r, mStyle.mColor.g, mStyle.mColor.b);

			// Move the layout into the correct position on the surface/context before drawing!
			// This removes the need for additional texture padding & fixes clipping ascenders
			cairo_translate(cairoContext, mPixelOffsetX, mPixelOffsetY);
			pango_cairo_update_layout(cairoContext, mPangoLayout);
			pango_cairo_show_layout(cairoContext, mPangoLayout);

			//	cairo_surface_write_to_png(cairoSurface, "test_font.png");

			// Copy it out to a texture
			unsigned char* pixels = cairo_image_surface_get_data(cairoSurface);

			ci::gl::Texture::Format format;
			format.enableMipmapping(mEngine.getEngineSettings().getBool("text_mipmap", 0, false));

			if (mPreserveSpanColors) {
				mTexture = ci::gl::Texture::create(pixels, GL_BGRA, mPixelWidth, mPixelHeight, format);
			} else {
				auto imgWitdh = mPixelWidth;
				if (imgWitdh % 4 != 0) {
					imgWitdh += 4 - imgWitdh % 4;
				}
				format.setInternalFormat(GL_RED);
				format.setDataType(GL_UNSIGNED_BYTE);
				mTexture = ci::gl::Texture::create(pixels, GL_RED, imgWitdh, mPixelHeight, format);
			}

			mTexture->setTopDown(true);

			mNeedsTextRender = false;

			cairo_destroy(cairoContext);
		}

		if (cairoSurface) {
			cairo_surface_destroy(cairoSurface);
		}
	}
}

void Text::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if (mDirty.has(TEXT_DIRTY)) {
		buf.add(TEXT_ATT);
		buf.add(mText);
	}

	if (mDirty.has(FONT_DIRTY)) {
		buf.add(FONTNAME_ATT);
		buf.add(mStyle.mFont);
		buf.add(mStyle.mSize);
		buf.add(mStyle.mLeading);
		buf.add(mStyle.mLetterSpacing);
		buf.add(mStyle.mColor);
		buf.add((int)mStyle.mAlignment);
	}
	if (mDirty.has(LAYOUT_DIRTY)) {
		buf.add(LAYOUT_ATT);
		buf.add(mResizeLimitWidth);
		buf.add(mResizeLimitHeight);
		buf.add(mFitToResizeLimit);
		buf.add((int)mStyle.mFitSizes.size());
		for (auto font_size : mStyle.mFitSizes) {
			buf.add(font_size);
		}
		buf.add(mStyle.mFitMinTextSize);
		buf.add(mStyle.mFitMaxTextSize);
		buf.add((int)mEllipsizeMode);
		buf.add((int)mWrapMode);
		buf.add(mShrinkToBounds);
	}
}

void Text::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == TEXT_ATT) {
		setText(buf.read<std::string>());
	} else if (attributeId == FONTNAME_ATT) {

		std::string fontName	  = buf.read<std::string>();
		double		fontSize	  = buf.read<double>();
		double		leading		  = buf.read<double>();
		double		letterSpacing = buf.read<double>();
		ci::ColorA	fontColor	  = buf.read<ci::ColorA>();
		auto		alignment	  = (ds::ui::Alignment::Enum)(buf.read<int>());

		setFont(fontName);
		setFontSize(fontSize);
		setLeading(leading);
		setLetterSpacing(letterSpacing);
		setTextColor(fontColor);
		setAlignment(alignment);

	} else if (attributeId == LAYOUT_ATT) {
		float				rsw		   = buf.read<float>();
		float				rsh		   = buf.read<float>();
		bool				fit		   = buf.read<bool>();
		int					font_count = buf.read<int>();
		std::vector<double> fontSizes;
		for (int i = 0; i < font_count; i++) {
			auto font_size = buf.read<double>();
			fontSizes.push_back(font_size);
		}
		double fontMinSize	= buf.read<double>();
		double fontMaxSize	= buf.read<double>();
		auto   ellipsesMode = (EllipsizeMode)(buf.read<int>());
		auto   wrapMode		= (WrapMode)(buf.read<int>());
		auto   shrink		= buf.read<bool>();

		setResizeLimit(rsw, rsh);
		setFitToResizeLimit(fit);
		setFitFontSizes(fontSizes);
		setFitMinFontSize(fontMinSize);
		setFitMaxFontSize(fontMaxSize);
		setEllipsizeMode(ellipsesMode);
		setWrapMode(wrapMode);
		setShrinkToBounds(shrink);
	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

} // namespace ds::ui
