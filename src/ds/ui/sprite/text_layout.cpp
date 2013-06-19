#include "ds/ui/sprite/text_layout.h"

#include <iostream>
#include "ds/data/data_buffer.h"
#include "ds/ui/sprite/text.h"
#include "ds/util/string_util.h"
#include <time.h>

using namespace ci;

static void tokenize(const std::string& ws, std::vector<std::string>& tokens);

namespace ds {
namespace ui {

int getFontSize( const FontPtr &font )
{
  return abs(font->ascender())+abs(font->descender());
}

float getFontAscender( const FontPtr &font )
{
  return font->ascender() / (float)getFontSize(font);
}

float getFontDescender( const FontPtr &font )
{
  return font->descender() / (float)getFontSize(font);
}

float getFontHeight( const FontPtr &font, const float leading )
{
	if (!font) return 0.0f;
	return font->pointSize()*leading + font->pointSize();
}

namespace {
  
class LimitCheck {
  public:
    LimitCheck(const TextLayout::Input& in)
      : mLimitToHeight(!(in.mSprite.autoResizeHeight()))
      , mDescent(getFontDescender(in.mFont))
      , mMaxY(in.mSize.y)
    {
    }

    inline bool outOfBounds(const float y) const
    {
      // Not doing the limit check any more -- it just makes sense to never bother
      // with this, instead always having a limit, but using an unreasonably high one
      // for "unlimited"
//      if (!limitToHeight) return false;
      return (y + mDescent > mMaxY);
    }

