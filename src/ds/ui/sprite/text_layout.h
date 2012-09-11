#pragma once
#ifndef DS_UI_SPRITE_TEXTLAYOUT_H
#define DS_UI_SPRITE_TEXTLAYOUT_H

#include <functional>
#include <vector>
#include <cinder/Vector.h>
#include <cinder/gl/TextureFont.h>

namespace ds {
namespace ui {
class Text;

enum Alignment
{
  Left,
  Right,
  Center
};
/**
 * \class ds::ui::TextLayout
 * A text layout is a series of lines, where each line
 * has a position (offset from 0,0) and text string.
 */
class TextLayout
{
  public:
    // A single line of text
    class Line {
      public:
        Line();
        ci::Vec2f         mPos;
        std::string       mText;
    };
    // A bundle of all data necessary to create a layout
    class Input {
      public:
        Input(const Text&, const ci::gl::TextureFontRef&, const ci::gl::TextureFont::DrawOptions&,
              const ci::Vec2f& size, const std::string& text);
        const Text&           mSprite;
        const ci::gl::TextureFontRef&
                              mFont;
        const ci::gl::TextureFont::DrawOptions&
                              mOptions;
        const ci::Vec2f&      mSize;
        const std::string&    mText;
      private:
        Input();
    };

  public:
    TextLayout();

    void                    clear();

    void                    addLine(const ci::Vec2f&, const std::string&);

    const std::vector<Line> getLines() const    { return mLines; }

    // Print my line info
    void                    debugPrint() const;

  private:
    std::vector<Line>       mLines;

  public:
    // Predefined layout functions.  A layout function needs to install
    // lines, typically where the Y value of each line is the baseline
    // (i.e. font ascent for the first line)
    typedef std::function<void(const TextLayout::Input&, TextLayout&)> MAKE_FUNC;

    static const MAKE_FUNC&   SINGLE_LINE();

    // Any layout function that needs additional information is supplied
    // as a separate class, below.
};

/**
 * \class ds::ui::TextLayoutVertical
 * A text layout that is bound by the text width, but will add lines
 * as necessary.  This class provides additional controls beyond the
 * standard text sprite.
 */
class TextLayoutVertical {
public:
  TextLayoutVertical();
  // Automatically install on the text object
  TextLayoutVertical(Text&);

  void                  installOn(Text&);

  // Adjust the font leading value, where 0 = no space between lines,
  // and 1 = the default leading.
  float                 mLeading;
  Alignment             mAlignment;
private:
  void                  run(const TextLayout::Input&, TextLayout&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_TEXTLAYOUT_H
