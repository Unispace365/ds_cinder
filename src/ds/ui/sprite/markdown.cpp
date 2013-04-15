#include "markdown.h"
#ifdef AWESOMIUM
#include "sprite_engine.h"
#include "Awesomium/WebCore.h"
#include "Awesomium/WebSession.h"
#include "CinderAwesomium.h"
#include "ds/debug/logger.h"
#include "ds/math/math_func.h"
#include "ds/util/string_util.h"
#include "cinder/gl/gl.h"

namespace ds {
namespace ui {

Markdown::Markdown( SpriteEngine &engine, float width /*= 0.0f*/, float height /*= 0.0f*/ )
  : Sprite(engine, ds::math::max(width, 10.0f), ds::math::max(height, 10.0f))
  , mWebViewPtr(nullptr)
  , mWebSessionPtr(nullptr)
  , mScrollHeight(getHeight())
  , mScrollTop(0)
  , mScrollWidth(getWidth())
  , mScrollLeft(0)
  , mResizeToLength(false)
{
  setTransparent(false);
  setColor(1.0f, 1.0f, 1.0f);
  setUseShaderTextuer(true);

  Awesomium::WebCore *webCorePtr = mEngine.getWebCore();

  if (!webCorePtr) {
    std::stringstream ss;
    ss << "Exception: " << "Web has not been initialized." << " | File: " << __FILE__ << " Line: " << __LINE__;

    DS_LOG_ERROR(ss.str());
    throw std::runtime_error(ss.str().c_str());
  }
  //Awesomium::WebSession *webSessionPtr = mEngine.getWebSession();


  Awesomium::WebPreferences preferences;
  preferences.user_stylesheet.Append(Awesomium::WSLit("::-webkit-scrollbar { visibility: hidden; }"));
  preferences.user_stylesheet.Append(Awesomium::WSLit("html { background-color: transparent; }"));
  preferences.user_stylesheet.Append(Awesomium::WSLit("html { opacity: 0.99; }"));

  std::stringstream ss;

  mWebSessionPtr = webCorePtr->CreateWebSession(Awesomium::WSLit(""), preferences);
  // create a webview
  mWebViewPtr = webCorePtr->CreateWebView(static_cast<int>(getWidth()), static_cast<int>(getHeight()), mWebSessionPtr);
  mWebViewPtr->SetTransparent(true);
}

Markdown::~Markdown()
{
  if (mWebViewPtr)
    mWebViewPtr->Destroy();
  if (mWebSessionPtr)
    mWebSessionPtr->Release();
}

void Markdown::setSizeAll( float width, float height, float depth )
{
  if (ds::math::isEqual(width, getWidth()) &&
    ds::math::isEqual(height, getHeight()) &&
    ds::math::isEqual(depth, getDepth()))
    return;

  Sprite::setSizeAll(width, height, depth);
  mWebViewPtr->Resize(static_cast<int>(width), static_cast<int>(height));
}

void Markdown::updateServer( const ds::UpdateParams &updateParams )
{
  Sprite::updateServer(updateParams);

  // create or update our OpenGL Texture from the webview
  if (!mWebViewPtr->IsLoading() && ph::awesomium::isDirty( mWebViewPtr )) {
    Awesomium::JSArray jsarray = mWebViewPtr->ExecuteJavascriptWithResult(Awesomium::WSLit("[document.body.scrollHeight, document.body.scrollTop, document.body.scrollWidth, document.body.scrollLeft]"),
      Awesomium::WSLit("")).ToArray();
    int size = jsarray.size();
    if (size == 4) {
      mScrollHeight = jsarray.At(0).ToInteger();
      mScrollTop = jsarray.At(1).ToInteger();
      mScrollWidth = jsarray.At(2).ToInteger();
      mScrollLeft = jsarray.At(3).ToInteger();
    }

    if (mResizeToLength && mScrollHeight != static_cast<int>(getHeight())) {
      setSize(getWidth(), static_cast<float>(mScrollHeight));
    }

    if (mPageUpdatedCallback)
      mPageUpdatedCallback();

    try {
      // set texture filter to NEAREST if you don't intend to transform (scale, rotate) it
      ci::gl::Texture::Format fmt;
      fmt.setMagFilter( GL_NEAREST );
      fmt.setMinFilter( GL_NEAREST );

      // get the texture using a handy conversion function
      mWebTexture = ph::awesomium::toTexture( mWebViewPtr, fmt );
    } catch( const std::exception &e ) {
      DS_LOG_ERROR("Exception: " << e.what() << " | File: " << __FILE__ << " Line: " << __LINE__);
    }
  }
}

void Markdown::drawLocalClient()
{
  if (mWebTexture) {
    //ci::gl::color(ci::Color::white());
    ci::gl::enable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    ci::gl::draw(mWebTexture);
  }
}

void Markdown::loadUrl( const std::wstring &url )
{
  mWebViewPtr->LoadURL(Awesomium::WebURL(Awesomium::WSLit(ds::utf8_from_wstr(url).c_str())));
  mWebViewPtr->Focus();
}

void Markdown::loadUrl( const std::string &url )
{
  loadUrl(ds::wstr_from_utf8(url));
}

void Markdown::setResizeToLength( bool flag )
{
  mResizeToLength = true;
  if (mScrollHeight != static_cast<int>(getHeight())) {
    setSize(getWidth(), static_cast<float>(mScrollHeight));
  }
}

void Markdown::setText( const std::wstring &text )
{
  mText = text;
  std::stringstream ss;
  ss << "SetText(\"" << ds::utf8_from_wstr(mText) << "\");";

  Awesomium::JSObject window = mWebViewPtr->ExecuteJavascriptWithResult( Awesomium::WSLit("window"), Awesomium::WSLit("") ).ToObject();
  Awesomium::JSArray jsarray;
  jsarray.Push(Awesomium::WSLit(ds::utf8_from_wstr(text).c_str()));
  window.InvokeAsync( Awesomium::WSLit("SetText"), jsarray );
}

void Markdown::setText( const std::string &text )
{
  setText(ds::wstr_from_utf8(text));
}

const std::wstring &Markdown::getText() const
{
  return mText;
}

int Markdown::getScrollHeight() const
{
  return mScrollHeight;
}

int Markdown::getScrollTop() const
{
  return mScrollTop;
}

int Markdown::getScrollWidth() const
{
  return mScrollWidth;
}

int Markdown::getScrollLeft() const
{
  return mScrollLeft;
}

void Markdown::setScrollTop( int top )
{
  mScrollTop = top;
  std::stringstream ss;
  ss << "window.scrollTo(" << mScrollLeft << ", " << mScrollTop << ")";
  mWebViewPtr->ExecuteJavascript(Awesomium::WSLit(ss.str().c_str()), Awesomium::WSLit(""));
}

void Markdown::setScrollLeft( int left )
{
  mScrollLeft = left;
  std::stringstream ss;
  ss << "window.scrollTo(" << mScrollLeft << ", " << mScrollTop << ")";
  mWebViewPtr->ExecuteJavascript(Awesomium::WSLit(ss.str().c_str()), Awesomium::WSLit(""));
}

void Markdown::setPageUpdatedCallback( const std::function<void(void)> &func )
{
  mPageUpdatedCallback = func;
}

}
}

#endif