  private:
    const bool    mLimitToHeight;
    const float   mDescent;
    const float   mMaxY;
};

}

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
                          const FontPtr& f,
                          const ci::Vec2f& size,
                          const std::wstring& text)
  : mSprite(sprite)
  , mFont(f)
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

void TextLayout::addLine(const ci::Vec2f& pos, const std::wstring& text)
{
  mLines.push_back(Line());
  Line&     l = mLines.back();
  l.mPos = pos;
  l.mText = text;
}

void TextLayout::writeTo(ds::DataBuffer& buf) const
{
  buf.add(mLines.size());
  int k = 0;
  for (auto it=mLines.begin(), end=mLines.end(); it != end; ++it) {
    const Line&   line(*it);
    buf.add(k);
    buf.add(line.mPos.x);
    buf.add(line.mPos.y);
    buf.add(line.mText);
    ++k;
  }
}

bool TextLayout::readFrom(ds::DataBuffer& buf)
{
  if (!buf.canRead<int>()) return false;
  const int     count = buf.read<int>();
  // XXX Not really sure what the max count should be
  if (count < 0 || count > 255) return false;
  for (int k=0; k<count; ++k) {
    if (!buf.canRead<int>()) return false;
    if (buf.read<int>() != k) return false;

    mLines.push_back(Line());
    Line& l = mLines.back();

    if (!buf.canRead<float>()) return false;
    l.mPos.x = buf.read<float>();
    if (!buf.canRead<float>()) return false;
    l.mPos.y = buf.read<float>();
    l.mText = buf.read<std::wstring>();
  }
  return true;
}

void TextLayout::debugPrint() const
{
  int             k = 0;
  for (auto it=mLines.begin(), end=mLines.end(); it != end; ++it) {
    const Line&   line(*it);
    std::wcout << L"\t" << k << L" (" << line.mPos.x << L", " << line.mPos.y << L") " << line.mText << std::endl;
  }
}

const TextLayout::MAKE_FUNC& TextLayout::SINGLE_LINE()
{
  static const MAKE_FUNC    ANS = [](const TextLayout::Input& i, TextLayout& l) { l.addLine(ci::Vec2f(0, ceilf((1.0f - getFontAscender(i.mFont)) * i.mFont->pointSize())), i.mText); };
  return ANS;
}

/**
 * \class ds::ui::TextLayoutVertical
 */
TextLayoutVertical::TextLayoutVertical()
  : mLeading(1)
  , mAlignment(Left)
{
}

TextLayoutVertical::TextLayoutVertical(Text& t)
  : mLeading(1)
  , mAlignment(Left)
{
  installOn(t);
}

void TextLayoutVertical::installOn(Text& t)
{
  auto f = [this](const TextLayout::Input& in, TextLayout& out) { this->run(in, out); };
  t.setLayoutFunction(f);
}

ci::Vec2f getSizeFromString(const FontPtr &font, const std::string &str)
{
  OGLFT::BBox box = font->measureRaw(str);
  ci::Vec2f   size(box.x_max_-box.x_min_, box.y_max_-box.y_min_);

  return size;
}

ci::Vec2f getSizeFromString( const FontPtr &font, const std::wstring &str )
{
  OGLFT::BBox box = font->measureRaw(str);
  ci::Vec2f   size(box.x_max_-box.x_min_, box.y_max_-box.y_min_);

  return size;
}

float getDifference(const FontPtr &font, const std::wstring &str, float lineH, float leading)
{
  //float lineHeight = getSizeFromString(font, str).y + font->height() * leading;
  //if (lineHeight < lineH)
  //  return lineH - lineHeight;
  return 0.0f;
}

void TextLayoutVertical::run(const TextLayout::Input& in, TextLayout& out)
{
  if (in.mText.empty())
    return;
  // Per line, find the word breaks, then create a line.
  std::vector<std::wstring>    tokens;
  //tokenize(in.mText, tokens);
  static std::vector<std::wstring> partitioners;
  if (partitioners.empty()) {
    partitioners.push_back(L" ");
    partitioners.push_back(L"-");
    partitioners.push_back(L"|");
    //partitioners.push_back(L"_");
    partitioners.push_back(L"\n");
    partitioners.push_back(L"\r");
    partitioners.push_back(L"\t");
    //partitioners.push_back(L",");
  }

  tokens = ds::partition(in.mText, partitioners);

  LimitCheck                  check(in);
  float                       y = ceilf((1.0f - getFontAscender(in.mFont)) * in.mFont->pointSize());
                                                                                       //address this
  const float                 lineH = in.mFont->pointSize()*mLeading + in.mFont->pointSize();//in.mFont->ascender() + in.mFont->descender() + (in.mFont->getFont().getLeading()*mLeading);
  std::wstring                 lineText;

  // Before we do anything, make sure we have room for the first line,
  // otherwise that will slip past.
  if (check.outOfBounds(y)) return;

  float maxWidth = 0.0f;
  // spaces don't have a size so we make something up
  const ci::Vec2f             spaceSize = getSizeFromString(in.mFont, L"o");
  //const ci::Vec2f             spaceSize = in.mFont->measureString("o", in.mOptions);
  const ci::Vec2f             tabSize(spaceSize.x*3.0f, spaceSize.y);
  //for (auto it=tokens.begin(), end=tokens.end(); it != end; ++it) {
  typedef std::pair<float,std::wstring> outPair;
  std::vector<outPair> linesToWrite;

  for (size_t i = 0; i < tokens.size(); ++i) {
    // Test the new string to see if it's too long
    std::wstring               newLine(lineText);
    // If the new line is too large, then flush the previous and
    // continue with the current token
    const std::wstring &token = tokens[i];
    if (token == L" ") {
      lineText.append(L" ");
      continue;
    } else if (token == L"\n" ) {
      // Flush the current line
      if (!lineText.empty()) {
        float inSize = getSizeFromString(in.mFont, lineText).x;
        if (inSize > maxWidth)
          maxWidth = inSize;
        linesToWrite.push_back(outPair(y, lineText));
        ////out.addLine(ci::Vec2f(0, y + getDifference(in.mFont, lineText, lineH, mLeading)), lineText);
        //if (mAlignment == Left) {
        //  //out.addLine(ci::Vec2f(0, y), lineText);
        //  linesToWrite.push_back(outPair(y, lineText));
        //} else if (mAlignment == Right) {
        //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
        //  float x = in.mSize.x - size;
        //  out.addLine(ci::Vec2f(x, y), lineText);
        //} else {
        //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
        //  float x = (in.mSize.x - size) / 2.0f;
        //  out.addLine(ci::Vec2f(x, y), lineText);
        //}
      }
      lineText.clear();
      //lineText.append(" ");
      y += lineH;
      if (check.outOfBounds(y)) return;
      continue;
    } else if (token == L"\t") {
      lineText.append(L" ");
      lineText.append(L" ");
      lineText.append(L" ");
      lineText.append(L" ");
      continue;
    } else if (token == L"\r") {
      continue;
    }

    newLine.append(token);
    ci::Vec2f           size = getSizeFromString(in.mFont, newLine);//in.mFont->measureString(newLine, in.mOptions);
    if (size.x > in.mSize.x) {
      // Flush the current line
      if (!lineText.empty()) {
        float inSize = getSizeFromString(in.mFont, lineText).x;
        if (inSize > maxWidth)
          maxWidth = inSize;
        linesToWrite.push_back(outPair(y, lineText));
        //if (mAlignment == Left) {
        //  out.addLine(ci::Vec2f(0, y), lineText);
        //} else if (mAlignment == Right) {
        //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
        //  float x = in.mSize.x - size;
        //  out.addLine(ci::Vec2f(x, y), lineText);
        //} else {
        //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
        //  float x = (in.mSize.x - size) / 2.0f;
        //  out.addLine(ci::Vec2f(x, y), lineText);
        //}
        y += lineH;
        if (check.outOfBounds(y)) return;
      }

      lineText = token;

      size = getSizeFromString(in.mFont, lineText);//in.mFont->measureString(lineText, in.mOptions);
      while (size.x > in.mSize.x) {
        for (unsigned i = 1; i <= lineText.size(); ++i) {
        	float cSize = getSizeFromString(in.mFont, lineText.substr(0, i)).x;//in.mFont->measureString(lineText.substr(0, i), in.mOptions).x;
          if (cSize > in.mSize.x && i > 0) {
            // Eric, you said there was an infinite loop here with the code like this. The way you changed it wasn't the correct
            // split and would cause lettings out of bounds. If you get the infinite loop again let me know how to reproduce it.
            std::wstring sub = lineText.substr(0, i-1);
            lineText = lineText.substr(i-1, lineText.size() - i + 1);
            if (!sub.empty()) {
              float inSize = getSizeFromString(in.mFont, sub).x;
              if (inSize > maxWidth)
                maxWidth = inSize;
              linesToWrite.push_back(outPair(y, sub));
              //if (mAlignment == Left) {
              //  out.addLine(ci::Vec2f(0, y), sub);
              //} else if (mAlignment == Right) {
              //  float size = getSizeFromString(in.mFont, sub).x;//in.mFont->measureString(sub, in.mOptions).x;
              //  float x = in.mSize.x - size;
              //  out.addLine(ci::Vec2f(x, y), sub);
              //} else {
              //  float size = getSizeFromString(in.mFont, sub).x;//in.mFont->measureString(sub, in.mOptions).x;
              //  float x = (in.mSize.x - size) / 2.0f;
              //  out.addLine(ci::Vec2f(x, y), sub);
              //}

              y += lineH;
              break;
            } else {
              return;
            }
          }
        }

        if (check.outOfBounds(y)) return;

        size = getSizeFromString(in.mFont, lineText);//in.mFont->measureString(lineText, in.mOptions);
      }

      //lineText.append(" ");
      //y += lineH;
      // If the sprite is not auto resizing, then don't go past its bounds
      if (check.outOfBounds(y)) return;
      // Otherwise maintain the new line.
    } else {
      lineText.swap(newLine);
      //lineText.append(" ");
    }
  }

  if (!lineText.empty() && !check.outOfBounds(y)) {
    float inSize = getSizeFromString(in.mFont, lineText).x;
    if (inSize > maxWidth)
      maxWidth = inSize;
    linesToWrite.push_back(outPair(y, lineText));
    //if (mAlignment == Left) {
    //  out.addLine(ci::Vec2f(0, y), lineText);
    //} else if (mAlignment == Right) {
    //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
    //  float x = in.mSize.x - size;
    //  out.addLine(ci::Vec2f(x, y), lineText);
    //} else {
    //  float size = getSizeFromString(in.mFont, lineText).x;//in.mFont->measureString(lineText, in.mOptions).x;
    //  float x = (in.mSize.x - size) / 2.0f;
    //  out.addLine(ci::Vec2f(x, y), lineText);
    //}
  }

  if (maxWidth > in.mSize.x)
    maxWidth = in.mSize.x;

  for (auto it = linesToWrite.begin(), it2 = linesToWrite.end(); it != it2; ++it)
  {
  	float y = it->first;
    std::wstring &str = it->second;

    if (mAlignment == Left) {
      out.addLine(ci::Vec2f(0, y), str);
    } else if (mAlignment == Right) {
      float size = getSizeFromString(in.mFont, str).x;//in.mFont->measureString(lineText, in.mOptions).x;
      float x = maxWidth - size;
      out.addLine(ci::Vec2f(x, y), str);
    } else {
      float size = getSizeFromString(in.mFont, str).x;//in.mFont->measureString(lineText, in.mOptions).x;
      float x = (maxWidth - size) / 2.0f;
      out.addLine(ci::Vec2f(x, y), str);
    }
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
