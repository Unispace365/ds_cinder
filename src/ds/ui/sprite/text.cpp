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

using namespace ci;

namespace {

std::map<std::string, std::map<float, std::shared_ptr<ci::Font>>> mFontCache;

std::map<std::string, std::map<float, ci::gl::TextureFontRef>>
                                  mTextureFonts;
}

static const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("text sprite");
static ci::gl::TextureFontRef get_font(const std::string& filename, const float size);

namespace ds {
namespace ui {

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
    , mLayoutFunc(TextLayout::SINGLE_LINE())
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

Text& Text::setFont(const std::string& filename, const float fontSize)
{
  mTextureFont = get_font(filename, fontSize);
  mFontSize = fontSize;
  markAsDirty(FONT_DIRTY);
  mNeedsLayout = true;
  return *this;
}

void Text::updateServer(const UpdateParams& p)
{
  inherited::updateServer(p);

  makeLayout();
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

  if (!mTextureFont) return;

  auto& lines = mLayout.getLines();
  if (lines.empty()) return;

//  gl::enableAlphaBlending();
//  applyBlendingMode(NORMAL);

  for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
    const TextLayout::Line&   line(*it);
    mTextureFont->drawString(line.mText, ci::Vec2f(line.mPos.x+mBorder.x1, line.mPos.y+mBorder.y1), mDrawOptions);
  }

#if 0
    if ( mBoxChanged )
    {
        mTexture = ci::gl::Texture( mTextBox.render() );
        mBoxChanged = false;
    }
    if ( mTexture )
        ci::gl::draw(mTexture);
#endif
}

void Text::setSizeAll( float width, float height, float depth )
{
  if (mResizeToTextF) {
    DS_LOG_WARNING_M("Text::setSizeAll() while auto resize is on, statement does nothing", SPRITE_LOG);
    return;
  }

  inherited::setSizeAll(width, height, depth);
  mNeedsLayout = true;
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
  if (mTextString == text) return *this;

  mTextString = text;
  mNeedsLayout = true;
  return *this;
}

std::string Text::getText() const
{
    return mTextString;
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
  return *this;
}

float Text::getFontAscent() const
{
  if (!mTextureFont) return 0;
  return mTextureFont->getAscent();
}

float Text::getFontDescent() const
{
  if (!mTextureFont) return 0;
  return mTextureFont->getDescent();
}

float Text::getFontHeight() const
{
  if (!mTextureFont) return 0;
  return mTextureFont->getAscent() + mTextureFont->getDescent();
}

float Text::getFontLeading() const
{
  if (!mTextureFont) return 0;
  return mTextureFont->getFont().getLeading();
}

void Text::makeLayout()
{
  if (mNeedsLayout) {
    mNeedsLayout = false;
    mLayout.clear();
    if (mLayoutFunc && mTextureFont) {
      ci::Vec2f      size(mWidth-mBorder.x1-mBorder.x2, mHeight-mBorder.y1-mBorder.y2);
      // If we're auto resizing, then the area to perform the layout should be unlimited.
      if ((mResizeToTextF&RESIZE_W) != 0) size.x = 100000;
      if ((mResizeToTextF&RESIZE_H) != 0) size.y = 100000;
      TextLayout::Input    in(*this, mTextureFont, mDrawOptions, size, mTextString);
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
  if (!mTextureFont) return;

  const float     descent = mTextureFont->getDescent();
  float           w = 0, h = 0;
  auto&           lines = mLayout.getLines();

  for (auto it=lines.begin(), end=lines.end(); it!=end; ++it) {
    const TextLayout::Line&   line(*it);
    const ci::Vec2f           size = mTextureFont->measureString(line.mText, mDrawOptions);
    const float               lineW = line.mPos.x + size.x,
                              lineH = line.mPos.y + descent;
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

} // namespace ui
} // namespace ds

static ci::gl::TextureFontRef get_font(const std::string& filename, const float size)
{
    auto found = mTextureFonts.find(filename);
    if ( found != mTextureFonts.end() )
    {
        auto found2 = found->second.find(size);
        if ( found2 != found->second.end() )
            return found2->second;
    }

    ci::DataSourcePathRef src = ci::DataSourcePath::create(filename);
    ci::Font              f(src, size);
    if (!f) {
      DS_LOG_ERROR_M("Text::get_font() failed to load font (" << filename << ")", SPRITE_LOG);
      DS_ASSERT(false);
      return nullptr;
    }
    ci::gl::TextureFontRef  tf = ci::gl::TextureFont::create(f);
    if (!tf) {
      DS_LOG_ERROR_M("Text::get_font() failed to create font texture (" << filename << ")", SPRITE_LOG);
      DS_ASSERT(false);
      return nullptr;
    }
    mTextureFonts[filename][size] = tf;
    return tf;
}
