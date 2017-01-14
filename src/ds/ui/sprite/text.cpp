#include "stdafx.h"

#include "text.h"

namespace ds {
namespace ui {
void clearFontCache(){}
}
}
#if 0

#include <map>
#include <cinder/Vector.h>
#include <cinder/app/App.h>
#include <cinder/Buffer.h>
#include <cinder/DataSource.h>
#include "ds/data/font_list.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/cfg/settings.h"
#include "ds/data/data_buffer.h"
#include <ds/gl/save_camera.h>
#include "ds/ui/sprite/sprite_engine.h"
#include "cinder/Camera.h"
#include <stdexcept>
#include "ds/util/string_util.h"

#include <cinder/Surface.h>
#include <cinder/ImageIo.h>

using namespace ci;

namespace {
std::map<std::string, std::map<float, FontPtr>> mFontCache;
}

static const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("text sprite");

FontPtr get_font(const std::string& filename, const float size);

namespace ds {
namespace ui {


void clearFontCache()
{
	mFontCache.clear();
}

namespace {
char				BLOB_TYPE			= 0;

const DirtyState&	FONT_DIRTY			= INTERNAL_A_DIRTY;
const DirtyState&	TEXT_DIRTY			= INTERNAL_B_DIRTY;
const DirtyState&	LAYOUT_DIRTY		= INTERNAL_C_DIRTY;
const DirtyState&	BORDER_DIRTY		= INTERNAL_D_DIRTY;

const char			FONTNAME_ATT		= 80;
const char			FONTID_ATT			= 81;
const char			TEXT_ATT			= 82;
const char			LAYOUT_ATT			= 83;
const char			BORDER_ATT			= 84;

const int			RESIZE_W			= (1<<0);
const int			RESIZE_H			= (1<<1);
}

void Text::installAsServer(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Text::installAsClient(ds::BlobRegistry& registry)
{
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Text>(r);});
}

Text::Text(SpriteEngine& engine)
	: inherited(engine)
	, mFontSize(0)
	, mBorder(0, 0, 0, 0)
	, mResizeToTextF(RESIZE_W|RESIZE_H)
	, mNeedsLayout(false)
	, mNeedRedrawing(false)
	, mLayoutFunc(TextLayout::SINGLE_LINE())
	, mResizeLimitWidth(0)
	, mResizeLimitHeight(0)
	, mHasSplitLine(false)
	, mDebugShowFrame(engine.getDebugSettings().getBool("text:show_frame", 0, false))
	, mGenerateIndex(false)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTexture(true);
}

Text::~Text()
{
}

Text& Text::setResizeToText(const bool on)
{
	return setResizeToText(on, on);
}

Text& Text::setResizeToText(const bool width, const bool height)
{
	int			newF = 0;
	if (width) newF |= RESIZE_W;
	if (height) newF |= RESIZE_H;
	if (mResizeToTextF == newF) return *this;

	mResizeToTextF = newF;
	mNeedsLayout = true;
	mNeedRedrawing = true;
	return *this;
}

bool Text::autoResizeWidth() const
{
	return (mResizeToTextF&RESIZE_W) != 0;
}

bool Text::autoResizeHeight() const
{
	return (mResizeToTextF&RESIZE_H) != 0;
}

bool Text::autoResize() const
{
	return autoResizeWidth() && autoResizeHeight();
}

float Text::getResizeLimitWidth() const
{
	return mResizeLimitWidth;
}

float Text::getResizeLimitHeight() const
{
	return mResizeLimitHeight;
}

Text& Text::setResizeLimit(const float width, const float height)
{
	if (width == mResizeLimitWidth && height == mResizeLimitHeight) return *this;

	mResizeLimitWidth = width;
	mResizeLimitHeight = height;
	mNeedsLayout = true;
	mNeedRedrawing = true;
	return *this;
}

Text& Text::setFont(const std::string& name, const float fontSize)
{
	mFont = get_font(mEngine.getFonts().getFileNameFromName(name), fontSize);
	mFontFileName = name;
	mFontSize = fontSize;
	markAsDirty(FONT_DIRTY);
	mNeedsLayout = true;
	mNeedRedrawing = true;
	return *this;
}

