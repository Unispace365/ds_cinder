#include "text.h"
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

//std::map<std::string, std::map<float, std::shared_ptr<ci::Font>>> mFontCache;
//
//std::map<std::string, std::map<float, ci::gl::TextureFontRef>>
//                                  mTextureFonts;

std::map<std::string, std::map<float, FontPtr>> mFontCache;
}

static const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("text sprite");
//static ci::gl::TextureFontRef get_font(const std::string& filename, const float size);

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
	, mDebugShowFrame(engine.getDebugSettings().getBool("text:show_frame", 0, false))
#ifdef TEXT_RENDER_ASYNC
	, mShared(new RenderTextShared())
	, mRenderClient(engine.getRenderTextService(), [this](RenderTextFinished& f) { this->onRenderFinished(f); })
#endif
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTextuer(true);
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

	// NOTE: This won't work here. The font will draw nothing. Don't know OpenGL well
	// enough to say, maybe there's some translation that's interfering with it? We should
	// figure it out, because this is the ONLY place this should happen, and it should be
	// removed from updateServer() and updateClient().
//  if (mNeedRedrawing) {
//    drawIntoFbo();
//  }

  //if (!mTextureFont) return;

#ifdef TEXT_RENDER_ASYNC
	if (mTestTexture) {

#if 0
//if (mTextString == L"FCC Approval") {
//if (mTextString == L"01.") {
if (mTextString == L"D. Encore") {
	int		fd = 32;
	static int	C400 = 0;
	std::cout << "w=" << mTestTexture.getWidth() << " h=" << mTestTexture.getHeight() << " C400=" << (++C400) << std::endl;
}
#endif
#if 0
if (mTextString == L"2010") {
	int		fd = 32;
	static int	C2010 = 0;
//	std::cout << "w=" << mTestTexture.getWidth() << " h=" << mTestTexture.getHeight() << " C2010=" << (++C2010) << std::endl;
	if (C2010 < 2) {
		ci::Surface8u	s(mTestTexture);
		ci::writeImage("C:\\Users\\erich\\Documents\\downstream\\wtf2010.png", s);
	}
}
if (mTextString == L"2012") {
	int		fd = 32;
	static int	C2012 = 0;
	std::cout << "w=" << mTestTexture.getWidth() << " h=" << mTestTexture.getHeight() << " C2012=" << (++C2012) << std::endl;
	if (C2012 < 2) {
		ci::Surface8u	s(mTestTexture);
		ci::writeImage("C:\\Users\\erich\\Documents\\downstream\\wtf2012.png", s);
	}
}
#endif
		mTestTexture.bind();
		if (getPerspective())
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTestTexture.getWidth()), static_cast<float>(mTestTexture.getHeight())));
		else
			ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mTestTexture.getHeight()), static_cast<float>(mTestTexture.getWidth()), 0.0f));
		mTestTexture.unbind();
	}
#endif

	if (mTexture) {
		mTexture.bind();
		if (getPerspective())
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight())));
		else
			ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mTexture.getHeight()), static_cast<float>(mTexture.getWidth()), 0.0f));
		mTexture.unbind();
	}

//std::cout << "Size: " << lines.size() << std::endl;
//for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
//  const TextLayout::Line&   line(*it);
//  mTextureFont->drawString(line.mText, ci::Vec2f(line.mPos.x+mBorder.x1, line.mPos.y+mBorder.y1), mDrawOptions);
//}
}

void Text::setSizeAll( float width, float height, float depth )
{
	if (mResizeToTextF) {
		DS_LOG_WARNING_M("Text::setSizeAll() while auto resize is on, statement does nothing", SPRITE_LOG);
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

#if 0
void Text::setAlignment( int alignment )
{
// This will be handled in a layout
}
#endif

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

void Text::debugPrint()
{
	makeLayout();
	std::cout << "Text lines=" << mLayout.getLines().size() << std::endl;
	mLayout.debugPrint();
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
		const float       fontSize = buf.read<float>();
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
			ci::Vec2f      size(mWidth-mBorder.x1-mBorder.x2, mHeight-mBorder.y1-mBorder.y2);
			// If we're auto resizing, then the area to perform the layout should be unlimited.
			if ((mResizeToTextF&RESIZE_W) != 0) {
				size.x = 100000;
				if (mResizeLimitWidth > 0) size.x = mResizeLimitWidth;
			}
			if ((mResizeToTextF&RESIZE_H) != 0) {
				size.y = 100000;
				if (mResizeLimitHeight > 0) size.y = mResizeLimitHeight;
			}
			TextLayout::Input    in(*this, mFont, size, mTextString);
			mLayoutFunc(in, mLayout);
		}
		markAsDirty(LAYOUT_DIRTY);

		if (mResizeToTextF) {
			calculateFrame(mResizeToTextF);
		}
	}
}

void Text::calculateFrame(const int flags)
{
  if (!mFont) return;

  //const float     descent = mFont->descender();
  const float     lineHeight = static_cast<float>(mFont->height());
  const float     height = mFont->pointSize();
  float           w = 0, h = 0;
  auto&           lines = mLayout.getLines();

  for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
    const TextLayout::Line&   line(*it);
    const ci::Vec2f           size = getSizeFromString(mFont, line.mText);//mFont->measureRaw(line.mText.c_str());
    const float               lineW = line.mPos.x + size.x;
          float               lineH = line.mPos.y + height;
    if (it + 1 != lines.end()) {
      lineH += lineHeight;
    } else {
      OGLFT::BBox box = mFont->measureRaw(line.mText.c_str());
      lineH += -box.y_min_;
    }
    if (lineW > w) w = lineW;
    if (lineH > h) h = lineH;
  }

  w = mBorder.x1 + w + mBorder.x2;
  h = mBorder.y1 + h + mBorder.y2;
  // Only change the dimensions specified by the flags
  if ((flags&RESIZE_W) == 0) w = mWidth;
  if ((flags&RESIZE_H) == 0) h = mHeight;
  inherited::setSizeAll(w, h, mDepth);
}

