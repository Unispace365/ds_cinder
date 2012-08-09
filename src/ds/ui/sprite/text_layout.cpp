#include "ds/ui/sprite/text_layout.h"

#include <iostream>
#include "ds/ui/sprite/text.h"
#include "ds/util/string_util.h"

using namespace ci;

static void tokenize(const std::string& ws, std::vector<std::string>& tokens);

namespace ds {
namespace ui {

/**
 * \class ds::ui::TextLayout::Line
 */
TextLayout::Line::Line()
{
}

/**
 * \class ds::ui::TextLayout::Input
 */
TextLayout::Input::Input( const Text& sprite,
                          const ci::gl::TextureFontRef& f,
                          const ci::gl::TextureFont::DrawOptions& o,
                          const ci::Vec2f& size,
                          const std::string& text)
  : mSprite(sprite)
  , mFont(f)
  , mOptions(o)
  , mSize(size)
  , mText(text)
{
}

/**
 * \class ds::ui::TextLayout
 */
TextLayout::TextLayout()
{
}

void TextLayout::clear()
{
  mLines.clear();
}

void TextLayout::addLine(const ci::Vec2f& pos, const std::string& text)
{
  mLines.push_back(Line());
  Line&     l = mLines.back();
  l.mPos = pos;
  l.mText = text;
}

void TextLayout::debugPrint() const
{
  int             k = 0;
  for (auto it=mLines.begin(), end=mLines.end(); it != end; ++it) {
    const Line&   line(*it);
    std::cout << "\t" << k << " (" << line.mPos.x << ", " << line.mPos.y << ") " << line.mText << std::endl;
  }
}

const TextLayout::MAKE_FUNC& TextLayout::SINGLE_LINE()
{
  static const MAKE_FUNC    ANS = [](const TextLayout::Input& i, TextLayout& l) { l.addLine(ci::Vec2f(0, i.mFont->getAscent()), i.mText); };
  return ANS;
}

/**
 * \class ds::ui::TextLayoutVertical
 */
TextLayoutVertical::TextLayoutVertical()
  : mLeading(1)
{
}

TextLayoutVertical::TextLayoutVertical(Text& t)
  : mLeading(1)
{
  installOn(t);
}

void TextLayoutVertical::installOn(Text& t)
{
  auto f = [this](const TextLayout::Input& in, TextLayout& out) { this->run(in, out); };
  t.setLayoutFunction(f);
}

void TextLayoutVertical::run(const TextLayout::Input& in, TextLayout& out)
{
  // Per line, find the word breaks, then create a line.
  std::vector<std::string>    tokens;
  //tokenize(in.mText, tokens);
  std::vector<std::string> partitioners;
  partitioners.push_back(" ");
  partitioners.push_back("-");
  partitioners.push_back("_");
  partitioners.push_back("\n");
  partitioners.push_back("\t");
  tokens = ds::partition(in.mText, partitioners);

  // Now that I think about it, it makes a lot more sense to just always limit
  // myself to the supplied dimensions, which will be absurdly high for clients
  // that don't want to be limited.
//  const bool                  limitToHeight = !(in.mSprite.autoResizeHeight());
  const bool                  limitToHeight = true;
  float                       y = in.mFont->getAscent();
  const float                 lineH = in.mFont->getAscent() + in.mFont->getDescent() + (in.mFont->getFont().getLeading()*mLeading);
  std::string                 lineText;

  // Before we do anything, make sure we have room for the first line,
  // otherwise that will slip past.
  if (limitToHeight) {
    if (y + in.mFont->getDescent() > in.mSize.y) return;
  }

  // spaces don't have a size so we make something up
  const ci::Vec2f             spaceSize = in.mFont->measureString("o", in.mOptions);
  const ci::Vec2f             tabSize(spaceSize.x*3.0f, spaceSize.y);
  for (auto it=tokens.begin(), end=tokens.end(); it != end; ++it) {
    // Test the new string to see if it's too long
    std::string               newLine(lineText);
    // If the new line is too large, then flush the previous and
    // continue with the current token
    if (*it == " ") {
      lineText.append(" ");
      continue;
    } else if (*it == "\n") {
      // Flush the current line
      if (!lineText.empty()) {
        out.addLine(ci::Vec2f(0, y), lineText);
      }
      lineText.clear();
      //lineText.append(" ");
      y += lineH;
      // If the sprite is not auto resizing, then don't go past its bounds
      if (limitToHeight && y > in.mSize.y)
        break;
      continue;
    } else if (*it == "\t") {
      lineText.append(" ");
      lineText.append(" ");
      lineText.append(" ");
      lineText.append(" ");
      continue;
    }

    newLine.append(*it);
    ci::Vec2f           size = in.mFont->measureString(newLine, in.mOptions);
    if (size.x > in.mSize.x) {
      // Flush the current line
      if (!lineText.empty()) {
        out.addLine(ci::Vec2f(0, y), lineText);
        y += lineH;
      }

      lineText = *it;

      size = in.mFont->measureString(lineText, in.mOptions);
      while (size.x > in.mSize.x) {
        for (unsigned i = 1; i <= lineText.size(); ++i) {
        	float cSize = in.mFont->measureString(lineText.substr(0, i), in.mOptions).x;
          if (cSize > in.mSize.x && i > 0) {
            std::string sub = lineText.substr(0, i-1);
            lineText = lineText.substr(i-1, lineText.size() - (i-1));
            if (!sub.empty()) {
              out.addLine(ci::Vec2f(0, y), sub);
              y += lineH;
            }
            break;
          }
        }

        if (limitToHeight && y > in.mSize.y)
          break;

        size = in.mFont->measureString(lineText, in.mOptions);
      }

      //lineText.append(" ");
      //y += lineH;
      // If the sprite is not auto resizing, then don't go past its bounds
      if (limitToHeight && y > in.mSize.y)
        break;
      // Otherwise maintain the new line.
    } else {
      lineText.swap(newLine);
      //lineText.append(" ");
    }
  }
  //for (auto it=tokens.begin(), end=tokens.end(); it != end; ++it) {
  //  // Test the new string to see if it's too long
  //  std::string               newLine(lineText);
  //  newLine.append(*it);
  //  const ci::Vec2f           size = in.mFont->measureString(newLine, in.mOptions);
  //  // If the new line is too large, then flush the previous and
  //  // continue with the current token
  //  if (size.x > in.mSize.x) {
  //    // Flush the current line
  //    if (!lineText.empty()) {
  //      out.addLine(ci::Vec2f(0, y), lineText);
  //    }
  //    lineText = *it;
  //    lineText.append(" ");
  //    y += lineH;
  //    // If the sprite is not auto resizing, then don't go past its bounds
  //    if (limitToHeight && y + lineH > in.mSize.y) break;
  //  // Otherwise maintain the new line.
  //  } else {
  //    lineText.swap(newLine);
  //    lineText.append(" ");
  //  }
  //}
  if (!lineText.empty() && !(limitToHeight && y > in.mSize.y)) {
    out.addLine(ci::Vec2f(0, y), lineText);
  }
}

} // namespace ui
} // namespace ds

/* TOKENIZE
 * Special function that breaks things first by line, then by white space.
 *************************************************************************/
static void tokenize(const std::string& ws, std::vector<std::string>& tokens)
{
	try {
		// First by line...
		std::istringstream			lineBuf(ws);
		while (lineBuf.good()) {
			std::string					out;
			std::getline(lineBuf, out);
			if (tokens.size() > 0) tokens.push_back("\n");
			
			// ...then by white space
			if (out.length() > 0) {
				std::istringstream		wordBuf(out);
				std::istream_iterator<std::string, char, std::char_traits<char> >	it(wordBuf);
				std::istream_iterator<std::string, char, std::char_traits<char> >	end;
				while (it != end) {
					tokens.push_back(*it);
					++it;
				}
			}
		}
	} catch (std::exception&) {
	}
}
