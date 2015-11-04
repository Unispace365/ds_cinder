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
	int size = getFontSize(font);
	if(size > 0){
		return (float)font->ascender() / (float)size;
	} else {
		return (float)font->ascender();
	}
}

float getFontDescender( const FontPtr &font )
{
	int size = getFontSize(font);
	if(size > 0){
		return (float)font->descender() / (float)size;
	} else {
		return (float)font->descender();
	}
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
TextLayout::Input::Input(const Text& sprite,
						 const FontPtr& f,
						 const ci::Vec2f& size,
						 const std::wstring& text)
						 : mSprite(sprite)
						 , mFont(f)
						 , mSize(size)
						 , mText(text)
						 , mLineWasSplit(false)
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

void TextLayout::addLine(const TextLayout::Line& newLine){
	mLines.push_back(newLine);
}

void TextLayout::writeTo(ds::DataBuffer& buf) const
{
	buf.add(mLines.size());
	int k = 0;
	for(auto it = mLines.begin(), end = mLines.end(); it != end; ++it) {
		const Line& line(*it);
		buf.add(k);
		buf.add(line.mPos.x);
		buf.add(line.mPos.y);
		buf.add(line.mText);
		++k;
	}
}

bool TextLayout::readFrom(ds::DataBuffer& buf)
{
	if(!buf.canRead<int>()) return false;
	const int count = buf.read<int>();
	// XXX Not really sure what the max count should be
	if(count < 0 || count > 255) return false;
	for(int k = 0; k < count; ++k) {
		if(!buf.canRead<int>()) return false;
		if(buf.read<int>() != k) return false;

		mLines.push_back(Line());
		Line& l = mLines.back();

		if(!buf.canRead<float>()) return false;
		l.mPos.x = buf.read<float>();
		if(!buf.canRead<float>()) return false;
		l.mPos.y = buf.read<float>();
		l.mText = buf.read<std::wstring>();
	}
	return true;
}

void TextLayout::debugPrint() const
{
	int					k = 0;
	for(auto it = mLines.begin(), end = mLines.end(); it != end; ++it) {
		const Line&		line(*it);
		std::wcout << L"\t" << k << L" (" << line.mPos.x << L", " << line.mPos.y << L") " << line.mText << std::endl;
	}
}

const TextLayout::MAKE_FUNC& TextLayout::SINGLE_LINE()
{
	static const MAKE_FUNC ANS = [](TextLayout::Input& i, TextLayout& l) { 
		TextLayout::Line newLine;
		newLine.mPos = ci::Vec2f(0, ceilf((1.0f - getFontAscender(i.mFont)) * i.mFont->pointSize()));
		newLine.mFontBox = getBoxFromString(i.mFont, i.mText);
		newLine.mText = i.mText;
		l.addLine(newLine); 
	};
	return ANS;
}

/**
 * \class ds::ui::TextLayoutVertical
 */
TextLayoutVertical::TextLayoutVertical()
		: mLeading(1)
		, mAlignment(Alignment::kLeft) {
}

TextLayoutVertical::TextLayoutVertical(Text& t)
		: mLeading(1)
		, mAlignment(Alignment::kLeft) {
	installOn(t);
}

void TextLayoutVertical::installOn(Text& t) {
	auto f = [this](TextLayout::Input& in, TextLayout& out) { this->run(in, out); };
	t.setLayoutFunction(f);
}

ci::Vec2f getSizeFromString(const FontPtr &font, const std::string &str){
	OGLFT::BBox box = font->measureRaw(str);
	return ci::Vec2f(box.x_max_ - box.x_min_, box.y_max_ - box.y_min_);
}

ci::Vec2f getSizeFromString(const FontPtr &font, const std::wstring &str){
	OGLFT::BBox box = font->measureRaw(str);
	return ci::Vec2f(box.x_max_ - box.x_min_, box.y_max_ - box.y_min_);
}

ci::Rectf getBoxFromString(const FontPtr &font, const std::wstring &str){
	OGLFT::BBox box = font->measureRaw(str);
	return ci::Rectf(box.x_min_, box.y_min_, box.x_max_, box.y_max_);
}

void TextLayoutVertical::addLine(const FontPtr& font, const std::wstring& lineText, const float y, std::vector<TextLayout::Line>& outputVector, float& inOutMaxWidth){
	ci::Rectf fontBox = getBoxFromString(font, lineText);

	if(fontBox.getWidth() > inOutMaxWidth){
		inOutMaxWidth = fontBox.getWidth();
	}
	outputVector.push_back(TextLayout::Line());

	TextLayout::Line& thisLine = outputVector.back();
	thisLine.mPos.y = y;
	thisLine.mText = lineText;
	thisLine.mFontBox = fontBox;	
}

void TextLayoutVertical::run(TextLayout::Input& in, TextLayout& out)
{
	if(in.mText.empty())
		return;
	// Per line, find the word breaks, then create a line.
	std::vector<std::wstring>			tokens;

	static std::vector<std::wstring>	partitioners;
	if(partitioners.empty()) {
		partitioners.push_back(L" ");
		partitioners.push_back(L"-");
		partitioners.push_back(L"|");
		partitioners.push_back(L"\n");
		partitioners.push_back(L"\r");
		partitioners.push_back(L"\t");
	}

	tokens = ds::partition(in.mText, partitioners);

	LimitCheck			check(in);
	float				y = ceilf((1.0f - getFontAscender(in.mFont)) * in.mFont->pointSize());
	//address this
	const float			lineH = in.mFont->pointSize()*mLeading + in.mFont->pointSize();
	std::wstring		lineText;

	// Before we do anything, make sure we have room for the first line,
	// otherwise that will slip past.
	if(check.outOfBounds(y)) return;

	float maxWidth = 0.0f;

	std::vector<TextLayout::Line> linesToWrite;

	for(size_t i = 0; i < tokens.size(); ++i) {
		// Test the new string to see if it's too long
		std::wstring	newLine(lineText);

		// If the new line is too large, then flush the previous and
		// continue with the current token

		const std::wstring &token = tokens[i];

		if(token == L" ") {
			lineText.append(L" ");
			continue;
		} else if(token == L"\n") {
			// Flush the current line
			if(!lineText.empty()) {
				addLine(in.mFont, lineText, y, linesToWrite, maxWidth);
			}

			lineText.clear();

			y += lineH;
			if(check.outOfBounds(y)) break;
			continue;
		} else if(token == L"\t") {
			lineText.append(L" ");
			lineText.append(L" ");
			lineText.append(L" ");
			lineText.append(L" ");
			continue;
		} else if(token == L"\r") {
			continue;
		} else if (token == L"-") {
			lineText.append(token);
			continue;
		}

		newLine.append(token);
		ci::Vec2f size = getSizeFromString(in.mFont, newLine);
		if(size.x > in.mSize.x) {
			// Flush the current line
			if(!lineText.empty()) {

				addLine(in.mFont, lineText, y, linesToWrite, maxWidth);
				y += lineH;
				if(check.outOfBounds(y)) break;
			}

			lineText = token;

			size = getSizeFromString(in.mFont, lineText);

			// This detects if a single 'word' is wider than the available area
			// If it is, break it down into displayable chunks
			while(size.x > in.mSize.x) {
				for(unsigned i = 1; i <= lineText.size(); ++i) {
					float cSize = getSizeFromString(in.mFont, lineText.substr(0, i)).x;
					if(cSize > in.mSize.x && i > 0) {
						std::wstring sub = lineText.substr(0, i - 1);
						lineText = lineText.substr(i - 1, lineText.size() - i + 1);
						if(!sub.empty()) {
							in.mLineWasSplit = true;

							addLine(in.mFont, sub, y, linesToWrite, maxWidth);
							y += lineH;
						} 
						break;
					}
				}

				if(check.outOfBounds(y)) break;

				size = getSizeFromString(in.mFont, lineText);
			}

			if(check.outOfBounds(y)) break;
		} else {
			lineText.swap(newLine);
		}
	}

	// add in any remaining text on the last line
	if(!lineText.empty() && !check.outOfBounds(y)) {
		addLine(in.mFont, lineText, y, linesToWrite, maxWidth);
	}

	if(maxWidth > in.mSize.x){
		maxWidth = in.mSize.x;
	}

	for(auto it = linesToWrite.begin(), it2 = linesToWrite.end(); it != it2; ++it){
		TextLayout::Line& thisLine = (*it);
		float y = thisLine.mPos.y;
		std::wstring &str = thisLine.mText;

		if(mAlignment == Alignment::kLeft) {
			thisLine.mPos.x = 0.0f;
		} else if(mAlignment == Alignment::kRight) {
			thisLine.mPos.x = maxWidth - thisLine.mFontBox.getWidth();
		} else {
			float size = getSizeFromString(in.mFont, str).x;
			thisLine.mPos.x = (maxWidth - thisLine.mFontBox.getWidth() ) / 2.0f;
		}

		out.addLine(thisLine);
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
		while(lineBuf.good()) {
			std::string					out;
			std::getline(lineBuf, out);
			if(tokens.size() > 0) tokens.push_back("\n");

			// ...then by white space
			if(out.length() > 0) {
				std::istringstream		wordBuf(out);
				std::istream_iterator<std::string, char, std::char_traits<char> >	it(wordBuf);
				std::istream_iterator<std::string, char, std::char_traits<char> >	end;
				while(it != end) {
					tokens.push_back(*it);
					++it;
				}
			}
		}
	} catch(std::exception&) {
	}
}