float Text::getFontSize()
{
	return mFontSize;
}

void Text::setFontSize(float fontSize)
{
	mFont = get_font(mEngine.getFonts().getFileNameFromName(mFontFileName), fontSize);
	mFontSize = fontSize;
	markAsDirty(FONT_DIRTY);
	mNeedsLayout = true;
	mNeedRedrawing = true;
}

void Text::updateServer(const UpdateParams& p)
{
	inherited::updateServer(p);

	makeLayout();
	// NOTE: Needs to be here. If this is called in drawLocalClient(),
	// then the font won't render.
	// ALSO, this really shouldn't be here. Need to work out a way for
	// the texture to be created only in client or clientserver mode;
	// this also drags in server mode.
	if (mNeedRedrawing) {
		drawIntoFbo();
	}
}

void Text::updateClient(const UpdateParams& p)
{
	inherited::updateClient(p);

	// NOTE: Needs to be here. If this is called in drawLocalClient(),
	// then the font won't render.
	if (mNeedRedrawing) {
		drawIntoFbo();
	}
}

void Text::drawLocalClient()
{
	if (mDebugShowFrame) {
		ci::gl::SaveColorState		scs;
		mSpriteShader.getShader().unbind();
		glPushAttrib(GL_COLOR);
		ci::gl::color(0.25f, 0, 0, 0.5f);
		ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight));
		glPopAttrib();
		mSpriteShader.getShader().bind();
	}

	if (mTexture) {
		mTexture.bind();
		if (getPerspective())
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight())));
		else
			ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mTexture.getHeight()), static_cast<float>(mTexture.getWidth()), 0.0f));
		mTexture.unbind();
	}
}

void Text::setSizeAll( float width, float height, float depth )
{
	if (mResizeToTextF) {
		// This warning is kinda unneccessary, really.
		//DS_LOG_WARNING_M("Text::setSizeAll() while auto resize is on, statement does nothing", SPRITE_LOG);
		return;
	}

	inherited::setSizeAll(width, height, depth);
	mNeedsLayout = true;
	mNeedRedrawing = true;
}

float Text::getWidth() const
{
	if (mResizeToTextF && mNeedsLayout) {
		(const_cast<Text*>(this))->makeLayout();
	}
	return mWidth;
}

float Text::getHeight() const
{
	if (mResizeToTextF && mNeedsLayout) {
		(const_cast<Text*>(this))->makeLayout();
	}
	return mHeight;
}

Text& Text::setText( const std::string &text )
{
	setText(ds::wstr_from_utf8(text));
	return *this;
}

Text& Text::setText( const std::wstring &text )
{
	if (mTextString == text) return *this;

	mTextString = text;
	mNeedsLayout = true;
	mNeedRedrawing = true;
	return *this;
}

std::wstring Text::getText() const
{
	return mTextString;
}

std::string Text::getTextAsString() const
{
	return ds::utf8_from_wstr(mTextString);
}


bool Text::hasText() const
{
	return !mTextString.empty();
}

Text& Text::setBorder(const ci::Rectf& r)
{
	if (mBorder.x1 == r.x1 && mBorder.y1 == r.y1 && mBorder.x2 == r.x2 && mBorder.y2 == r.y2) return *this;

	mBorder = r;
	markAsDirty(BORDER_DIRTY);
	return *this;
}

Text& Text::setLayoutFunction(const TextLayout::MAKE_FUNC& f)
{
	mLayoutFunc = f;
	mNeedsLayout = true;
	mNeedRedrawing = true;
	return *this;
}

float Text::getFontAscent() const
{
	if (!mFont) return 0;
	return getFontAscender(mFont) * mFont->pointSize();
}

float Text::getFontDescent() const
{
	if (!mFont) return 0;
	return getFontDescender(mFont) * mFont->pointSize();
}

float Text::getFontHeight() const
{
	if (!mFont) return 0;
	return getFontAscent() + getFontDescent();
}

float Text::getFontLeading() const
{
	//////////////////////////////////////////////////////////////////////////
	// Come back to this.
	//////////////////////////////////////////////////////////////////////////

	if (!mFont) return 0;
	return static_cast<float>(mFont->height());
}

