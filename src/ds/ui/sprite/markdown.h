#pragma once
#ifndef DS_MARKDOWN_H
#define DS_MARKDOWN_H
#include "sprite.h"

namespace Awesomium {

class WebSession;
class WebView;

}

namespace ds {
namespace ui {

class Markdown : public Sprite
{
  public:
    Markdown(SpriteEngine &engine, float width = 0.0f, float height = 0.0f);
    virtual ~Markdown();

    void setSizeAll(float width, float height, float depth);
    void updateServer(const ds::UpdateParams &updateParams);
    void drawLocalClient();

    void loadUrl(const std::string &url);
    void loadUrl(const std::wstring &url);

    void setResizeToLength(bool flag);
    void setText(const std::string &text);
    void setText(const std::wstring &text);
    const std::wstring &getText() const;

    int getScrollHeight() const;
    //scroll position
    //only values from 0 to (getScrollHeight() - getHeight())
    int getScrollTop() const;
    int getScrollWidth() const;
    //scroll position
    //only values from 0 to (getScrollWidth() - getWidth())
    int getScrollLeft() const;

    //only values from 0 to (getScrollHeight() - getHeight())
    void setScrollTop(int top);
    //only values from 0 to (getScrollWidth() - getWidth())
    void setScrollLeft(int left);

    void setPageUpdatedCallback(const std::function<void(void)> &func);
  protected:
  private:
	  Awesomium::WebView    *mWebViewPtr;
    Awesomium::WebSession *mWebSessionPtr;

    ci::gl::Texture				mWebTexture;

    int mScrollHeight;
    int mScrollTop;
    int mScrollWidth;
    int mScrollLeft;

    bool mResizeToLength;

    std::function<void(void)> mPageUpdatedCallback;

    std::wstring mText;
};

}
}

#endif//DS_MARKDOWN_H
