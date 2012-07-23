#include "ds/ui/sprite/text_layout.h"

#include "ds/ui/sprite/text.h"

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
TextLayout::Input::Input( const ci::gl::TextureFontRef& f,
                          const ci::gl::TextureFont::DrawOptions& o,
                          const ci::Vec2f& size,
                          const std::string& text)
  : mFont(f)
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
  tokenize(in.mText, tokens);
  float                       y = in.mFont->getAscent();
  const float                 lineH = in.mFont->getAscent() + in.mFont->getDescent() + (in.mFont->getFont().getLeading()*mLeading);
  std::string                 lineText;
  // spaces don't have a size so we make something up
  const ci::Vec2f             spaceSize = in.mFont->measureString("o", in.mOptions);
  for (auto it=tokens.begin(), end=tokens.end(); it != end; ++it) {
    // Test the new string to see if it's too long
    std::string               newLine(lineText);
    newLine.append(*it);
    const ci::Vec2f           size = in.mFont->measureString(newLine, in.mOptions);
    // If the new line is too large, then flush the previous and
    // continue with the current token
    if (size.x > in.mSize.x) {
      // Flush the current line
      if (!lineText.empty()) {
        out.addLine(ci::Vec2f(0, y), lineText);
      }
      lineText = *it;
      lineText.append(" ");
      y += lineH;
    // Otherwise maintain the new line.
    } else {
      lineText.swap(newLine);
      lineText.append(" ");
    }
  }
  if (!lineText.empty()) {
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
