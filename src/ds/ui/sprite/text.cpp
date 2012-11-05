#include "text.h"
#include <map>
#include <cinder/Vector.h>
#include <cinder/app/App.h>
#include <cinder/Buffer.h>
#include <cinder/DataSource.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/config/settings.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "cinder/Camera.h"
#include <stdexcept>
#include "ds/util/string_util.h"

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
char                BLOB_TYPE         = 0;

const DirtyState&   FONT_DIRTY 		    = INTERNAL_A_DIRTY;
const DirtyState&   TEXT_DIRTY 		    = INTERNAL_B_DIRTY;
const DirtyState&   LAYOUT_DIRTY 		  = INTERNAL_C_DIRTY;
const DirtyState&   BORDER_DIRTY 		  = INTERNAL_D_DIRTY;

const char          FONT_ATT          = 80;
const char          TEXT_ATT          = 81;
const char          LAYOUT_ATT        = 82;
const char          BORDER_ATT        = 83;

const int           RESIZE_W          = (1<<0);
const int           RESIZE_H          = (1<<1);
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
  int           newF = 0;
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

Text& Text::setFont(const std::string& filename, const float fontSize)
{
  mFont = get_font(filename, fontSize);
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
  if (mNeedRedrawing)
    drawIntoFbo();
}

void Text::drawLocalClient()
{
  if (mDebugShowFrame) {
    mSpriteShader.getShader().unbind();
    glPushAttrib(GL_COLOR);
    gl::color(0.25f, 0, 0, 0.5f);
    ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight));
    glPopAttrib();
    mSpriteShader.getShader().bind();
  }

  //if (!mTextureFont) return;

  //auto& lines = mLayout.getLines();
  //if (lines.empty()) return;

//  gl::enableAlphaBlending();
//  applyBlendingMode(NORMAL);

  //if (mFbo) {
  //  mFbo.getTexture().bind();
  //  ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mFbo.getHeight()), static_cast<float>(mFbo.getWidth()), 0.0f));
  //  mFbo.getTexture().unbind();
  //}
  if (mTexture) {
    mTexture.bind();
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
  std::wstring temp = ds::wstr_from_utf8(text);

  if (mTextString == temp) return *this;

  mTextString = temp;
  mNeedsLayout = true;
  mNeedRedrawing = true;
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

void Text::setAlignment( int alignment )
{
  // This will be handled in a layout
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
  return mFont->height();
}

void Text::debugPrint()
{
  makeLayout();
  std::cout << "Text lines=" << mLayout.getLines().size() << std::endl;
  mLayout.debugPrint();
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
  const float     lineHeight = mFont->height();
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

void Text::drawIntoFbo()
{
  mTexture.reset();

  if (!mFont) return;

  auto& lines = mLayout.getLines();
  if (lines.empty()) return;

  if (mNeedRedrawing) {
    mNeedRedrawing = false;
    const int w = (int)ceilf(getWidth());
    const int h = (int)ceilf(getHeight());

    if (w == 0 || h == 0)
      return;

    if (!mTexture || mTexture.getWidth() < w || mTexture.getHeight() < h) {
      ci::gl::Texture::Format format;
      format.setTarget(GL_TEXTURE_2D);
      //format.setMagFilter(GL_NEAREST);
      mTexture = ci::gl::Texture(w, h);
    }

    gl::enableAlphaBlending();
    applyBlendingMode(LIGHTEN);
    {
      gl::SaveFramebufferBinding bindingSaver;

      std::unique_ptr<ds::ui::FboGeneral> fbo = std::move(mEngine.getFbo());
      fbo->attach(mTexture, true);
      fbo->begin();

      ci::Area fboBounds(0, 0, fbo->getWidth(), fbo->getHeight());
      ci::gl::setViewport(fboBounds);
      ci::CameraOrtho camera;
      camera.setOrtho(static_cast<float>(fboBounds.getX1()), static_cast<float>(fboBounds.getX2()), static_cast<float>(fboBounds.getY2()), static_cast<float>(fboBounds.getY1()), -1.0f, 1.0f);

      ci::gl::setMatrices(camera);


      ci::gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
      ci::gl::color(Color(1.0f, 1.0f, 1.0f));

      mFont->setForegroundColor( 1.0f, 1.0f, 1.0f );
      mFont->setBackgroundColor( 0.0f, 0.0f, 0.0f, 0.0f );
      //std::cout << "Size: " << lines.size() << std::endl;
      float height = mFont->pointSize();
      for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
        const TextLayout::Line&   line(*it);
        //mTextureFont->drawString(line.mText, ci::Vec2f(line.mPos.x+mBorder.x1, line.mPos.y+mBorder.y1), mDrawOptions);
        OGLFT::BBox box = mFont->measureRaw(line.mText);
        mFont->draw(line.mPos.x+mBorder.x1 - box.x_min_, line.mPos.y+mBorder.y1 + height, line.mText);
      }

      fbo->end();
      fbo->detach();
      mEngine.giveBackFbo(std::move(fbo));
    }
    mEngine.setCamera();
  }
}

float Text::getLeading() const
{
  return 1.0f;
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

  font->setCompileMode(OGLFT::Face::IMMEDIATE);

  mFontCache[filename][size] = font;
  return font;
}