float Text::getPixelFontHeight() const
{
	if (!mFont) return 0;
	float y = ceilf((1.0f - getFontAscender(mFont)) * mFont->pointSize());
	auto p = mFont->pointSize();
	return p + y - (getFontDescender(mFont) * p);
}

float Text::getFontFullHeight() const
{
	return ds::ui::getFontHeight(mFont, 0.0f);
}


const std::string Text::getFontFileName()const{
	return mFontFileName;
}

void Text::debugPrint()
{
	makeLayout();
	std::cout << "Text lines=" << mLayout.getLines().size() << std::endl;
	mLayout.debugPrint();
}


ci::vec2 Text::getPositionForCharacterIndex(const int characterIndex){
	if(mTextString.empty() || characterIndex < 0 || characterIndex > mTextString.size()) return ci::vec2::zero();
	if(!mGenerateIndex){
		mGenerateIndex = true;
		mNeedsLayout = true;
	}
	makeLayout();
	if(mLayout.getLines().empty()) return ci::vec2::zero();
	
	const std::vector<TextLayout::Line>& lines = mLayout.getLines();

	// look through all the lines for the character matching this index
	for(auto it = lines.begin(); it < lines.end(); ++it){
		const TextLayout::Line& line = (*it);
		auto fit = line.mIndexPositions.find(characterIndex);
		if(fit == line.mIndexPositions.end()) continue;
		return ci::vec2(line.mPos.x + fit->second, line.mPos.y);
	}

	// Assume we're past the end of the text sprite
	const TextLayout::Line& line = lines.back();
	if(line.mIndexPositions.empty()) return ci::vec2::zero();
	auto fit = line.mIndexPositions.rbegin();
	return ci::vec2(line.mPos.x + fit->second, line.mPos.y);
}

namespace {

int parseLine(const TextLayout::Line& line, const ci::vec2& possy){
	if(line.mIndexPositions.empty()){
		// This is technically an error condition.
		// But we're not gonna log anything, cause there's not a lot of consequences for it
		return 0;
	}

	int firstCharacter = line.mIndexPositions.begin()->first;
	// we're before the first character on the line, so return the index on the first character
	if(possy.x <= line.mPos.x){
		return firstCharacter;
	}

	// track the previous character cause we only know when we got there when we're past it
	int previousIndex = firstCharacter;
	for(auto lit = line.mIndexPositions.begin(); lit != line.mIndexPositions.end(); ++lit){
		if(lit->second > possy.x){
			return previousIndex;
		}

		previousIndex = lit->first;
	}

	// we got past the end of the line, so return the last character
	return previousIndex;
}

}

int Text::getCharacterIndexForPosition(const ci::vec2& possy){
	if(mTextString.empty()) return 0;
	if(!mGenerateIndex){
		mGenerateIndex = true;
		mNeedsLayout = true;
	}
	makeLayout();
	if(mLayout.getLines().empty()) return 0;

	const std::vector<TextLayout::Line>& lines = mLayout.getLines();

	// look through all the lines for the line that contains this point
	for(auto it = lines.begin(); it < lines.end(); ++it){
		const TextLayout::Line& line = (*it);
		if(possy.y < line.mPos.y + line.mFontBox.getHeight()){
			return parseLine(line, possy);
		}
	}

	// since we didn't find anything in the lines, we assume we're looking at the last line
	return parseLine(lines.back(), possy);
}

void Text::writeAttributesTo(ds::DataBuffer& buf)
{
	inherited::writeAttributesTo(buf);

	if (mDirty.has(FONT_DIRTY)) {
		// Try to find an efficient token, if the app has the FontList setup.
		const int fontId = mEngine.getFonts().getIdFromName(mFontFileName);
		if (fontId > 0) {
			buf.add(FONTID_ATT);
			buf.add(fontId);
		} else {
			buf.add(FONTNAME_ATT);
			buf.add(mFontFileName);
		}
		buf.add(mFontSize);
	}
	if (mDirty.has(LAYOUT_DIRTY)) {
		makeLayout();
		buf.add(LAYOUT_ATT);
		mLayout.writeTo(buf);
	}
	if (mDirty.has(BORDER_DIRTY)) {
		buf.add(BORDER_ATT);
		buf.add(mBorder.x1);
		buf.add(mBorder.y1);
		buf.add(mBorder.x2);
		buf.add(mBorder.y2);
	}
}