void Text::drawIntoFbo() {
	mTexture.reset();
	if (!mFont) return;

	auto& lines = mLayout.getLines();
	if (lines.empty()) return;

	if (mNeedRedrawing) {
		ds::gl::SaveCamera		save_camera;
#ifdef TEXT_RENDER_ASYNC
	int		code = 0;
	if (mTextString == L"2010") code = 2010;
	else if (mTextString == L"2012") code = 2012;
	else if (mTextString == L"FCC Approval") {
		code = 900;
	} else if (mTextString == L"D. Encore") {
		std::cout << "HERE" << std::endl;
		code = 400;
	}
std::cout << "START=" << ds::utf8_from_wstr(mTextString) << std::endl;
	mRenderClient.start(mEngine.getFonts().getFileNameFromName(mFontFileName), mFontSize, mShared, code);
#endif
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
			//format.setMagFilter(GL_NEAREST);
			mTexture = ci::gl::Texture(w, h, format);
		}

		ci::gl::enableAlphaBlending();
		applyBlendingMode(LIGHTEN);
		{
			ci::gl::SaveFramebufferBinding bindingSaver;
			std::unique_ptr<ds::ui::FboGeneral> fbo = std::move(mEngine.getFbo());
			fbo->attach(mTexture, true);
			fbo->begin();

//			glLoadIdentity();
			ci::Area fboBounds(0, 0, fbo->getWidth(), fbo->getHeight());
			ci::gl::setViewport(fboBounds);
			ci::CameraOrtho camera;
			camera.setOrtho(static_cast<float>(fboBounds.getX1()), static_cast<float>(fboBounds.getX2()), static_cast<float>(fboBounds.getY2()), static_cast<float>(fboBounds.getY1()), -1.0f, 1.0f);
			ci::gl::setMatrices(camera);

			ci::gl::clear(ColorA(1.0f, 1.0f, 1.0f, 0.0f));
			ci::gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));

			mFont->setForegroundColor( 1.0f, 1.0f, 1.0f, 0.99f );
			mFont->setBackgroundColor( 1.0f, 1.0f, 1.0f, 0.0f );
			//std::cout << "Size: " << lines.size() << std::endl;
			const float						height = mFont->pointSize();
			for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
				const TextLayout::Line&		line(*it);
				//mTextureFont->drawString(line.mText, ci::Vec2f(line.mPos.x+mBorder.x1, line.mPos.y+mBorder.y1), mDrawOptions);
				OGLFT::BBox box = mFont->measureRaw(line.mText);

				// Make sure textures are disabled, or else I can end up not
				// drawing and it can be very difficult to know why.
				ci::gl::BoolState	tex_2d_state(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_2D);
				mFont->draw(line.mPos.x+mBorder.x1 - box.x_min_, line.mPos.y+mBorder.y1 + height, line.mText);
			}

			fbo->end();
			fbo->detach();
			mEngine.giveBackFbo(std::move(fbo));
		}
	}
}

float Text::getLeading() const
{
  return 1.0f;
}

void Text::onRenderFinished(RenderTextFinished& finished)
{
#ifdef TEXT_RENDER_ASYNC
std::cout << "onFinished code=" << finished.mCode << " text='" << ds::utf8_from_wstr(mTextString) << "'" << std::endl;
if (mTextString == L"2010") {
	std::cout << "Got 2010" << std::endl;
}
if (mTextString == L"2012") {
	std::cout << "Got 2012" << std::endl;
}
	mTestTexture = finished.mTexture;
#endif
}

} // namespace ui
} // namespace ds

//static ci::gl::TextureFontRef get_font(const std::string& filename, const float size)
//{
//    auto found = mTextureFonts.find(filename);
//    if ( found != mTextureFonts.end() )
//    {
//        auto found2 = found->second.find(size);
//        if ( found2 != found->second.end() )
//            return found2->second;
//    }
//
//    ci::DataSourcePathRef src = ci::DataSourcePath::create(filename);
//    ci::Font              f(src, size);
//    if (!f) {
//      DS_LOG_ERROR_M("Text::get_font() failed to load font (" << filename << ")", SPRITE_LOG);
//      DS_ASSERT(false);
//      return nullptr;
//    }
//    ci::gl::TextureFontRef  tf = ci::gl::TextureFont::create(f);
//    if (!tf) {
//      DS_LOG_ERROR_M("Text::get_font() failed to create font texture (" << filename << ")", SPRITE_LOG);
//      DS_ASSERT(false);
//      return nullptr;
//    }
//    mTextureFonts[filename][size] = tf;
//    return tf;
//}

/**
 * miscellaneous
 */
static FontPtr get_font(const std::string& filename, const float size)
{
  auto found = mFontCache.find(filename);
  if ( found != mFontCache.end() )
  {
    auto found2 = found->second.find(size);
    if ( found2 != found->second.end() )
      return found2->second;
  }

  FontPtr font = FontPtr(new OGLFT::Translucent(filename.c_str(), size));

  if (!font->isValid())
    throw std::runtime_error("Font: " + filename + " was unable to load.");

  font->setCompileMode(OGLFT::Face::COMPILE);

  mFontCache[filename][size] = font;
  return font;
}
