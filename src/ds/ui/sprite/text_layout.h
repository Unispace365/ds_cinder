#pragma once
#ifndef DS_UI_SPRITE_TEXTLAYOUT_H
#define DS_UI_SPRITE_TEXTLAYOUT_H

#include <functional>
#include <vector>
#include <cinder/Vector.h>
#include <cinder/gl/TextureFont.h>
#include <OGLFT.h>
#include <ds/ui/sprite/text_defs.h>

typedef std::shared_ptr<OGLFT::Translucent> FontPtr;

namespace ds {
class DataBuffer;

namespace ui {
class Text;

ci::Vec2f getSizeFromString(const FontPtr &font, const std::string &str);
ci::Vec2f getSizeFromString(const FontPtr &font, const std::wstring &str);
ci::Rectf getBoxFromString(const FontPtr &font, const std::wstring &str);
int getFontSize(const FontPtr &font);
float getFontAscender(const FontPtr &font);
float getFontDescender(const FontPtr &font);
float getFontHeight(const FontPtr &font, const float leading);

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
		ci::Vec2f				mPos;
		ci::Rectf				mFontBox;
		std::wstring			mText;		// potentially modified by the layout class from the original input (tabs -> four spaces, returns, etc)
		std::map<int, float>	mIndexPositions; // maps input string indices to x-pixel-position in the line. x-position is relative to the mPos.x value of this line
	};
	// A bundle of all data necessary to create a layout
	class Input {
	public:
		Input(const Text&, const FontPtr &,
			const ci::Vec2f& size, const std::wstring& text);
		const Text&			mSprite;
		const FontPtr&		mFont;
		const ci::Vec2f&	mSize;
		const std::wstring&	mText;
		bool				mLineWasSplit;
		bool				mGenerateIndex; // generate positions of each character, potentially expensive operation
	private:
		Input();
	};

public:
	TextLayout();

	void					clear();

	void					addLine(const TextLayout::Line& newLine);

	const std::vector<Line> getLines() const	{ return mLines; }

	void					writeTo(ds::DataBuffer&) const;
	bool					readFrom(ds::DataBuffer&);

	// Print my line info
	void					debugPrint() const;

private:
	std::vector<Line>		mLines;

public:
	// Predefined layout functions. A layout function needs to install
	// lines, typically where the Y value of each line is the baseline
	// (i.e. font ascent for the first line)
	typedef std::function<void(TextLayout::Input&, TextLayout&)> MAKE_FUNC;

	static const MAKE_FUNC&	SINGLE_LINE();

	// Any layout function that needs additional information is supplied
	// as a separate class, below.
};

/**
 * \class ds::ui::TextLayoutVertical
 * A text layout that is bound by the text width, but will add lines
 * as necessary. This class provides additional controls beyond the
 * standard text sprite.
 */
class TextLayoutVertical {
public:
	TextLayoutVertical();
	// Automatically install on the text object
	TextLayoutVertical(Text&);

	void					installOn(Text&);

	// Adjust the font leading value, where 0 = no space between lines,
	// and 1 = the default leading.
	float					mLeading;
	Alignment::Enum			mAlignment;
private:

	// some 'temp' variables to use during the layout
	std::map<int, float>			mCurLineIndexPositions;
	float							mCurXPosition;
	int								mCurInputIndex;
	float							mMaxWidth;
	std::vector<TextLayout::Line>	mOutputLines;
	float							mY;
	float							mLineHeight;
	float							mSpaceWidth;
	bool							mGenerateIndices;

	void							run(TextLayout::Input&, TextLayout&);

	// Adds a line to the output layout
	void							addLine(const FontPtr& font, const std::wstring& lineText);

	// Maps the x-position of each character in the input string
	void							addStringSegment(const FontPtr& font, const std::wstring& inputText);
	void							addBlankSegment();
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_TEXTLAYOUT_H