void Text::readAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
	if (attributeId == FONTNAME_ATT) {
		const std::string filename = buf.read<std::string>();
		const float		fontSize = buf.read<float>();
		if (!filename.empty()) {
			setFont(filename, fontSize);
			mNeedRedrawing = true;
		}
	} else if (attributeId == FONTID_ATT) {
		const std::string filename = mEngine.getFonts().getFileNameFromId(buf.read<int>());
		const float fontSize = buf.read<float>();
		if (!filename.empty()) {
			setFont(filename, fontSize);
			mNeedRedrawing = true;
		}
	} else if (attributeId == LAYOUT_ATT) {
		mLayout.readFrom(buf);
		mNeedRedrawing = true;
	} else if (attributeId == BORDER_ATT) {
		float x1 = mBorder.x1, y1 = mBorder.y1, x2 = mBorder.x2, y2 = mBorder.y2;
		if (buf.canRead<float>()) x1 = buf.read<float>();
		if (buf.canRead<float>()) y1 = buf.read<float>();
		if (buf.canRead<float>()) x2 = buf.read<float>();
		if (buf.canRead<float>()) y2 = buf.read<float>();
		mBorder = ci::Rectf(x1, y1, x2, y2);
		mNeedRedrawing = true;
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
	// This stuff should always been off -- it's server-side only, and
	// will prevent the drawing. Which suggests the division between
	// server and client functionality isn't defined well enough.
	mResizeToTextF = 0;
	mNeedsLayout = false;
}

void Text::makeLayout()
{
	if (mNeedsLayout) {
		mNeedsLayout = false;
		mLayout.clear();
		if (mLayoutFunc && mFont) {
			ci::vec2	size(mWidth-mBorder.x1-mBorder.x2, mHeight-mBorder.y1-mBorder.y2);
			// If we're auto resizing, then the area to perform the layout should be unlimited.
			if ((mResizeToTextF&RESIZE_W) != 0) {
				size.x = 100000;
				if (mResizeLimitWidth > 0) size.x = mResizeLimitWidth;
			}
			if ((mResizeToTextF&RESIZE_H) != 0) {
				size.y = 100000;
				if (mResizeLimitHeight > 0) size.y = mResizeLimitHeight;
			}
			TextLayout::Input	in(*this, mFont, size, mTextString);
			in.mGenerateIndex = mGenerateIndex;
			mLayoutFunc(in, mLayout);
			mHasSplitLine = in.mLineWasSplit;
		}
		markAsDirty(LAYOUT_DIRTY);

		if (mResizeToTextF) {
			calculateFrame(mResizeToTextF);
		}
	}
}

void Text::calculateFrame(const int flags)
{
	if(!mFont || !mFont->isValid()) return;

	const float		lineHeight = static_cast<float>(mFont->height());
	const float		height = mFont->pointSize();
	float			w = 0, h = 0;
	auto&			lines = mLayout.getLines();

	for(auto it = lines.begin(), end = lines.end(); it != end; ++it) {
		const TextLayout::Line&		line(*it);
		const float					lineW = line.mPos.x + line.mFontBox.getWidth();
		float						lineH = line.mPos.y + height;
		if(it + 1 != lines.end()) {
			lineH += lineHeight;
		} else {
			lineH += - line.mFontBox.getY1();
		}
		if(lineW > w) w = lineW;
		if(lineH > h) h = lineH;
	}

	w = mBorder.x1 + w + mBorder.x2;
	h = mBorder.y1 + h + mBorder.y2;

	// Only change the dimensions specified by the flags
	if((flags&RESIZE_W) == 0) w = mWidth;
	if((flags&RESIZE_H) == 0) h = mHeight;
	inherited::setSizeAll(w, h, mDepth);
}

