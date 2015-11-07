#include "ds/ui/sprite/text_layout.h"

#include <iostream>
#include "ds/data/data_buffer.h"
#include "ds/ui/sprite/text.h"
#include "ds/util/string_util.h"
#include "ds/debug/logger.h"
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
						 , mGenerateIndex(false)
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
	int linesSize = mLines.size();
	if(linesSize > 10000){
		linesSize = 10000;
		DS_LOG_WARNING("TextLayout: arbitrarily clipping to 10000 lines of text. If you need more, then search for this message and increase the number.");
	}
	buf.add(mLines.size());
	int k = 0;
	for(auto it = mLines.begin(), end = mLines.end(); it != end; ++it) {
		const Line& line(*it);
		buf.add(k);
		buf.add(line.mPos.x);
		buf.add(line.mPos.y);
		buf.add(line.mText);
		buf.add(line.mFontBox.getX1());
		buf.add(line.mFontBox.getY1());
		buf.add(line.mFontBox.getX2());
		buf.add(line.mFontBox.getY2());
		++k;
		if(k > 10000 - 1){
			break;
		}
	}
}

bool TextLayout::readFrom(ds::DataBuffer& buf)
{
	if(!buf.canRead<int>()) return false;
	const int count = buf.read<int>();
	// XXX Not really sure what the max count should be
	// GN it's 10000 now. CAUSE. BECAUSE. OK
	if(count < 0 || count > 10000) return false;
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
		l.mFontBox = ci::Rectf(buf.read<float>(), buf.read<float>(), buf.read<float>(), buf.read<float>());
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
		, mAlignment(Alignment::kLeft)
		, mGenerateIndices(false)
{
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

void TextLayoutVertical::addStringSegment(const FontPtr& font, const std::wstring& inputText){
	if(!mGenerateIndices) return;

	unsigned textSize = inputText.size();

	if(textSize == 0) return;

	// measure the 
	OGLFT::BBox box = font->measureRaw(inputText);
	float endX = mCurXPosition + box.x_max_ - box.x_min_;

	if(inputText == L" "){
		endX = mCurXPosition + mSpaceWidth;
	}
	
	// add the first position, which is the same as the advancement from the last character
	mCurLineIndexPositions[mCurInputIndex] = mCurXPosition;

	// Measure each character as part of the string, starting from the beginning of the current string
	for(unsigned int i = 1; i < textSize; i++){
		std::wstring substringy = inputText.substr(0, i);
		float thisW = 0.0f;
		if(!substringy.empty()){
			thisW = getSizeFromString(font, substringy).x;
		}

		mCurLineIndexPositions[mCurInputIndex + i] = mCurXPosition + thisW;
	}
	mCurXPosition = endX;
	mCurInputIndex = mCurInputIndex + textSize;
}

void TextLayoutVertical::addBlankSegment(){
	if(!mGenerateIndices) return;
	mCurLineIndexPositions[mCurInputIndex] = mCurXPosition;
	mCurInputIndex++;
}

void TextLayoutVertical::addLine(const FontPtr& font, const std::wstring& lineText){
	ci::Rectf fontBox = getBoxFromString(font, lineText);

	if(fontBox.getWidth() > mMaxWidth){
		mMaxWidth = fontBox.getWidth();
	}
	
	// Catch the case where the line only has empty space (tabs, spaces, returns)
	if(fontBox.getHeight() == 0.0f){
		fontBox.set(fontBox.getX1(), fontBox.getY1(), fontBox.getX2(), mLineHeight);
	}

	mOutputLines.push_back(TextLayout::Line());

	TextLayout::Line& thisLine = mOutputLines.back();
	thisLine.mPos.y = mY;
	thisLine.mText = lineText;
	thisLine.mFontBox = fontBox;	
	thisLine.mIndexPositions = mCurLineIndexPositions;

	mY += mLineHeight;
	mCurXPosition = 0.0f;
	mCurLineIndexPositions.clear();
}

void TextLayoutVertical::run(TextLayout::Input& in, TextLayout& out)
{
	mOutputLines.clear();
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

	// Keep track of the current line of text, in case the next token makes it too long to fit on a line
	std::wstring	lineText;

	LimitCheck			check(in);
	mY = ceilf((1.0f - getFontAscender(in.mFont)) * in.mFont->pointSize());
	mLineHeight = in.mFont->pointSize()*mLeading + in.mFont->pointSize();
	mCurInputIndex = 0;
	mCurXPosition = 0.0f;
	mCurLineIndexPositions.clear();
	mMaxWidth = 0.0f;
	mGenerateIndices = in.mGenerateIndex;

	OGLFT::BBox box = in.mFont->measureRaw(L" ");
	mSpaceWidth = box.advance_.dx_ * 1.25f; // ???? The actual advance amount doesn't seem to match the actual output, so multiply it?

	// Before we do anything, make sure we have room for the first line,
	// otherwise that will slip past.
	if(check.outOfBounds(mY)) return;

	for(size_t i = 0; i < tokens.size(); ++i) {
		// Test the new string to see if it's too long
		std::wstring	newLine(lineText);

		// If the new line is too large, then flush the previous and
		// continue with the current token

		const std::wstring &token = tokens[i];

		if(token == L" ") {
			addStringSegment(in.mFont, token);
			lineText.append(token);
			continue;

		} else if(token == L"\n") {
			addBlankSegment();
			addLine(in.mFont, lineText);

			lineText.clear();

			if(check.outOfBounds(mY)) break;

			continue;

		} else if(token == L"\t") {
			const std::wstring spacey = L" ";
			mCurLineIndexPositions[mCurInputIndex] = mCurXPosition;
			mCurXPosition += mSpaceWidth * 4.0f;
			mCurInputIndex++;

			lineText.append(spacey);
			lineText.append(spacey);
			lineText.append(spacey);
			lineText.append(spacey);
			continue;

		} else if(token == L"\r") {
			addBlankSegment();
			continue;

		} else if (token == L"-") {
			addStringSegment(in.mFont, token);
			lineText.append(token);
			continue;
		}

		newLine.append(token);
		ci::Vec2f size = getSizeFromString(in.mFont, newLine);
		if(size.x > in.mSize.x) {
			// Flush the current line, cause we're wrapping
			if(!lineText.empty()) {

				addLine(in.mFont, lineText);
				if(check.outOfBounds(mY)) break;
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

							addStringSegment(in.mFont, sub);
							addLine(in.mFont, sub);
						} 
						break;
					}
				}

				if(check.outOfBounds(mY)) break;

				size = getSizeFromString(in.mFont, lineText);
			}

			addStringSegment(in.mFont, lineText);
			if(check.outOfBounds(mY)) break;
		} else {
			addStringSegment(in.mFont, token);
			lineText.swap(newLine);
		}
	}

	// add in any remaining text on the last line
	if(!check.outOfBounds(mY)) {
		addBlankSegment();
		addLine(in.mFont, lineText);
	}

	if(mMaxWidth > in.mSize.x){
		mMaxWidth = in.mSize.x;
	}

	for(auto it = mOutputLines.begin(), it2 = mOutputLines.end(); it != it2; ++it){
		TextLayout::Line& thisLine = (*it);
		float y = thisLine.mPos.y;
		std::wstring &str = thisLine.mText;

		if(mAlignment == Alignment::kLeft) {
			thisLine.mPos.x = 0.0f;
		} else if(mAlignment == Alignment::kRight) {
			thisLine.mPos.x = mMaxWidth - thisLine.mFontBox.getWidth();
		} else {
			float size = getSizeFromString(in.mFont, str).x;
			thisLine.mPos.x = (mMaxWidth - thisLine.mFontBox.getWidth()) / 2.0f;
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