void Text::drawIntoFbo() {
	mTexture.reset();
	if(!mFont || !mFont->isValid()) return;

	auto& lines = mLayout.getLines();
	if (lines.empty()) return;

	if(mNeedRedrawing) {
		ds::gl::SaveCamera		save_camera;

		mNeedRedrawing = false;

		// XXX I noticed some fonts were getting the bottom right row of pixels
		// chopped off, so I did this, although realistically, it probably means
		// the actual w/h of the sprite should be increased, not just the texture.
		const int w = (int)ceilf(getWidth()) + 1;
		const int h = (int)ceilf(getHeight()) + 1;

		if (w < 1 || h < 1) {
			return;
		}

		if (!mTexture || mTexture.getWidth() < w || mTexture.getHeight() < h) {
			ci::gl::Texture::Format format;
			format.setTarget(GL_TEXTURE_2D);
			format.setMagFilter(GL_LINEAR);
			format.setMinFilter(GL_LINEAR);
			mTexture = ci::gl::Texture(w, h, format);
		}

		ci::gl::enableAlphaBlending();
		applyBlendingMode(LIGHTEN);
		{
			ci::gl::SaveFramebufferBinding bindingSaver;
			std::unique_ptr<ds::ui::FboGeneral> fbo = std::move(mEngine.getFbo());
			if(!fbo){
				DS_LOG_WARNING("Fbo doesn't exist when drawing into it in a text sprite!");
				return;
			}
			if(!fbo->attach(mTexture, true)){
				DS_LOG_WARNING("Error attaching a texture to the fbo!");
				fbo->detach();
				mEngine.giveBackFbo(std::move(fbo));
				return;
			}
			fbo->begin();

			ci::Area fboBounds(0, 0, fbo->getWidth(), fbo->getHeight());
			ci::gl::setViewport(fboBounds);
			ci::CameraOrtho camera;
			camera.setOrtho(static_cast<float>(fboBounds.getX1()), static_cast<float>(fboBounds.getX2()), static_cast<float>(fboBounds.getY2()), static_cast<float>(fboBounds.getY1()), -1.0f, 1.0f);
			ci::gl::setMatrices(camera);

			ci::gl::clear(ColorA(1.0f, 1.0f, 1.0f, 0.0f));
			ci::gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));

			mFont->setForegroundColor( 1.0f, 1.0f, 1.0f, 1.0f );
			mFont->setBackgroundColor( 1.0f, 1.0f, 1.0f, 0.0f );

			const float						height = mFont->pointSize();
			for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
				const TextLayout::Line&		line(*it);

				// Make sure textures are disabled, or else I can end up not
				// drawing and it can be very difficult to know why.
				ci::gl::BoolState	tex_2d_state(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_2D);

				float xPos = line.mPos.x + mBorder.x1 - line.mFontBox.getX1();
				float yPos = line.mPos.y + mBorder.y1 + height;

				// If x or y are negative, nothing will draw.
				// This is technically an error condition, but I suspect it's caused by float imprecision.
				// Better to draw a pixel or two off then to not draw at all.
				if(xPos < 0.0f) xPos = 0.0f;
				if(yPos < 0.0f) yPos = 0.0f;

				mFont->draw(xPos, yPos, line.mText);
			}

			fbo->end();
			fbo->detach();
			mEngine.giveBackFbo(std::move(fbo));
		}
	}
}

float Text::getLeading() const {
	return 1.0f;
}


} // namespace ui
} // namespace ds

/**
 * miscellaneous
 */
static FontPtr get_font(const std::string& filename, const float size)
{
	auto found = mFontCache.find(filename);
	if(found != mFontCache.end())
	{
		auto found2 = found->second.find(size);
		if(found2 != found->second.end())
			return found2->second;
	}

	FontPtr font = FontPtr(new OGLFT::Translucent(filename.c_str(), size));

	if(!font->isValid()){
		DS_LOG_WARNING("Font: " + filename + " was unable to load.");
		return font;
	}

	font->setCompileMode(OGLFT::Face::COMPILE);

	mFontCache[filename][size] = font;
	return font;
}

#endif