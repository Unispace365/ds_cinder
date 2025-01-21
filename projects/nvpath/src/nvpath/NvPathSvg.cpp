/*
Copyright (c) 2023, Paul Houx Creative Coding - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable : 4996)

#include <cinder/Base64.h>
#include <cinder/ImageIo.h>
#include <cinder/Log.h>
#include <cinder/Text.h>
#include <cinder/Utilities.h>

#include <ds/util/string_util.h>

#include "nvpath/NvPathSvg.h"

using namespace std;
using namespace ci;

namespace nvpath::svg {

namespace {

	char charToLower(const char c) {
		if (c >= 'A' && c <= 'Z') return char(c + 32);
		return c;
	}

	char charToUpper(const char c) {
		if (c >= 'a' && c <= 'z') return char(c - 32);
		return c;
	}

	// float parseFloat(const char** sInOut) {
	//	char		  temp[256];
	//	unsigned char i = 0;
	//	const char*	  s = *sInOut;
	//	while (*s && (isspace(*s) || *s == ','))
	//		s++;
	//	if (!s) throw FloatParseExc();
	//	if (ds::isNumeric(*s)) {
	//		while (*s == '-' || *s == '+') {
	//			temp[i++] = *s;
	//			if (!i) throw FloatParseExc(); // buffer overflow
	//			s++;
	//		}
	//		bool parsingExponent  = false;
	//		bool startingExponent = false;
	//		bool seenDecimal	  = false;
	//		while (*s && (parsingExponent || (*s != '-' && *s != '+')) && ds::isNumeric(*s)) {
	//			startingExponent = false;
	//			if (*s == '.' && seenDecimal)
	//				break;
	//			else if (*s == '.')
	//				seenDecimal = true;
	//			temp[i++] = *s;
	//			if (!i) throw FloatParseExc(); // buffer overflow
	//			if (*s == 'e' || *s == 'E') {
	//				parsingExponent	 = true;
	//				startingExponent = true;
	//			} else
	//				parsingExponent = false;
	//			s++;
	//		}
	//		if (startingExponent) { // if we got a false positive on an exponent, for example due to "ex" or "em"
	//								// unit, back up one
	//			--i;
	//			--s;
	//		}
	//		temp[i]			  = 0;
	//		const auto result = float(strtod(temp, nullptr));
	//		*sInOut			  = s;
	//		return result;
	//	} else
	//		throw FloatParseExc();
	// }

	// parses float from comma-separated parenthetical list
	vector<float> parseFloatList(const char** c) {
		vector<float> result;
		while (**c && isspace(**c))
			(*c)++;
		if (**c != '(') return result; // failure
		(*c)++;
		do {
			result.push_back(ds::parseFloat(c));
			while (**c && (**c == ',' || isspace(**c)))
				(*c)++;
		} while (**c && **c != ')');

		// get rid of trailing closing paren
		if (**c) (*c)++;

		return result;
	}

	vector<Value> parseValueList(const char** c, bool requireParens = true) {
		vector<Value> result;
		while (**c && isspace(**c))
			(*c)++;
		if (requireParens) {
			if (**c != '(') return result; // failure
			(*c)++;
		}
		do {
			result.push_back(Value::parse(c));
			while (**c && (**c == ',' || isspace(**c)))
				(*c)++;
		} while (**c && **c != ')');

		// get rid of trailing closing paren
		if (requireParens && **c) (*c)++;

		return result;
	}

	vector<Value> readValueList(const std::string& s, bool requireParens = true) {
		const char* temp = s.c_str();
		return parseValueList(&temp, requireParens);
	}

	vector<Value> readValueList(const char* c, bool requireParens = true) {
		const char* temp = c;
		return parseValueList(&temp, requireParens);
	}

	Value readValue(const std::string& s, float minV, float maxV) {
		const char* temp   = s.c_str();
		Value		result = Value::parse(&temp);
		if (result.value() < minV) result = minV;
		if (result.value() > maxV) result = maxV;
		return result;
	}

	Value readValue(const std::string& s) {
		const char* temp   = s.c_str();
		const Value result = Value::parse(&temp);
		return result;
	}

	// breaks comma-separated list into strings, optionally strips single or double quotes; removes all leading and
	// trailing white space
	vector<string> readStringList(const std::string& s, bool stripQuotes = false) {
		vector<string> result = split(s, ",");
		for (auto& resultIt : result) {
			auto trimmed = trim(resultIt);
			if (stripQuotes) {
				trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), '"'), trimmed.end());
				trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), '\''), trimmed.end());
			}
			resultIt = trimmed;
		}

		return result;
	}

} // anonymous namespace

CapsStyle toCapsStyle(LineCap lineCap) {
	switch (lineCap) {
	case LINE_CAP_BUTT:
		return CapsStyle::FLAT;
	case LINE_CAP_ROUND:
		return CapsStyle::ROUND;
	case LINE_CAP_SQUARE:
		return CapsStyle::SQUARE;
	}

	return CapsStyle::DEFAULT;
}

JoinStyle toJoinStyle(LineJoin lineJoin) {
	switch (lineJoin) {
	case LINE_JOIN_BEVEL:
		return JoinStyle::BEVEL;
	case LINE_JOIN_MITER:
		return JoinStyle::MITER_REVERT;
	case LINE_JOIN_ROUND:
		return JoinStyle::ROUND;
	}

	return JoinStyle::DEFAULT;
}

SpreadMethod toSpreadMethod(std::string style) {
	style = trim(style);
	if (!asciiCaseCmp(style.c_str(), "reflect")) return SpreadMethod::REFLECT;
	if (asciiCaseCmp(style.c_str(), "repeat")) return SpreadMethod::REPEAT;
	return SpreadMethod::PAD;
}

CoordinateSpace toGradientUnits(std::string style) {
	style = trim(style);
	if (style == "userSpaceOnUse") return CoordinateSpace::USER_SPACE_ON_USE;
	return CoordinateSpace::OBJECT_BOUNDING_BOX;
}

////////////////////////////////////////////////////////////////////////////////////
// Renderer
void Renderer::setVisitor(const function<bool(const SvgNode&, Style*)>& visitor) {
	mVisitor = std::make_shared<function<bool(const SvgNode&, Style*)>>(visitor);
}

////////////////////////////////////////////////////////////////////////////////////
// Style
Style::Style() {
	clear();
}

Style::Style(const XmlTree& xml, const SvgNode* parent) {
	clear();

	// Make sure to parse the 'color' property before 'fill' or 'stroke',
	// so that 'fill="currentColor"' works. See: https://www.w3.org/TR/SVGTiny12/painting.html#ColorProperty
	if (xml.hasAttribute("color")) parseProperty("color", xml.getAttribute("color").getValue(), parent);

	for (const auto& attrib : xml.getAttributes()) {
		if (attrib.getName() == "class")
			parseClassAttribute(attrib.getValue(), parent);
		else if (attrib.getName() == "style")
			parseStyleAttribute(attrib.getValue(), parent);
		else
			parseProperty(attrib.getName(), attrib.getValue(), parent);
	}
}

Style Style::makeGlobalDefaults() {
	Style result;

	result.setColor(getColorDefault());
	result.setFill(getFillDefault());
	result.setStroke(getStrokeDefault());
	result.setFillOpacity(getFillOpacityDefault());
	result.setStrokeOpacity(getStrokeOpacityDefault());

	result.setStrokeWidth(getStrokeWidthDefault());
	result.setFillRule(getFillRuleDefault());
	result.setLineCap(getLineCapDefault());
	result.setLineJoin(getLineJoinDefault());
	result.setMiterLimit(getMiterLimitDefault());
	result.setDashArray(getDashArrayDefault());
	result.setDashOffset(getDashOffsetDefault());
	result.setStopColor(getStopColorDefault());
	result.setStopOpacity(getStopOpacityDefault());

	result.setFontFamilies(getFontFamiliesDefault());
	result.setFontSize(getFontSizeDefault());
	result.setFontWeight(getFontWeightDefault());
	result.setTextAnchor(getTextAnchorDefault());

	result.setVisible(true);
	result.setDisplayNone(false);

	return result;
}

void Style::clear() {
	mSpecifiesColor = false;
	mSpecifiesFill = mSpecifiesStroke = false;
	mSpecifiesOpacity = mSpecifiesFillOpacity = mSpecifiesStrokeOpacity = false;
	mOpacity															= 1.0f;
	mSpecifiesStrokeWidth												= false;
	mSpecifiesFillRule													= false;
	mSpecifiesLineCap													= false;
	mSpecifiesLineJoin													= false;
	mSpecifiesMiterLimit												= false;
	mSpecifiesDashArray													= false;
	mSpecifiesDashOffset												= false;
	mSpecifiesClipPath													= false;
	mSpecifiesStopColor													= false;
	mSpecifiesStopOpacity												= false;
	mSpecifiesFontFamilies = mSpecifiesFontSize = mSpecifiesFontWeight = mSpecifiesTextAnchor = false;
	mSpecifiesVisible																		  = false;
	mVisible																				  = true;
	mDisplayNone																			  = false;
	mClipPath																				  = nullptr;
}

const ColorA8u& Style::getColorDefault() {
	static ColorA8u sBlack(0, 0, 0, 255); // Default color depends on user agent.
	return sBlack;
}

const Paint& Style::getFillDefault() {
	static const Paint sPaintBlack = Paint(Color::black());
	return sPaintBlack;
}
const Paint& Style::getStrokeDefault() {
	static const Paint sPaintNone = Paint();
	return sPaintNone;
}

const std::vector<std::string>& Style::getFontFamiliesDefault() {
	static shared_ptr<vector<string>> sDefault;
	if (!sDefault) {
		sDefault = std::make_shared<vector<string>>();
		sDefault->push_back("Times New Roman"); // Most browsers use Times New Roman as the default font.
	}

	return *sDefault;
}

void Style::parseClassAttribute(const std::string& stylePropertyString, const SvgNode* parent) {
	// Find <style> tag in ancestors.
	const auto styles = dynamic_cast<const SvgStyles*>(parent->findTagInAncestors("style"));

	// Merge styles.
	if (styles) *this += styles->findStyle(stylePropertyString);
}

void Style::parseStyleAttribute(const std::string& stylePropertyString, const SvgNode* parent) {
	// separate into pairs based on semicolons
	const vector<string> valuePairs = split(stylePropertyString, ';');
	for (const auto& pair : valuePairs) {
		vector<string> valuePair = split(pair, ':');
		if (valuePair.size() != 2) continue;

		parseProperty(trim(valuePair[0]), trim(valuePair[1]), parent);
	}
}

bool Style::parseProperty(const std::string& key, const std::string& value, const SvgNode* parent) {
	if (key == "color") {
		mColor = parsePaint(value.c_str(), &mSpecifiesColor, parent).getColor();
		return true;
	} else if (key == "fill") {
		mFill = parsePaint(value.c_str(), &mSpecifiesFill, parent);
		return true;
	} else if (key == "stroke") {
		mStroke = parsePaint(value.c_str(), &mSpecifiesStroke, parent);
		return true;
	} else if (key == "opacity") {
		mOpacity		  = readValue(value, 0, 1).asUser();
		mSpecifiesOpacity = true;
		return true;
	} else if (key == "fill-opacity") {
		mFillOpacity		  = readValue(value, 0, 1).asUser();
		mSpecifiesFillOpacity = true;
		return true;
	} else if (key == "stroke-opacity") {
		mStrokeOpacity			= readValue(value, 0, 1).asUser();
		mSpecifiesStrokeOpacity = true;
		return true;
	} else if (key == "stroke-width") {
		if (value != "inherit") {
			mSpecifiesStrokeWidth = true;
			mStrokeWidth		  = float(strtod(value.c_str(), nullptr));
		}
		return true;
	} else if (key == "fill-rule") {
		if (value == "evenodd") {
			mSpecifiesFillRule = true;
			mFillRule		   = FILL_RULE_EVEN_ODD;
		} else if (value == "nonzero") {
			mSpecifiesFillRule = true;
			mFillRule		   = FILL_RULE_NONZERO;
		}
		return true;
	} else if (key == "stroke-linecap") {
		if (value == "butt") {
			mSpecifiesLineCap = true;
			mLineCap		  = LINE_CAP_BUTT;
		} else if (value == "round") {
			mSpecifiesLineCap = true;
			mLineCap		  = LINE_CAP_ROUND;
		} else if (value == "square") {
			mSpecifiesLineCap = true;
			mLineCap		  = LINE_CAP_SQUARE;
		}
		return true;
	} else if (key == "stroke-linejoin") {
		if (value == "miter") {
			mSpecifiesLineJoin = true;
			mLineJoin		   = LINE_JOIN_MITER;
		} else if (value == "round") {
			mSpecifiesLineJoin = true;
			mLineJoin		   = LINE_JOIN_ROUND;
		} else if (value == "bevel") {
			mSpecifiesLineJoin = true;
			mLineJoin		   = LINE_JOIN_BEVEL;
		}
		return true;
	} else if (key == "stroke-miterlimit") {
		if (value != "inherit") {
			mSpecifiesMiterLimit = true;
			mMiterLimit			 = float(atof(value.c_str())); // should be >= 1, otherwise error
		}
		return true;
	} else if (key == "stroke-dasharray") {
		if (value != "inherit") {
			mSpecifiesDashArray = true;
			mDashArray.clear();
			if (!(value == "none" || value.empty())) {
				const auto values = readValueList(value, false);
				for (const auto& val : values)
					mDashArray.push_back(val.asUser());
			}
		}
		return true;
	} else if (key == "stroke-dashoffset") {
		if (value != "inherit") {
			if (!(value == "none" || value.empty())) {
				mSpecifiesDashOffset = true;
				mDashOffset			 = Value::parse(value).asUser();
			}
		}
		return true;
	} else if (key == "clip-path") {
		if (value != "inherit") {
			if (!strncmp(value.c_str(), "url", 3)) {
				char		id[1024];
				const char* hash	   = strchr(value.c_str(), '#');
				const char* closeParen = strchr(value.c_str(), ')');
				if ((!closeParen) || (!hash) || (closeParen - hash >= 1024)) return false;
				strncpy(id, hash + 1, closeParen - hash - 1);
				id[closeParen - hash - 1] = 0;
				mSpecifiesClipPath		  = true;
				mClipPathId				  = id;
				return true;
			}
			return false;
		}
		return true;
	} else if (key == "stop-color") {
		if (value != "inherit") mStopColor = parsePaint(value.c_str(), &mSpecifiesStopColor, nullptr).getColor();
		return true;
	} else if (key == "stop-opacity") {
		if (value != "inherit") {
			mStopOpacity		  = readValue(value, 0, 1).asUser();
			mSpecifiesStopOpacity = true;
		}
		return true;
	} else if (key == "font-family") {
		mSpecifiesFontFamilies = true;
		setFontFamilies(readStringList(value, true));
		return true;
	} else if (key == "font-size") {
		if (!value.empty() && isdigit(value[0])) { // we don't parse something like font-size:medium
			mSpecifiesFontSize = true;
			setFontSize(readValue(value));
		}
		return true;
	} else if (key == "font-weight") {
		const string weightString = trim(value);
		if (isdigit(weightString[0])) {
			int v = strtol(weightString.c_str(), nullptr, 10);
			if (v > 900) v = 900;
			if (v < 100) v = 100;
			mFontWeight			 = FontWeight(static_cast<int>(WEIGHT_100) + ((v / 100) - 1));
			mSpecifiesFontWeight = true;
		} else if (asciiCaseEqual(weightString, "normal")) {
			mFontWeight			 = WEIGHT_NORMAL;
			mSpecifiesFontWeight = true;
		} else if (asciiCaseEqual(weightString, "bold")) {
			mFontWeight			 = WEIGHT_BOLD;
			mSpecifiesFontWeight = true;
		}
		return true;
	} else if (key == "text-anchor") {
		mSpecifiesTextAnchor = true;
		if (asciiCaseEqual(value, "start")) mTextAnchor = TEXT_ANCHOR_START;
		if (asciiCaseEqual(value, "middle"))
			mTextAnchor = TEXT_ANCHOR_MIDDLE;
		else if (asciiCaseEqual(value, "end"))
			mTextAnchor = TEXT_ANCHOR_END;
		else
			mSpecifiesTextAnchor = false;
		return true;
	} else if (key == "display") {
		// we can't handle most of the possibilities yet; only 'none'
		if (value == "none")
			mDisplayNone = true;
		else
			mDisplayNone = false;
		return true;
	} else if (key == "visibility") {
		if (value != "inherit") {
			mSpecifiesVisible = true;
			if (value == "hidden" || value == "collapse")
				mVisible = false;
			else
				mVisible = true;
		}
		return true;
	} else
		return false;
}

Paint Style::parsePaint(const char* value, bool* specified, const SvgNode* parent) const {
	*specified = false;
	while (*value && isspace(*value))
		value++;

	if (!*value) return {};

	if (!strncmp(value, "inherit", 7)) {
		*specified = false;
		return {};
	}

	if (!strncmp(value, "currentColor", 12)) {
		*specified = true;
		return Paint{parent->getColor()};
	}

	if (value[0] == '#') { // hex color
		uint32_t v = 0;
		if (strlen(value) > 4) {
			for (int c = 0; c < 6; ++c) {
				const char	   ch  = charToUpper(value[1 + c]);
				const uint32_t col = ch - ((ch > '9') ? ('A' - 10) : '0');
				v += col << ((5 - c) * 4);
			}
		} else { // 3-digit hex shorthand; double each digit
			for (int c = 0; c < 3; ++c) {
				const char	   ch  = charToUpper(value[1 + c]);
				const uint32_t col = ch - ((ch > '9') ? ('A' - 10) : '0');
				v += col << ((5 - (c * 2 + 0)) * 4);
				v += col << ((5 - (c * 2 + 1)) * 4);
			}
		}
		*specified = true;
		return Paint{ColorA8u(char(v >> 16), char(v >> 8) & 255, char(v) & 255, 255)};
	} else if (!strncmp(value, "none", 4)) {
		*specified = true;
		return {};
	} else if (!strncmp(value, "rgba", 4)) {
		const vector<Value> values = readValueList(value + 4);
		if (values.size() == 4) {
			*specified = true;
			return Paint{ColorA8u(uint8_t(values[0].asUser(255)), uint8_t(values[1].asUser(255)),
								  uint8_t(values[2].asUser(255)), uint8_t(255 * values[3].asUser(1)))};
		}
		*specified = false;
		return {};
	} else if (!strncmp(value, "rgb", 3)) {
		const vector<Value> values = readValueList(value + 3);
		if (values.size() == 3) {
			*specified = true;
			return Paint{ColorA8u(uint8_t(values[0].asUser(255)), uint8_t(values[1].asUser(255)),
								  uint8_t(values[2].asUser(255)), 255)};
		}
		*specified = false;
		return {};
	} else if (!strncmp(value, "url", 3)) {
		char		id[1024];
		const char* hash	   = strchr(value, '#');
		const char* closeParen = strchr(value, ')');
		if ((!closeParen) || (!hash) || (closeParen - hash >= 1024)) return {};
		strncpy(id, hash + 1, closeParen - hash - 1);
		id[closeParen - hash - 1] = 0;

		Paint result = parent->findPaintInAncestors(id);
		if ((closeParen + 1)) // Parse fallback color.
			result.setFallback(std::make_shared<Paint>(parsePaint(closeParen + 1, specified, parent)));

		*specified = true;

		return result;
	} else { // try to find color amongst named colors
		Color8u result = svgNameToRgb(value, specified);
		if (specified)
			return Paint{result};
		else
			return {};
	}
}

void Style::resolve(const SvgNode* node) const {
	if (mSpecifiesFill && mFill.needsResolve()) {
		mFill = node->findPaintInAncestors(mFill.getId());
	}
	if (mSpecifiesStroke && mStroke.needsResolve()) {
		mStroke = node->findPaintInAncestors(mStroke.getId());
	}
}

bool Style::operator==(const Style& other) const {
	if (mSpecifiesOpacity && !ds::approxEqual(mOpacity, other.mOpacity)) return false;
	if (mSpecifiesFillOpacity && !ds::approxEqual(mFillOpacity, other.mFillOpacity)) return false;
	if (mSpecifiesStrokeOpacity && !ds::approxEqual(mStrokeOpacity, other.mStrokeOpacity)) return false;
	if (mSpecifiesFill && mFill != other.mFill) return false;
	if (mSpecifiesStroke && mStroke != other.mStroke) return false;
	if (mSpecifiesStrokeWidth && !ds::approxEqual(mStrokeWidth, other.mStrokeWidth)) return false;
	if (mSpecifiesFillRule && mFillRule != other.mFillRule) return false;
	if (mSpecifiesLineCap && mLineCap != other.mLineCap) return false;
	if (mSpecifiesLineJoin && mLineJoin != other.mLineJoin) return false;
	if (mSpecifiesMiterLimit && !ds::approxEqual(mMiterLimit, other.mMiterLimit)) return false;
	if (mSpecifiesDashArray && mDashArray != other.mDashArray) return false;
	if (mSpecifiesDashOffset && !ds::approxEqual(mDashOffset, other.mDashOffset)) return false;
	if (mSpecifiesClipPath && mClipPathId != other.mClipPathId) return false;
	return true;
}

void Style::operator+=(const Style& other) {
	if (other.mSpecifiesOpacity) setOpacity(other.mOpacity);
	if (other.mSpecifiesFillOpacity) setFillOpacity(other.mFillOpacity);
	if (other.mSpecifiesStrokeOpacity) setStrokeOpacity(other.mStrokeOpacity);
	if (other.mSpecifiesFill) setFill(other.mFill);
	if (other.mSpecifiesStroke) setStroke(other.mStroke);
	if (other.mSpecifiesStrokeWidth) setStrokeWidth(other.mStrokeWidth);
	if (other.mSpecifiesFillRule) setFillRule(other.mFillRule);
	if (other.mSpecifiesLineCap) setLineCap(other.mLineCap);
	if (other.mSpecifiesLineJoin) setLineJoin(other.mLineJoin);
	if (other.mSpecifiesMiterLimit) setMiterLimit(other.mMiterLimit);
	if (other.mSpecifiesDashArray) setDashArray(other.mDashArray);
	if (other.mSpecifiesDashOffset) setDashOffset(other.mDashOffset);
	if (other.mSpecifiesClipPath) setClipPath(other.mClipPathId);
	mClipPath = other.mClipPath;
}

Style Style::operator+(const Style& other) const {
	Style result(*this);
	result += other;
	return result;
}

void Style::startRender(Renderer& renderer, const SvgNode* node) const {
	if (mSpecifiesFill) renderer.pushFill(mFill);
	if (mSpecifiesStroke) renderer.pushStroke(mStroke);
	if (mSpecifiesOpacity) {
		// if this node draws, we'll force both fill opacity and stroke opacity to be 'opacity'
		if (node->isDrawable()) {
			renderer.pushFillOpacity(mOpacity);
			renderer.pushStrokeOpacity(mOpacity);
		}
	} else {
		if (mSpecifiesFillOpacity) renderer.pushFillOpacity(mFillOpacity);
		if (mSpecifiesStrokeOpacity) renderer.pushStrokeOpacity(mStrokeOpacity);
	}
	if (mSpecifiesStrokeWidth) renderer.pushStrokeWidth(mStrokeWidth);
	if (mSpecifiesFillRule) renderer.pushFillRule(mFillRule);
	if (mSpecifiesLineCap) renderer.pushLineCap(mLineCap);
	if (mSpecifiesLineJoin) renderer.pushLineJoin(mLineJoin);
	if (mSpecifiesMiterLimit) renderer.pushMiterLimit(mMiterLimit);
	if (mSpecifiesDashArray) renderer.pushDashArray(mDashArray);
	if (mSpecifiesDashOffset) renderer.pushDashOffset(mDashOffset);
	if (mSpecifiesClipPath) {
		if (!mClipPath) mClipPath = node->getClipPath(*this);
		if (mClipPath->useObjectBoundingBox()) {
			// Calculate object space transform matrix.
			const auto bounds	 = node->getBoundingBox();
			const auto transform = scale(translate(mat3(), bounds.getUpperLeft()), bounds.getSize());

			// Apply matrix prior to rendering the clip path and restore afterwards.
			renderer.pushMatrix(transform);
			renderer.pushClipPath(*mClipPath);
			renderer.popMatrix();
		} else
			renderer.pushClipPath(*mClipPath);
	}
}

void Style::finishRender(Renderer& renderer, const SvgNode* node) const {
	if (mSpecifiesClipPath) renderer.popClipPath();
	if (mSpecifiesDashOffset) renderer.popDashOffset();
	if (mSpecifiesDashArray) renderer.popDashArray();
	if (mSpecifiesMiterLimit) renderer.popMiterLimit();
	if (mSpecifiesLineJoin) renderer.popLineJoin();
	if (mSpecifiesLineCap) renderer.popLineCap();
	if (mSpecifiesFillRule) renderer.popFillRule();
	if (mSpecifiesStrokeWidth) renderer.popStrokeWidth();
	if ((mSpecifiesOpacity && node->isDrawable()) || ((!mSpecifiesOpacity) && mSpecifiesStrokeOpacity))
		renderer.popStrokeOpacity();
	if ((mSpecifiesOpacity && node->isDrawable()) || ((!mSpecifiesOpacity) && mSpecifiesFillOpacity))
		renderer.popFillOpacity();
	if (mSpecifiesStroke) renderer.popStroke();
	if (mSpecifiesFill) renderer.popFill();
}

////////////////////////////////////////////////////////////////////////////////////
// Value
float Value::asUser(float percentOf, float dpi, float fontSize, float fontXHeight) const {
	switch (mUnit) {
	case USER:
	case PX:
		return mValue;
	case PERCENT:
		return mValue * percentOf / 100;
	case PT:
		return mValue * (dpi / 72);
	case PC:
		return mValue * 12 * (dpi / 72); // there are 12pts in a pica
	case MM:
		return mValue * dpi / 25.4f; // 25.4mm in an inch
	case CM:
		return mValue * dpi / 2.54f; // 2.54cm in an inch
	case INCH:
		return mValue * dpi;
	case EM:
		return mValue * fontSize;
	case EX:
		return mValue * fontXHeight;
	}

	return mValue;
}

float Value::asUser(const SvgDoc* doc, const Style& style) const {
	return asUser(100, doc->getDpi(), style.getFontSize().asUser(), style.getFontSize().asUser() * 7.0f / 12.0f);
}

float Value::asUserWidth(const SvgDoc* doc, const Style& style) const {
	return asUser(doc->getWidth(), doc->getDpi(), style.getFontSize().asUser(),
				  style.getFontSize().asUser() * 7.0f / 12.0f);
}

float Value::asUserHeight(const SvgDoc* doc, const Style& style) const {
	return asUser(doc->getHeight(), doc->getDpi(), style.getFontSize().asUser(),
				  style.getFontSize().asUser() * 7.0f / 12.0f);
}

// Reads the suffix and converts it to user units based on dpi
Value Value::parse(const char** sInOut) {
	float v = ds::parseFloat(sInOut);
	if (strncmp(*sInOut, "px", 2) == 0) {
		*sInOut += 2;
		return {v, PX};
	} else if (**sInOut == '%') {
		*sInOut += 1;
		return {v, PERCENT};
	} else if (strncmp(*sInOut, "pt", 2) == 0) {
		*sInOut += 2;
		return {v, PT};
	} else if (strncmp(*sInOut, "pc", 2) == 0) { // picas
		*sInOut += 2;
		return {v, PC};
	} else if (strncmp(*sInOut, "mm", 2) == 0) {
		*sInOut += 2;
		return {v, MM};
	} else if (strncmp(*sInOut, "cm", 2) == 0) {
		*sInOut += 2;
		return {v, CM};
	} else if (strncmp(*sInOut, "in", 2) == 0) {
		*sInOut += 2;
		return {v, INCH};
	} else if (strncmp(*sInOut, "em", 2) == 0) {
		*sInOut += 2;
		return {v, EM};
	} else if (strncmp(*sInOut, "ex", 2) == 0) {
		*sInOut += 2;
		return {v, EX};
	} else if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		return {0, AUTO};
	} else
		return {v, USER};
}

Value Value::parse(const std::string& s) {
	const char* temp = s.c_str();
	return parse(&temp);
}

////////////////////////////////////////////////////////////////////////////////////
// Node
SvgNode::SvgNode(SvgNode* parent, const XmlTree& xml)
  : mParent(parent)
  , mBoundingBoxCached(false)
  , mStyle(xml, this) {
	mUuid				= nextUuid();
	mSpecifiesTransform = false;
	mTag				= xml.getTag();
	mId					= xml["id"];
	if (xml.hasAttribute("transform")) {
		mSpecifiesTransform = true;
		mTransform			= parseTransform(xml["transform"]);
	} else
		mTransform = mat3();
}

const SvgDoc* SvgNode::getDoc() const {
	const SvgNode* parent = mParent;
	while (parent) {
		const SvgDoc* doc = dynamic_cast<const SvgDoc*>(parent);
		if (doc) return doc;

		parent = parent->mParent;
	}

	return nullptr;
}

string SvgNode::getDomPath() const {
	string		   result = mId;
	const SvgNode* parent = this;
	while (parent && parent->mParent) {
		parent = parent->mParent;
		result = parent->getId();
		result += string("/") + result;
	}

	return result;
}

const ColorA8u& SvgNode::getColor() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesColor())
		return style.getColor();
	else if (mParent)
		return mParent->getColor();
	else
		return Style::getColorDefault();
}

const Paint& SvgNode::getFill() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesFill())
		return style.getFill();
	else if (mParent)
		return mParent->getFill();
	else
		return Style::getFillDefault();
}

const Paint& SvgNode::getStroke() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesStroke())
		return style.getStroke();
	else if (mParent)
		return mParent->getStroke();
	else
		return Style::getStrokeDefault();
}

float SvgNode::getStrokeWidth() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesStrokeWidth())
		return style.getStrokeWidth();
	else if (mParent)
		return mParent->getStrokeWidth();
	else
		return Style::getStrokeWidthDefault();
}

float SvgNode::getOpacity() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesOpacity())
		return style.getOpacity();
	else if (mParent)
		return mParent->getOpacity();
	else
		return Style::getOpacityDefault();
}

float SvgNode::getFillOpacity() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesFillOpacity())
		return style.getFillOpacity();
	else if (mParent)
		return mParent->getFillOpacity();
	else
		return Style::getFillOpacityDefault();
}

float SvgNode::getStrokeOpacity() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesStrokeOpacity())
		return style.getStrokeOpacity();
	else if (mParent)
		return mParent->getStrokeOpacity();
	else
		return Style::getStrokeOpacityDefault();
}

FillRule SvgNode::getFillRule() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesFillRule())
		return style.getFillRule();
	else if (mParent)
		return mParent->getFillRule();
	else
		return Style::getFillRuleDefault();
}

LineCap SvgNode::getLineCap() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesLineCap())
		return style.getLineCap();
	else if (mParent)
		return mParent->getLineCap();
	else
		return Style::getLineCapDefault();
}

LineJoin SvgNode::getLineJoin() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesLineJoin())
		return style.getLineJoin();
	else if (mParent)
		return mParent->getLineJoin();
	else
		return Style::getLineJoinDefault();
}

float SvgNode::getMiterLimit() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesMiterLimit())
		return style.getMiterLimit();
	else if (mParent)
		return mParent->getMiterLimit();
	else
		return Style::getMiterLimitDefault();
}

const std::vector<float>& SvgNode::getDashArray() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesDashArray())
		return style.getDashArray();
	else if (mParent)
		return mParent->getDashArray();
	else
		return Style::getDashArrayDefault();
}

float SvgNode::getDashOffset() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesDashOffset())
		return style.getDashOffset();
	else if (mParent)
		return mParent->getDashOffset();
	else
		return Style::getDashOffsetDefault();
}

ColorA8u SvgNode::getStopColor() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesStopColor())
		return style.getStopColor();
	else if (mParent)
		return mParent->getStopColor();
	else
		return Style::getStopColorDefault();
}

float SvgNode::getStopOpacity() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesStopOpacity())
		return style.getStopOpacity();
	else if (mParent)
		return mParent->getStopOpacity();
	else
		return Style::getStopOpacityDefault();
}

const vector<string>& SvgNode::getFontFamilies() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesFontFamilies())
		return style.getFontFamilies();
	else if (mParent)
		return mParent->getFontFamilies();
	else
		return Style::getFontFamiliesDefault();
}

Value SvgNode::getFontSize() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesFontSize())
		return style.getFontSize();
	else if (mParent)
		return mParent->getFontSize();
	else
		return Style::getFontSizeDefault();
}

TextAnchor SvgNode::getTextAnchor() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesTextAnchor())
		return style.getTextAnchor();
	else if (mParent)
		return mParent->getTextAnchor();
	else
		return Style::getTextAnchorDefault();
}

bool SvgNode::isVisible() const {
	const auto& style = getStyle(); // Resolves style if needed.
	if (style.specifiesVisible())
		return style.isVisible();
	else if (mParent)
		return mParent->isVisible();
	else
		return true;
}

Paint SvgNode::parsePaint(const char* value, bool* specified, const SvgNode* parentNode) {
	*specified = false;
	while (*value && isspace(*value))
		value++;

	if (!*value) return {};

	if (!strncmp(value, "inherit", 7)) {
		*specified = false;
		return {};
	}

	if (!strncmp(value, "currentColor", 12)) {
		*specified = true;
		return Paint{parentNode->getColor()};
	}

	if (value[0] == '#') { // hex color
		uint32_t v = 0;
		if (strlen(value) > 4) {
			for (int c = 0; c < 6; ++c) {
				const char	   ch  = charToUpper(value[1 + c]);
				const uint32_t col = ch - ((ch > '9') ? ('A' - 10) : '0');
				v += col << ((5 - c) * 4);
			}
		} else { // 3-digit hex shorthand; double each digit
			for (int c = 0; c < 3; ++c) {
				const char	   ch  = charToUpper(value[1 + c]);
				const uint32_t col = ch - ((ch > '9') ? ('A' - 10) : '0');
				v += col << ((5 - (c * 2 + 0)) * 4);
				v += col << ((5 - (c * 2 + 1)) * 4);
			}
		}
		*specified = true;
		return Paint{ColorA8u(char(v >> 16), char(v >> 8) & 255, char(v) & 255, 255)};
	} else if (!strncmp(value, "none", 4)) {
		*specified = true;
		return {};
	} else if (!strncmp(value, "rgb", 3)) {
		const vector<Value> values = readValueList(value + 3);
		if (values.size() == 3) {
			*specified = true;
			return Paint{ColorA8u(uint8_t(values[0].asUser(255)), uint8_t(values[1].asUser(255)),
								  uint8_t(values[2].asUser(255)), 255)};
		}
		*specified = false;
		return {};
	} else if (!strncmp(value, "url", 3)) {
		char		id[1024];
		const char* hash	   = strchr(value, '#');
		const char* closeParen = strchr(value, ')');
		if ((!closeParen) || (!hash) || (closeParen - hash >= 1024)) return {};
		strncpy(id, hash + 1, closeParen - hash - 1);
		id[closeParen - hash - 1] = 0;
		*specified				  = true;
		return parentNode->findPaintInAncestors(id);
	} else { // try to find color amongst named colors
		Color8u result = svgNameToRgb(value, specified);
		if (specified)
			return Paint{result};
		else
			return {};
	}
}

mat3 SvgNode::parseTransform(const std::string& value) {
	const char* c = value.c_str();
	mat3		curMat;
	mat3		nextMat;
	while (parseTransformComponent(&c, &nextMat)) {
		curMat = curMat * nextMat;
	}
	return curMat;
}

bool SvgNode::parseTransformComponent(const char** c, mat3* result) {
	// skip leading whitespace
	while (**c && (isspace(**c) || (**c == ',')))
		(*c)++;

	mat3 m;
	if (!strncmp(*c, "scale", 5)) {
		*c += 5; // strlen( "scale" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 1) {
			m = scale(mat3(), vec2(v[0]));
		} else if (v.size() == 2) {
			m = scale(mat3(), vec2(v[0], v[1]));
		} else
			throw SvgTransformParseExc();
	} else if (!strncmp(*c, "translate", 9)) {
		*c += 9; // strlen( "translate" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 1)
			m = translate(mat3(), vec2(v[0], 0));
		else if (v.size() == 2) {
			m = translate(mat3(), vec2(v[0], v[1]));
		} else
			throw SvgTransformParseExc();
	} else if (!strncmp(*c, "rotate", 6)) {
		*c += 6; // strlen( "rotate" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 1) {
			const float a = toRadians(v[0]);
			m			  = rotate(mat3(), a);
		} else if (v.size() == 3) { // rotate around point
			const float a = toRadians(v[0]);
			const vec2	origin(v[1], v[2]);
			m = translate(mat3(), origin);
			m = rotate(m, a);
			m = translate(m, -origin);
		} else
			throw SvgTransformParseExc();
	} else if (!strncmp(*c, "matrix", 6)) {
		*c += 6; // strlen( "matrix" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 6)
			m = mat3(v[0], v[1], 0, v[2], v[3], 0, v[4], v[5], 1);
		else
			throw SvgTransformParseExc();
	} else if (!strncmp(*c, "skewX", 5)) {
		*c += 5; // strlen( "skewX" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 1) {
			const float a = toRadians(v[0]);
			m			  = shearY(mat3(), tan(a));
		} else
			throw SvgTransformParseExc();
	} else if (!strncmp(*c, "skewY", 5)) {
		*c += 5; // strlen( "skewY" );
		const vector<float> v = parseFloatList(c);
		if (v.size() == 1) {
			const float a = toRadians(v[0]);
			m			  = shearX(mat3(), tan(a));
		} else
			throw SvgTransformParseExc();
	} else
		return false;

	*result = m;
	return true;
}

// Parse a 'style' attribute searching for the key 'key', and returning its corresponding value or the empty string
// if not found
std::string SvgNode::findStyleValue(const std::string& styleString, const std::string& key) {
	const vector<string> valuePairs = split(styleString, ';');
	for (const auto& pair : valuePairs) {
		vector<string> valuePair = split(pair, ':');
		if (valuePair.size() != 2) continue;
		if (valuePair[0] == key) return valuePair[1];
	}
	return {};
}

void SvgNode::parseStyle(const XmlTree& xml) {
	mStyle = Style(xml, this);
}

Style SvgNode::calcInheritedStyle() const {
	Style result;
	result.setFill(getFill());
	result.setStroke(getStroke());
	result.setOpacity(getOpacity());
	result.setFillOpacity(getFillOpacity());
	result.setStrokeOpacity(getStrokeOpacity());
	result.setFillRule(getFillRule());
	result.setLineCap(getLineCap());
	result.setLineJoin(getLineJoin());
	result.setMiterLimit(getMiterLimit());
	result.setDashArray(getDashArray());
	result.setDashOffset(getDashOffset());
	result.setStrokeWidth(getStrokeWidth());
	result.setStopColor(getStopColor());
	result.setStopOpacity(getStopOpacity());
	result.setFontFamilies(getFontFamilies());
	result.setFontSize(getFontSize());
	return result;
}

const SvgClipPath* SvgNode::getClipPath() const {
	if (getStyle().specifiesClipPath())
		return dynamic_cast<const SvgClipPath*>(findInAncestors(getStyle().getClipPath()));

	return nullptr;
}

const SvgClipPath* SvgNode::getClipPath(const Style& style) const {
	if (style.specifiesClipPath()) return dynamic_cast<const SvgClipPath*>(findInAncestors(style.getClipPath()));

	return nullptr;
}

void SvgNode::render(Renderer& renderer) const {
	renderer.start();

	const Style style = calcInheritedStyle();
	if (mParent) renderer.pushMatrix(mParent->getTransformAbsolute());

	startRender(renderer, style);
	renderSelf(renderer);
	finishRender(renderer, style);

	renderer.finish();
}

void SvgNode::firstStartRender(Renderer& renderer) const {
	renderer.pushFill(getFill());
}

void SvgNode::startRender(Renderer& renderer, const Style& style) const {
	if (mSpecifiesTransform) renderer.pushMatrix(mTransform);
	renderer.pushStyle(style);
	style.startRender(renderer, this);
}

void SvgNode::finishRender(Renderer& renderer, const Style& style) const {
	style.finishRender(renderer, this);
	renderer.popStyle();
	if (mSpecifiesTransform) renderer.popMatrix();
}

const SvgNode* SvgNode::findInAncestors(const std::string& elementId) const {
	if (mId == elementId)
		return this;
	else if (mParent)
		return mParent->findInAncestors(elementId);
	else
		return nullptr;
}

Paint SvgNode::findPaintInAncestors(const std::string& paintName) const {
	const SvgNode* node = findInAncestors(paintName);
	if (!node) return Paint{paintName}; // Needs to be resolved later.

	if (typeid(SvgLinearGradient) == typeid(*node)) {
		const auto* linearGradient = static_cast<const SvgLinearGradient*>(node);
		return linearGradient->asPaint();
	} else if (typeid(SvgRadialGradient) == typeid(*node)) {
		const auto* radialGradient = static_cast<const SvgRadialGradient*>(node);
		return radialGradient->asPaint();
	} else
		return {};
}

const SvgNode* SvgNode::findTagInAncestors(const std::string& elementTag) const {
	if (mTag == elementTag)
		return this;
	else if (mParent)
		return mParent->findTagInAncestors(elementTag);
	else
		return nullptr;
}

mat3 SvgNode::getTransformAbsolute() const {
	mat3 result;
	if (mSpecifiesTransform)
		result = mTransform;
	else
		result = mat3();

	const SvgNode* parent = mParent;
	while (parent) {
		if (parent->specifiesTransform()) result = parent->getTransform() * result;
		parent = parent->getParent();
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Gradient
SvgGradient::SvgGradient(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	parse(xml);
}

void SvgGradient::parse(const XmlTree& xml) {
	std::string ref;
	if (xml.hasAttribute("xlink:href"))
		ref = xml.getAttributeValue<string>("xlink:href");
	else if (xml.hasAttribute("href"))
		ref = xml.getAttributeValue<string>("href");

	if (ref.size() > 1) {
		if (ref[0] == '#') {
			const string elementId		= ref.substr(1, string::npos);
			const auto*	 referencedGrad = dynamic_cast<const SvgGradient*>(findInAncestors(elementId));
			if (referencedGrad) {
				copyAttributesFrom(*referencedGrad);
			}
		}
	}

	for (XmlTree::ConstIter stopsIt = xml.begin("stop"); stopsIt != xml.end(); ++stopsIt) {
		mStops.emplace_back(this, *stopsIt);
	}
	if (xml.hasAttribute("gradientUnits"))
		mUseObjectBoundingBox = xml.getAttributeValue<string>("gradientUnits") != string("userSpaceOnUse");
	if (xml.hasAttribute("gradientTransform")) {
		mSpecifiesTransform = true;
		mTransform			= parseTransform(xml.getAttributeValue<string>("gradientTransform"));
	}
	if (xml.hasAttribute("spreadMethod")) {
		mSpecifiesSpreadMethod = true;
		mSpreadMethod		   = toSpreadMethod(xml.getAttributeValue<string>("spreadMethod"));
	}

	// See: https://svgwg.org/svg2-draft/pservers.html#GradientStops
	// "Each gradient offset value is required to be equal to or greater than the previous gradient stop's offset
	// value.
	//  If a given gradient stop's offset value is not equal to or greater than all previous offset values, then the
	//  offset value is adjusted to be equal to the largest of all previous offset values."
	if (!mStops.empty()) {
		float offset = mStops.front().offset;
		for (auto& stop : mStops) {
			offset = stop.offset = glm::max(offset, stop.offset);
		}
	}
}

void SvgGradient::copyAttributesFrom(const SvgGradient& rhs) {
	mStops				  = rhs.mStops;
	mUseObjectBoundingBox = rhs.mUseObjectBoundingBox;
	if (rhs.mSpecifiesTransform) {
		mSpecifiesTransform = true;
		mTransform			= rhs.mTransform;
	}
	if (rhs.mSpecifiesSpreadMethod) {
		mSpecifiesSpreadMethod = true;
		mSpreadMethod		   = rhs.mSpreadMethod;
	}
}

SvgGradient::Stop::Stop(const SvgNode* parent, const XmlTree& xml) {
	if (xml.hasAttribute("offset"))
		offset = Value::parse(xml.getAttributeValue<string>("offset"))
					 .asUser(1); // Percentages will be converted to decimals, where 100% = 1.0f
	if (xml.hasAttribute("stop-color"))
		color = parsePaint(xml.getAttributeValue<string>("stop-color").c_str(), &specifiesColor, parent).getColor();
	if (xml.hasAttribute("stop-opacity")) {
		const auto value = xml.getAttributeValue<string>("stop-opacity");
		if (value == "inherit") {
			specifiesOpacity = false;
		} else {
			specifiesOpacity = true;
			color.a			 = uint8_t(Value::parse(value).asUser() * 255);
		}
	}

	if (xml.hasAttribute("style")) {
		const string stopColorString = findStyleValue(xml.getAttributeValue<string>("style"), "stop-color");
		if (!stopColorString.empty()) color = parsePaint(stopColorString.c_str(), &specifiesColor, parent).getColor();
		const string stopOpacityString = findStyleValue(xml.getAttributeValue<string>("style"), "stop-opacity");
		if (!stopOpacityString.empty()) {
			color.a = uint8_t(Value::parse(stopOpacityString).asUser() * 255);
		}
	}

	// See: https://svgwg.org/svg2-draft/pservers.html#GradientStops
	// "Gradient offset values less than 0 (or less than 0%) are rounded up to 0%.
	//  Gradient offset values greater than 1 (or greater than 100%) are rounded down to 100%."
	offset = glm::clamp(offset, 0.0f, 1.0f);
}

Paint SvgGradient::asPaint() const {
	// See: https://svgwg.org/svg2-draft/pservers.html#GradientStops
	// "If no stops are defined, then painting shall occur as if 'none' were specified as the paint style."
	if (getStyle().isDisplayNone() || !getStyle().isVisible() || mStops.empty()) {
		return Paint{Paint::Type::NONE, getId()};
	}

	Paint result{getType(), getId()};
	auto& stops = result.getStops();

	stops.clear();
	for (const auto& stop : mStops) {
		if (stop.specifiesColor)
			stops.emplace_back(stop.offset, stop.color);
		else
			stops.emplace_back(stop.offset, getStopColor());

		// See: https://svgwg.org/svg2-draft/pservers.html#GradientStops
		// "The opacity value used for the gradient calculation is the product of the value of stop-opacity and
		// the opacity of the value of stop-color."
		auto& opacity = stops.back().color.a;
		if (stop.specifiesOpacity)
			opacity = opacity * uint8_t(stop.opacity * 255.0f) / 255;
		else
			opacity = opacity * uint8_t(getStopOpacity() * 255.0f) / 255;
	}

	result.setUseObjectBoundingBox(mUseObjectBoundingBox);

	if (mSpecifiesTransform) {
		result.setTransform(mTransform);
	}
	if (mSpecifiesSpreadMethod) {
		result.setSpreadMethod(mSpreadMethod);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// LinearGradient
SvgLinearGradient::SvgLinearGradient(SvgNode* parent, const XmlTree& xml)
  : SvgGradient(parent, xml) {
	parse(xml);
}

void SvgLinearGradient::parse(const XmlTree& xml) {
	std::string ref;
	if (xml.hasAttribute("xlink:href"))
		ref = xml.getAttributeValue<string>("xlink:href");
	else if (xml.hasAttribute("href"))
		ref = xml.getAttributeValue<string>("href");

	if (ref.size() > 1) {
		if (ref[0] == '#') {
			const string elementId		= ref.substr(1, string::npos);
			const auto*	 referencedGrad = dynamic_cast<const SvgLinearGradient*>(findInAncestors(elementId));
			if (referencedGrad) {
				copyAttributesFrom(*referencedGrad);
			}
		}
	}

	if (xml.hasAttribute("x1")) mX1 = Value::parse(xml.getAttributeValue<string>("x1"));
	if (xml.hasAttribute("y1")) mY1 = Value::parse(xml.getAttributeValue<string>("y1"));
	if (xml.hasAttribute("x2")) mX2 = Value::parse(xml.getAttributeValue<string>("x2"));
	if (xml.hasAttribute("y2")) mY2 = Value::parse(xml.getAttributeValue<string>("y2"));
}

void SvgLinearGradient::copyAttributesFrom(const SvgLinearGradient& rhs) {
	mX1 = rhs.mX1;
	mY1 = rhs.mY1;
	mX2 = rhs.mX2;
	mY2 = rhs.mY2;
}

Paint SvgLinearGradient::asPaint() const {
	Paint result = SvgGradient::asPaint();

	// Percentages will be converted to decimals, where 100% = 1.0f
	result.setCoords0(mX1.asUser(1), mY1.asUser(1));
	result.setCoords1(mX2.asUser(1), mY2.asUser(1));

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// RadialGradient
SvgRadialGradient::SvgRadialGradient(SvgNode* parent, const XmlTree& xml)
  : SvgGradient(parent, xml) {
	parse(xml);
}

void SvgRadialGradient::parse(const XmlTree& xml) {
	std::string ref;
	if (xml.hasAttribute("xlink:href"))
		ref = xml.getAttributeValue<string>("xlink:href");
	else if (xml.hasAttribute("href"))
		ref = xml.getAttributeValue<string>("href");

	if (ref.size() > 1) {
		if (ref[0] == '#') {
			const string elementId		= ref.substr(1, string::npos);
			const auto*	 referencedGrad = dynamic_cast<const SvgRadialGradient*>(findInAncestors(elementId));
			if (referencedGrad) {
				copyAttributesFrom(*referencedGrad);
			}
		}
	}

	if (xml.hasAttribute("cx")) mCx = Value::parse(xml.getAttributeValue<string>("cx"));
	if (xml.hasAttribute("cy")) mCy = Value::parse(xml.getAttributeValue<string>("cy"));
	if (xml.hasAttribute("r")) mR = Value::parse(xml.getAttributeValue<string>("r"));
	if (xml.hasAttribute("fx"))
		mFx = Value::parse(xml.getAttributeValue<string>("fx"));
	else
		mFx = mCx;
	if (xml.hasAttribute("fy"))
		mFy = Value::parse(xml.getAttributeValue<string>("fy"));
	else
		mFy = mCy;
	if (xml.hasAttribute("fr")) mFr = Value::parse(xml.getAttributeValue<string>("fr"));
}

void SvgRadialGradient::copyAttributesFrom(const SvgRadialGradient& rhs) {
	mCx = rhs.mCx;
	mCy = rhs.mCy;
	mR	= rhs.mR;
	mFx = rhs.mFx;
	mFy = rhs.mFy;
	mFr = rhs.mFr;
}

Paint SvgRadialGradient::asPaint() const {
	Paint result = SvgGradient::asPaint();

	// Percentages will be converted to decimals, where 100% = 1.0f
	result.setCoords0(mCx.asUser(1), mCy.asUser(1));
	result.setCoords1(mFx.asUser(1), mFy.asUser(1));
	result.setRadius0(mR.asUser(1));
	result.setRadius1(mFr.asUser(1));

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Svg
Svg::Svg(const DataSourceRef& src)
  : Svg(SvgDoc::create(src)) {}

Svg::Svg(const SvgDocRef& svg)
  : mDoc(svg)
  , mBounds(svg->getBounds()) {}

const Path* Svg::findPath(size_t uuid) const {
	if (mPaths.count(static_cast<GLuint>(uuid))) return &mPaths.at(static_cast<GLuint>(uuid));

	return nullptr;
}

const Path* Svg::insertPath(size_t uuid, const Path& path, bool isClipPath) {
	if (!findPath(uuid)) {
		mPaths.insert_or_assign(static_cast<GLuint>(uuid), path);

		// Set path parameters.
		if (!isClipPath) {
			const auto& path = mPaths.at(static_cast<GLuint>(uuid));
			path.setMiterLimit(mStacks.miterLimit.back());
			path.setDashPattern(mStacks.dashArray.back());
			path.setDashOffset(mStacks.dashOffset.back());
			path.setEndCaps(toCapsStyle(mStacks.lineCap.back()));
			path.setDashCaps(toCapsStyle(mStacks.lineCap.back()), toCapsStyle(mStacks.lineCap.back()));
			path.setJoinStyle(toJoinStyle(mStacks.lineJoin.back()));
			path.setStrokeWidth(mStacks.strokeWidth.back());
		}
	}

	return &mPaths.at(static_cast<GLuint>(uuid));
}

const Path* Svg::insertPath(size_t uuid, const std::string& path, bool isClipPath) {
	if (!findPath(uuid)) {
		mPaths.insert_or_assign(static_cast<GLuint>(uuid), Path(path));

		// Set path parameters.
		if (!isClipPath) {
			const auto& path = mPaths.at(static_cast<GLuint>(uuid));
			path.setMiterLimit(mStacks.miterLimit.back());
			path.setDashPattern(mStacks.dashArray.back());
			path.setDashOffset(mStacks.dashOffset.back());
			path.setEndCaps(toCapsStyle(mStacks.lineCap.back()));
			path.setDashCaps(toCapsStyle(mStacks.lineCap.back()), toCapsStyle(mStacks.lineCap.back()));
			path.setJoinStyle(toJoinStyle(mStacks.lineJoin.back()));
			path.setStrokeWidth(mStacks.strokeWidth.back());
		}
	}

	return &mPaths.at(static_cast<GLuint>(uuid));
}

const Path* Svg::insertPath(size_t uuid, const Path2d& path, bool isClipPath) {
	if (!findPath(uuid)) {
		mPaths.insert_or_assign(static_cast<GLuint>(uuid), Path(path));

		// Set path parameters.
		if (!isClipPath) {
			const auto& path = mPaths.at(static_cast<GLuint>(uuid));
			path.setMiterLimit(mStacks.miterLimit.back());
			path.setDashPattern(mStacks.dashArray.back());
			path.setDashOffset(mStacks.dashOffset.back());
			path.setEndCaps(toCapsStyle(mStacks.lineCap.back()));
			path.setDashCaps(toCapsStyle(mStacks.lineCap.back()), toCapsStyle(mStacks.lineCap.back()));
			path.setJoinStyle(toJoinStyle(mStacks.lineJoin.back()));
			path.setStrokeWidth(mStacks.strokeWidth.back());
		}
	}

	return &mPaths.at(static_cast<GLuint>(uuid));
}

const Path* Svg::insertPath(size_t uuid, const Shape2d& shape, bool isClipPath) {
	if (!findPath(uuid)) {
		mPaths.insert_or_assign(static_cast<GLuint>(uuid), Path(shape));

		// Set path parameters.
		if (!isClipPath) {
			const auto& path = mPaths.at(static_cast<GLuint>(uuid));
			path.setMiterLimit(mStacks.miterLimit.back());
			path.setDashPattern(mStacks.dashArray.back());
			path.setDashOffset(mStacks.dashOffset.back());
			path.setEndCaps(toCapsStyle(mStacks.lineCap.back()));
			path.setDashCaps(toCapsStyle(mStacks.lineCap.back()));
			path.setJoinStyle(toJoinStyle(mStacks.lineJoin.back()));
			path.setStrokeWidth(mStacks.strokeWidth.back());
		}
	}

	return &mPaths.at(static_cast<GLuint>(uuid));
}

void Svg::start() {
	assert(nullptr == mCtx);

	// Clear stacks.
	mStacks.defaults();

	// Keep track of OpenGL context.
	mCtx = gl::context();

	// Disable any shader.
	mCtx->pushGlslProg(nullptr);

	// Enable stencil buffer testing.
	mCtx->pushBoolState(GL_STENCIL_TEST, GL_TRUE);

	// Enable pre-multiplied alpha blending.
	mCtx->pushBoolState(GL_BLEND, GL_TRUE);
	mCtx->pushBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	// Disable sRGB correct rendering.
	mCtx->pushBoolState(GL_FRAMEBUFFER_SRGB, GL_FALSE);

	// Bind gradient texture.
	mPaints.bind(mCtx, 0);

	// Store current transformations.
	gl::pushModelView();
}

void Svg::finish() {
	assert(gl::context() == mCtx);

	// Restore current transformations.
	gl::popModelView();

	// Unbind gradient texture.
	mPaints.unbind(mCtx);

	// Restore sRGB correct rendering.
	mCtx->popBoolState(GL_FRAMEBUFFER_SRGB);

	// Restore blending.
	mCtx->popBlendFuncSeparate();
	mCtx->popBoolState(GL_BLEND);

	// Restore stencil buffer testing.
	mCtx->popBoolState(GL_STENCIL_TEST);

	// Restore shader.
	mCtx->popGlslProg();

	// Done.
	mCtx = nullptr;
}

void Svg::clear() {
	mPaints.clear();
	mTextures.clear();
	mPaths.clear();
	mPaints.clear();
}

void Svg::pushGroup(const SvgGroup& group, float opacity) {
	mStacks.groupOpacity.push_back(opacity);
}

void Svg::popGroup() {
	mStacks.groupOpacity.pop_back();
}

void Svg::pushClipPath(const SvgClipPath& clippath) {
	assert(gl::context() == mCtx);

	if (mStacks.clipPath.size() > 5) CI_LOG_W("Maximum number of nested clip-paths reached! Results are undefined.");

	// Only render clip path if visible.
	if (clippath.isVisible() && !clippath.isDisplayNone()) {
		// Store clip-path in cache.
		const Path* path = findPath(clippath.getUuid());
		if (!path) {
			// Obtain shape from clip-path, which is a merge of all shapes contained within.
			path = insertPath(clippath.getUuid(), clippath.getShape(), true);
		}

		/// <summary>
		/// Clip paths are handled as follows:
		///	  1. The path or group of paths are rendered to the stencil buffer using either non-zero or even-odd
		/// fill
		/// rule.
		///	  2. The lowest bits of the stencil are now set for all pixels that need to be covered.
		///	  3. We then cover the pixels without writing to the color buffer, replacing the stencil value with the
		/// highest bit (0x80) if the test is passed.
		///	  4. Repeat this for each nested clip path, but use the next highest bit (0x40, 0x20, etc.) instead.
		///	  5. When rendering the actual clipped content, render normally but only draw pixels if all clip bits
		/// are
		/// set.
		///	  6. We then reset the lowest clip bits by doing a cover with the appropriate stencil functions set.
		///	  7. At the end of each clip path, reset the corresponding clip bit.
		/// </summary>
		const GLuint pathId		 = path->getId();
		const GLuint clipMask	 = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask	 = clipMask - 1;
		const GLuint stencilMask = getStencilMask();

		// Render shape to stencil buffer to use it as a clip-path.
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

		ScopedColorMask	  scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.
		ScopedStencilMask scpStencilMask(coverMask | clipMask);					// Don't write to previous clip bits.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		gl::stencilFunc(GL_NOTEQUAL, static_cast<GLint>(clipMask), coverMask);

		gl::stencilFillPathNV(pathId, GL_COUNT_UP_NV,
							  coverMask & stencilMask); // Write path to LSB portion (step 1).

		gl::coverFillPathNV(pathId, GL_CONVEX_HULL_NV); // Convert LSB portion to clip bit (step 3).
	}

	//
	mStacks.clipPath.push_back(&clippath);
}

void Svg::popClipPath() {
	assert(gl::context() == mCtx);
	assert(!mStacks.clipPath.empty());

	// Generate draw call to reset the clip-path.
	const auto& clipPath = *mStacks.clipPath.back();
	if (clipPath.isVisible() && !clipPath.isDisplayNone()) {
		const Path* path = findPath(clipPath.getUuid());
		if (!path) {
			__debugbreak(); // Path should already be cached!
		}

		//
		const GLuint pathId	   = path->getId();
		const GLuint clipMask  = 0x80 >> (mStacks.clipPath.size() - 1);
		const GLuint coverMask = clipMask - 1;

		// Render shape to stencil buffer to use it as a clip-path.
		glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
		glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

		ScopedColorMask	  scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.
		ScopedStencilMask scpStencilMask(coverMask | clipMask);					// Don't write to previous clip bits.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		gl::coverFillPathNV(pathId, GL_CONVEX_HULL_NV); // Clear stencil buffer bits (step 7).
	}

	//
	mStacks.clipPath.pop_back();
}

void Svg::drawPath(const SvgPath& path) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(path.getUuid());
	if (!ptr) {
		ptr = insertPath(path.getUuid(), path.getShape());
	}

	render(*ptr);
}

void Svg::drawPolyline(const SvgPolyline& polyline) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(polyline.getUuid());
	if (!ptr) {
		ptr = insertPath(polyline.getUuid(), polyline.getShape());
	}

	render(*ptr);
}

void Svg::drawPolygon(const SvgPolygon& polygon) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(polygon.getUuid());
	if (!ptr) {
		ptr = insertPath(polygon.getUuid(), polygon.getShape());
	}

	render(*ptr);
}

void Svg::drawLine(const SvgLine& line) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(line.getUuid());
	if (!ptr) {
		ptr = insertPath(line.getUuid(), line.getShape());
	}

	render(*ptr);
}

void Svg::drawRect(const SvgRect& rect) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(rect.getUuid());
	if (!ptr) {
		ptr = insertPath(rect.getUuid(), rect.getShape());
	}

	render(*ptr);
}

void Svg::drawCircle(const SvgCircle& circle) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(circle.getUuid());
	if (!ptr) {
		ptr = insertPath(circle.getUuid(), circle.getShape());
	}

	render(*ptr);
}

void Svg::drawEllipse(const SvgEllipse& ellipse) {
	assert(gl::context() == mCtx);

	const Path* ptr = findPath(ellipse.getUuid());
	if (!ptr) {
		ptr = insertPath(ellipse.getUuid(), ellipse.getShape());
	}

	render(*ptr);
}

void Svg::drawImage(const SvgImage& image) {
	assert(gl::context() == mCtx);

	if (!shouldRender()) return;

	const Path* ptr = findPath(image.getUuid());
	if (!ptr) {
		ptr = insertPath(image.getUuid(), rectangle(image.getRect()));
	}

	//
	const GLuint pathId = ptr->getId();

	// Obtain image texture.
	if (!mTextures.count(static_cast<GLuint>(image.getUuid()))) {
		const auto svg = image.getSvg();
		if (svg) {
			// Render embedded SVG to texture.
			Svg renderer;

			gl::pushModelMatrix();
			gl::setModelMatrix(mat4());

			const Canvas canvas(static_cast<int>(svg->getWidth()), static_cast<int>(svg->getHeight()), 8, 16);
			canvas.bind();
			svg->render(renderer);
			canvas.unbind();

			gl::popModelMatrix();

			auto texture = canvas.getTexture();
			if (texture) mTextures.insert_or_assign(static_cast<GLuint>(image.getUuid()), texture);
		} else {
			const auto surface = image.getSurface();
			if (surface)
				mTextures.insert_or_assign(static_cast<GLuint>(image.getUuid()),
										   gl::Texture2d::create(*surface, gl::Texture2d::Format().loadTopDown(false)));
		}
	}

	const auto& texture = mTextures.at(static_cast<GLuint>(image.getUuid()));

	// Render image.
	gl::ScopedTextureBind scpImage(texture, 2);
	glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

	ScopedShader scpShader(Shader::Type::IMAGE);
	scpShader.setColor(ColorA::white());
	scpShader.setCoords(GL_PATH_OBJECT_BOUNDING_BOX_NV /* TODO support userSpaceOnUse */, image.getTextureMatrix());
	scpShader.uniform("image", 2);
	scpShader.uniform("opacity", image.getOpacity());

	if (!mStacks.clipPath.empty()) {
		// Render clipped image.
		const GLuint clipMask  = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask = clipMask - 1;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

		const GLuint mask = coverMask << 1 | 0x01;
		gl::stencilFunc(GL_LESS, static_cast<GLint>(~mask & 0xFF), 0xFF); // (step 5).
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		const GLuint stencilMask = getStencilMask();
		glStencilThenCoverFillPathNV(pathId, GL_COUNT_UP_NV, stencilMask & coverMask,
									 GL_CONVEX_HULL_NV); // (step 5).

		// Remove shape from stencil buffer (step 6).
		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		gl::coverFillPathNV(pathId, GL_CONVEX_HULL_NV);
	} else {
		const GLuint stencilMask = getStencilMask();
		gl::stencilFunc(GL_NOTEQUAL, 0, stencilMask);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		glStencilThenCoverFillPathNV(pathId, GL_COUNT_UP_NV, stencilMask, GL_CONVEX_HULL_NV);
	}
}

void Svg::pushMatrix(const mat3& m) {
	mStacks.matrix.push_back(mStacks.matrix.back() * m);
}

void Svg::popMatrix() {
	mStacks.matrix.pop_back();
}

void Svg::pushFill(const Paint& paint) {
	mStacks.fill.push_back(paint);

	// Render gradients to texture.
	mPaints.preparePaint(paint, 1, false);
}

void Svg::popFill() {
	mStacks.fill.pop_back();
}

void Svg::pushStroke(const Paint& paint) {
	mStacks.stroke.push_back(paint);

	// Render gradients to texture.
	mPaints.preparePaint(paint, 1, false);
}

void Svg::popStroke() {
	mStacks.stroke.pop_back();
}

void Svg::pushFillOpacity(float opacity) {
	mStacks.fillOpacity.push_back(opacity);
}

void Svg::popFillOpacity() {
	mStacks.fillOpacity.pop_back();
}

void Svg::pushStrokeOpacity(float opacity) {
	mStacks.strokeOpacity.push_back(opacity);
}

void Svg::popStrokeOpacity() {
	mStacks.strokeOpacity.pop_back();
}

void Svg::pushStrokeWidth(float x) {
	mStacks.strokeWidth.push_back(x);
}

void Svg::popStrokeWidth() {
	mStacks.strokeWidth.pop_back();
}

void Svg::pushFillRule(FillRule fillRule) {
	mStacks.fillRule.push_back(fillRule);
}

void Svg::popFillRule() {
	mStacks.fillRule.pop_back();
}

void Svg::pushLineCap(LineCap lineCap) {
	mStacks.lineCap.push_back(lineCap);
}

void Svg::popLineCap() {
	mStacks.lineCap.pop_back();
}

void Svg::pushLineJoin(LineJoin lineJoin) {
	mStacks.lineJoin.push_back(lineJoin);
}

void Svg::popLineJoin() {
	mStacks.lineJoin.pop_back();
}

void Svg::pushMiterLimit(float miterLimit) {
	mStacks.miterLimit.push_back(miterLimit);
}

void Svg::popMiterLimit() {
	mStacks.miterLimit.pop_back();
}

void Svg::pushDashArray(const std::vector<float>& dashArray) {
	mStacks.dashArray.push_back(dashArray);
}

void Svg::popDashArray() {
	mStacks.dashArray.pop_back();
}

void Svg::pushDashOffset(float dashOffset) {
	mStacks.dashOffset.push_back(dashOffset);
}

void Svg::popDashOffset() {
	mStacks.dashOffset.pop_back();
}

void Svg::pushTextPen(const vec2& vec2) {
	mStacks.textPen.push_back(vec2);
}

void Svg::popTextPen() {
	mStacks.textPen.pop_back();
}

void Svg::pushTextRotation(float x) {
	mStacks.textRotation.push_back(x);
}

void Svg::popTextRotation() {
	mStacks.textRotation.pop_back();
}

void Svg::render(const Path& path) {
	if (!shouldRender()) return;

	if (!mStacks.fill.back().isNone()) {
		// Vertical and horizontal lines don't have a bounding box, since they are one-dimensional,
		// even though the stroke-width makes it look like they should have a bounding box with non-zero width and
		// height.
		if (mStacks.fill.back().fallback() && ds::approxZero(path.getFillBounds().calcArea())) {
			if (!mStacks.fill.back().fallback()->isNone())
				fill(path.getId(), *mStacks.fill.back().fallback(),
					 mOpacity * mStacks.fillOpacity.back() * mStacks.calcGroupOpacity());
		} else
			fill(path.getId(), mStacks.fill.back(), mOpacity * mStacks.fillOpacity.back() * mStacks.calcGroupOpacity());
	}
	if (!mStacks.stroke.back().isNone()) {
		// Vertical and horizontal lines don't have a bounding box, since they are one-dimensional,
		// even though the stroke-width makes it look like they should have a bounding box with non-zero width and
		// height.
		if (mStacks.stroke.back().fallback() && ds::approxZero(path.getFillBounds().calcArea())) {
			if (!mStacks.stroke.back().fallback()->isNone())
				stroke(path.getId(), *mStacks.stroke.back().fallback(),
					   mOpacity * mStacks.strokeOpacity.back() * mStacks.calcGroupOpacity());
		} else
			stroke(path.getId(), mStacks.stroke.back(),
				   mOpacity * mStacks.strokeOpacity.back() * mStacks.calcGroupOpacity());
	}
}

void Svg::fill(GLuint pathId, const Paint& paint, float opacity) {
	glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

	//
	ScopedShader scpShader(mPaints.preparePaint(paint, opacity));

	//
	if (!mStacks.clipPath.empty()) {
		// Render clipped path.
		const GLuint clipMask  = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask = clipMask - 1;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

		const GLuint mask = coverMask << 1 | 0x01;
		gl::stencilFunc(GL_LESS, static_cast<GLint>(~mask & 0xFF), 0xFF); // (step 5).
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		const GLuint stencilMask = getStencilMask();
		glStencilThenCoverFillPathNV(pathId, GL_COUNT_UP_NV, stencilMask & coverMask,
									 GL_CONVEX_HULL_NV); // (step 5).

		// Remove path from stencil buffer (step 6).
		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		gl::coverFillPathNV(pathId, GL_CONVEX_HULL_NV);
	} else {
		const GLuint stencilMask = getStencilMask();
		gl::stencilFunc(GL_NOTEQUAL, 0, stencilMask);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		glStencilThenCoverFillPathNV(pathId, GL_COUNT_UP_NV, stencilMask, GL_CONVEX_HULL_NV);
	}
}

void Svg::stroke(GLuint pathId, const Paint& paint, float opacity) {
	glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

	//
	ScopedShader scpShader(mPaints.preparePaint(paint, opacity));

	if (!mStacks.clipPath.empty()) {
		// Render clipped path.
		const GLuint clipMask  = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask = clipMask - 1;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

		const GLuint mask = coverMask << 1 | 0x01;
		gl::stencilFunc(GL_LESS, static_cast<GLint>(~mask & 0xFF), 0xFF); // (step 5).
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glStencilThenCoverStrokePathNV(pathId, GL_COUNT_UP_NV, coverMask, GL_CONVEX_HULL_NV); // (step 5).

		// Remove path from stencil buffer (step 6).
		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		gl::coverStrokePathNV(pathId, GL_CONVEX_HULL_NV);
	} else {
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		glStencilThenCoverStrokePathNV(pathId, GL_COUNT_UP_NV, 0xFF, GL_CONVEX_HULL_NV);
	}
}

void Svg::fillInstanced(GLuint baseId, GLsizei count, const glm::mat3x2* transforms, const uint32_t* indices,
						const Paint& paint, float opacity) {
	glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

	//
	ScopedShader scpShader(mPaints.preparePaint(paint, opacity));

	//
	if (!mStacks.clipPath.empty()) {
		// Render clipped text.
		const GLuint clipMask  = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask = clipMask - 1;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

		const GLuint mask = coverMask << 1 | 0x01;
		gl::stencilFunc(GL_LESS, static_cast<GLint>(~mask & 0xFF), 0xFF); // (step 5).
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// (step 5).
		const GLuint stencilMask = getStencilMask();
		glStencilThenCoverFillPathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV,
											  stencilMask & coverMask, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV,
											  GL_AFFINE_2D_NV, reinterpret_cast<const GLfloat*>(transforms));

		// Remove text from stencil buffer (step 6).
		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		glStencilThenCoverFillPathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV,
											  stencilMask & coverMask, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV,
											  GL_AFFINE_2D_NV, reinterpret_cast<const GLfloat*>(transforms));
	} else {
		const GLuint stencilMask = getStencilMask();
		gl::stencilFunc(GL_NOTEQUAL, 0, stencilMask);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		glStencilThenCoverFillPathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV,
											  stencilMask, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV, GL_AFFINE_2D_NV,
											  reinterpret_cast<const GLfloat*>(transforms));
	}
}

void Svg::strokeInstanced(GLuint baseId, GLsizei count, const glm::mat3x2* transforms, const uint32_t* indices,
						  const Paint& paint, float opacity) {
	glMatrixLoadfEXT(GL_MODELVIEW, value_ptr(gl::getModelView()));
	glMatrixMult3x3fNV(GL_MODELVIEW, value_ptr(mStacks.matrix.back()));

	// Set stroke parameters for each glyph.
	if (!paint.isNone()) {
		const float scale = transforms[0][0][0]; // Assume uniform scaling and single font size per call.

		for (GLsizei i = 0; i < count; ++i) {
			const GLuint pathId = baseId + indices[i];
			gl::pathParameteriNV(pathId, GL_PATH_END_CAPS_NV, static_cast<GLint>(toCapsStyle(mStacks.lineCap.back())));
			gl::pathParameteriNV(pathId, GL_PATH_JOIN_STYLE_NV,
								 static_cast<GLint>(toJoinStyle(mStacks.lineJoin.back())));
			gl::pathParameterfNV(pathId, GL_PATH_STROKE_WIDTH_NV,
								 static_cast<GLfloat>(mStacks.strokeWidth.back() / scale));
			gl::pathParameterfNV(pathId, GL_PATH_MITER_LIMIT_NV, static_cast<GLfloat>(mStacks.miterLimit.back()));
			// TODO: add support for dashing?
		}
	}

	//
	ScopedShader scpShader(mPaints.preparePaint(paint, opacity));

	//
	if (!mStacks.clipPath.empty()) {
		// Render clipped text.
		const GLuint clipMask  = 0x80 >> mStacks.clipPath.size();
		const GLuint coverMask = clipMask - 1;

		ScopedStencilMask scpStencilMask(coverMask | clipMask); // Don't write to previous clip bits.

		const GLuint mask = coverMask << 1 | 0x01;
		gl::stencilFunc(GL_LESS, static_cast<GLint>(~mask & 0xFF), 0xFF); // (step 5).
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// (step 5).
		glStencilThenCoverStrokePathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV,
												coverMask, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV, GL_AFFINE_2D_NV,
												reinterpret_cast<const GLfloat*>(transforms));

		// Remove text from stencil buffer (step 6).
		ScopedColorMask scpColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write to color buffer.

		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
		gl::stencilFunc(GL_ALWAYS, static_cast<GLint>(clipMask), coverMask);

		glStencilThenCoverStrokePathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV,
												coverMask, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV, GL_AFFINE_2D_NV,
												reinterpret_cast<const GLfloat*>(transforms));
	} else {
		gl::stencilFunc(GL_NOTEQUAL, 0, 0xFF);
		gl::stencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		glStencilThenCoverStrokePathInstancedNV(count, GL_UNSIGNED_INT, indices, baseId, GL_PATH_FILL_MODE_NV, 0xFF,
												GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV, GL_AFFINE_2D_NV,
												reinterpret_cast<const GLfloat*>(transforms));
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Circle
SvgCircle::SvgCircle(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();
	if (xml.hasAttribute("cx")) mCenter.x = Value::parse(xml.getAttributeValue<string>("cx")).asUserWidth(doc, style);
	if (xml.hasAttribute("cy")) mCenter.y = Value::parse(xml.getAttributeValue<string>("cy")).asUserHeight(doc, style);

	const auto m = getTransformAbsolute();
	mRadius		 = Value::parse(xml.getAttributeValue<string>("r")).asUser(100 * m[0][0]); // Use absolute scale.
}

void SvgCircle::renderSelf(Renderer& renderer) const {
	if (mRadius > 0) // Zero-radius circles should never be drawn.
		renderer.drawCircle(*this);
}

Shape2d SvgCircle::getShape() const {
	Shape2d result;
	result.arc(mCenter, mRadius, 0, float(M_PI) * 2);
	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Ellipse
SvgEllipse::SvgEllipse(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();
	if (xml.hasAttribute("cx")) mCenter.x = Value::parse(xml.getAttributeValue<string>("cx")).asUserWidth(doc, style);
	if (xml.hasAttribute("cy")) mCenter.y = Value::parse(xml.getAttributeValue<string>("cy")).asUserHeight(doc, style);
	mRadiusX = Value::parse(xml.getAttributeValue<string>("rx")).asUserWidth(doc, style);
	mRadiusY = Value::parse(xml.getAttributeValue<string>("ry")).asUserHeight(doc, style);
}

void SvgEllipse::renderSelf(Renderer& renderer) const {
	if (mRadiusX > 0 && mRadiusY > 0) // Zero-radius ellipses should never be drawn.
		renderer.drawEllipse(*this);
}

bool SvgEllipse::containsPoint(const vec2& pt) const {
	const float x = (pt.x - mCenter.x) * (pt.x - mCenter.x) / (mRadiusX * mRadiusX);
	const float y = (pt.y - mCenter.y) * (pt.y - mCenter.y) / (mRadiusY * mRadiusY);
	return x + y < 1;
}

Shape2d SvgEllipse::getShape() const {
	Shape2d result;

	constexpr float magic = 0.552284749830793398402f; // 4/3*(sqrt(2)-1)
	const vec2		offset(mRadiusX * magic, mRadiusY * magic);

	result.moveTo(vec2(mCenter.x + mRadiusX, mCenter.y));
	result.curveTo(vec2(mCenter.x + mRadiusX, mCenter.y + offset.y), vec2(mCenter.x + offset.x, mCenter.y + mRadiusY),
				   vec2(mCenter.x, mCenter.y + mRadiusY));
	result.curveTo(vec2(mCenter.x - offset.x, mCenter.y + mRadiusY), vec2(mCenter.x - mRadiusX, mCenter.y + offset.y),
				   vec2(mCenter.x - mRadiusX, mCenter.y));
	result.curveTo(vec2(mCenter.x - mRadiusX, mCenter.y - offset.y), vec2(mCenter.x - offset.x, mCenter.y - mRadiusY),
				   vec2(mCenter.x, mCenter.y - mRadiusY));
	result.curveTo(vec2(mCenter.x + offset.x, mCenter.y - mRadiusY), vec2(mCenter.x + mRadiusX, mCenter.y - offset.y),
				   vec2(mCenter.x + mRadiusX, mCenter.y));
	result.close();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
//
void ellipticalArc(Shape2d& path, float x1, float y1, float x2, float y2, float rx, float ry, float xAxisRotation,
				   bool largeArcFlag, bool sweepFlag) {
	// This is a translation of the  spec section "Elliptical Arc Implementation Notes"
	// http://www.w3.org/TR//implnote.html#ArcImplementationNotes
	float	   cosXAxisRotation = cosf(xAxisRotation);
	float	   sinXAxisRotation = sinf(xAxisRotation);
	const vec2 cPrime(cosXAxisRotation * (x2 - x1) * 0.5f + sinXAxisRotation * (y2 - y1) * 0.5f,
					  -sinXAxisRotation * (x2 - x1) * 0.5f + cosXAxisRotation * (y2 - y1) * 0.5f);

	// http://www.w3.org/TR//implnote.html#ArcCorrectionOutOfRangeRadii
	float radiiScale = (cPrime.x * cPrime.x) / (rx * rx) + (cPrime.y * cPrime.y) / (ry * ry);
	if (radiiScale > 1) {
		radiiScale = math<float>::sqrt(radiiScale);
		rx *= radiiScale;
		ry *= radiiScale;
	}

	vec2 invRadius(1.0f / rx, 1.0f / ry);
	vec2 point1 =
		vec2(cosXAxisRotation * x1 + sinXAxisRotation * y1, -sinXAxisRotation * x1 + cosXAxisRotation * y1) * invRadius;
	vec2 point2 =
		vec2(cosXAxisRotation * x2 + sinXAxisRotation * y2, -sinXAxisRotation * x2 + cosXAxisRotation * y2) * invRadius;
	vec2  delta = point2 - point1;
	float d		= delta.x * delta.x + delta.y * delta.y;
	if (d <= 0) return;

	float theta1;
	float thetaDelta;
	vec2  center;

	float s = math<float>::sqrt(std::max<float>(1 / d - 0.25f, 0));
	if (sweepFlag == largeArcFlag) s = -s;

	center = vec2(0.5f * (point1.x + point2.x) - delta.y * s, 0.5f * (point1.y + point2.y) + delta.x * s);

	theta1		 = math<float>::atan2(point1.y - center.y, point1.x - center.x);
	float theta2 = math<float>::atan2(point2.y - center.y, point2.x - center.x);

	thetaDelta = theta2 - theta1;
	if (thetaDelta < 0 && sweepFlag)
		thetaDelta += 2 * float(M_PI);
	else if (thetaDelta > 0 && (!sweepFlag))
		thetaDelta -= 2 * float(M_PI);

	// divide the full arc delta into pi/2 arcs and convert those to cubic beziers
	int segments = int(ceilf(fabsf(thetaDelta / (float(M_PI) / 2))) + 1);
	for (int i = 0; i < segments; ++i) {
		float thetaStart	= theta1 + i * thetaDelta / segments;
		float thetaEnd		= theta1 + (i + 1) * thetaDelta / segments;
		float t				= (4 / 3.0f) * tanf(0.25f * (thetaEnd - thetaStart));
		float sinThetaStart = math<float>::sin(thetaStart);
		float cosThetaStart = math<float>::cos(thetaStart);
		float sinThetaEnd	= math<float>::sin(thetaEnd);
		float cosThetaEnd	= math<float>::cos(thetaEnd);

		vec2 startPoint			 = vec2(cosThetaStart - t * sinThetaStart, sinThetaStart + t * cosThetaStart) + center;
		startPoint				 = vec2(cosXAxisRotation * startPoint.x * rx - sinXAxisRotation * startPoint.y * ry,
										sinXAxisRotation * startPoint.x * rx + cosXAxisRotation * startPoint.y * ry);
		vec2 endPoint			 = vec2(cosThetaEnd, sinThetaEnd) + center;
		vec2 transformedEndPoint = vec2(cosXAxisRotation * endPoint.x * rx - sinXAxisRotation * endPoint.y * ry,
										sinXAxisRotation * endPoint.x * rx + cosXAxisRotation * endPoint.y * ry);
		vec2 midPoint			 = endPoint + vec2(t * sinThetaEnd, -t * cosThetaEnd);
		midPoint				 = vec2(cosXAxisRotation * midPoint.x * rx - sinXAxisRotation * midPoint.y * ry,
										sinXAxisRotation * midPoint.x * rx + cosXAxisRotation * midPoint.y * ry);
		path.curveTo(startPoint, midPoint, transformedEndPoint);
	}
}

static const char* getNextPathItem(const char* s, char it[64]) {
	int i = 0;
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (isspace(*s) || *s == ','))
		s++;
	if (!*s) return s;
	if (ds::isNumeric(*s)) {
		while (*s == '-' || *s == '+') {
			if (i < 63) it[i++] = *s;
			s++;
		}
		bool parsingExponent = false;
		while (*s && (parsingExponent || (*s != '-' && *s != '+')) && ds::isNumeric(*s)) {
			if (i < 63) it[i++] = *s;
			if (*s == 'e' || *s == 'E')
				parsingExponent = true;
			else
				parsingExponent = false;
			s++;
		}
		it[i] = '\0';
	} else {
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

char readNextCommand(const char** sInOut) {
	const char* s = *sInOut;
	while (*s && (isspace(*s) || *s == ','))
		s++;
	*sInOut = s + 1;
	return *s;
}

bool readFlag(const char** sInOut) {
	const char* s = *sInOut;
	while (*s && (isspace(*s) || *s == ',' || *s == '-' || *s == '+'))
		s++;
	*sInOut = s + 1;
	return *s != '0';
}

bool nextItemIsFloat(const char* s) {
	while (*s && (isspace(*s) || *s == ','))
		s++;
	return ds::isNumeric(*s);
}

Shape2d parsePath(const std::string& p) {
	const char* s = p.c_str();
	vec2		v0;
	vec2		v1;
	vec2		v2;
	vec2		lastPoint;
	vec2		lastPoint2;

	Shape2d result;
	try {
		bool done	  = false;
		bool firstCmd = true;
		char prevCmd  = '\0';
		while (!done) {
			char cmd = readNextCommand(&s);
			switch (cmd) {
			case 'm':
			case 'M':
				v0.x = ds::parseFloat(&s);
				v0.y = ds::parseFloat(&s);
				if ((!firstCmd) && (cmd == 'm')) v0 += lastPoint;
				result.moveTo(v0);
				lastPoint2 = lastPoint;
				lastPoint  = v0;
				while (nextItemIsFloat(s)) {
					v0.x = ds::parseFloat(&s);
					v0.y = ds::parseFloat(&s);
					if (cmd == 'm') v0 += lastPoint;
					result.lineTo(v0);
					lastPoint2 = lastPoint;
					lastPoint  = v0;
				}
				break;
			case 'l':
			case 'L':
				do {
					v0.x = ds::parseFloat(&s);
					v0.y = ds::parseFloat(&s);
					if (cmd == 'l') v0 += lastPoint;
					result.lineTo(v0);
					lastPoint2 = lastPoint;
					lastPoint  = v0;
				} while (nextItemIsFloat(s));
				break;
			case 'H':
			case 'h':
				do {
					float x = ds::parseFloat(&s);
					v0		= vec2((cmd == 'h') ? (lastPoint.x + x) : x, lastPoint.y);
					result.lineTo(v0);
					lastPoint2 = lastPoint;
					lastPoint  = v0;
				} while (nextItemIsFloat(s));
				break;
			case 'V':
			case 'v':
				do {
					float y = ds::parseFloat(&s);
					v0		= vec2(lastPoint.x, (cmd == 'v') ? (lastPoint.y + y) : (y));
					result.lineTo(v0);
					lastPoint2 = lastPoint;
					lastPoint  = v0;
				} while (nextItemIsFloat(s));
				break;
			case 'C':
			case 'c':
				do {
					v0.x = ds::parseFloat(&s);
					v0.y = ds::parseFloat(&s);
					v1.x = ds::parseFloat(&s);
					v1.y = ds::parseFloat(&s);
					v2.x = ds::parseFloat(&s);
					v2.y = ds::parseFloat(&s);
					if (cmd == 'c') { // relative
						v0 += lastPoint;
						v1 += lastPoint;
						v2 += lastPoint;
					}
					result.curveTo(v0, v1, v2);
					lastPoint2 = v1;
					lastPoint  = v2;
				} while (nextItemIsFloat(s));
				break;
			case 'S':
			case 's':
				do {
					if (prevCmd == 's' || prevCmd == 'S' || prevCmd == 'c' || prevCmd == 'C')
						v0 = lastPoint * 2.0f - lastPoint2;
					else
						v0 = lastPoint;
					prevCmd = cmd; // set this now in case we loop
					v1.x	= ds::parseFloat(&s);
					v1.y	= ds::parseFloat(&s);
					v2.x	= ds::parseFloat(&s);
					v2.y	= ds::parseFloat(&s);
					if (cmd == 's') { // relative
						v1 += lastPoint;
						v2 += lastPoint;
					}
					result.curveTo(v0, v1, v2);
					lastPoint2 = v1;
					lastPoint  = v2;
				} while (nextItemIsFloat(s));
				break;
			case 'Q':
			case 'q':
				do {
					v0.x = ds::parseFloat(&s);
					v0.y = ds::parseFloat(&s);
					v1.x = ds::parseFloat(&s);
					v1.y = ds::parseFloat(&s);
					if (cmd == 'q') { // relative
						v0 += lastPoint;
						v1 += lastPoint;
					}
					result.quadTo(v0, v1);
					lastPoint2 = v0;
					lastPoint  = v1;
				} while (nextItemIsFloat(s));
				break;
			case 'T':
			case 't':
				do {
					if (prevCmd == 't' || prevCmd == 'T' || prevCmd == 'q' || prevCmd == 'Q')
						v0 = lastPoint * 2.0f - lastPoint2;
					else
						v0 = lastPoint;
					prevCmd = cmd; // set this now in case we loop
					v1.x	= ds::parseFloat(&s);
					v1.y	= ds::parseFloat(&s);
					if (cmd == 't') { // relative
						v1 += lastPoint;
					}
					result.quadTo(v0, v1);
					lastPoint2 = v0;
					lastPoint  = v1;
				} while (nextItemIsFloat(s));
				break;
			case 'a':
			case 'A': {
				do {
					float ra			= ds::parseFloat(&s);
					float rb			= ds::parseFloat(&s);
					float xAxisRotation = ds::parseFloat(&s) * float(M_PI) / 180.0f;
					bool  largeArc		= readFlag(&s);
					bool  sweepFlag		= readFlag(&s);
					v0.x				= ds::parseFloat(&s);
					v0.y				= ds::parseFloat(&s);
					if (cmd == 'a') { // relative
						v0 += lastPoint;
					}
					ellipticalArc(result, lastPoint.x, lastPoint.y, v0.x, v0.y, ra, rb, xAxisRotation, largeArc,
								  sweepFlag);
					lastPoint2 = lastPoint;
					lastPoint  = v0;
				} while (nextItemIsFloat(s));
			} break;
			case 'z':
			case 'Z':
				result.close();
				lastPoint2 = lastPoint;
				lastPoint  = (result.empty() || result.getContours().back().empty())
								 ? vec2()
								 : result.getContours().back().getPoint(0);
				break;
			case '\0':
			default: // technically noise at the end of the string is acceptable according to the spec; see
					 // W3C_SVG_11/paths-data-18.svg
				done = true;
				break;
			}
			firstCmd = false;
			prevCmd	 = cmd;
		}
	} catch (...) {}

	//// TODO For consistency, make sure paths are defined in CCW order for filled sections, CW for holes.
	//// This is especially important when using instanced rendering.
	// bool isClockwise = result.getContour(0).calcClockwise();
	// if (isClockwise) result.reverse();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Path
SvgPath::SvgPath(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto p = xml.getAttributeValue<string>("d", "");
	if (!p.empty()) {
		mPath = parsePath(p);
	}
}

void SvgPath::appendShape2d(Shape2d* appendTo) const {
	for (const auto& contour : mPath.getContours()) {
		appendTo->appendContour(contour);
	}
}

void SvgPath::renderSelf(Renderer& renderer) const {
	renderer.drawPath(*this);
}

////////////////////////////////////////////////////////////////////////////////////
// Line
SvgLine::SvgLine(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();

	// If the attribute is not specified, the effect is as if a value of "0" were specified.
	if (xml.hasAttribute("x1")) mPoint1.x = Value::parse(xml.getAttributeValue<string>("x1")).asUserWidth(doc, style);
	if (xml.hasAttribute("y1")) mPoint1.y = Value::parse(xml.getAttributeValue<string>("y1")).asUserHeight(doc, style);
	if (xml.hasAttribute("x2")) mPoint2.x = Value::parse(xml.getAttributeValue<string>("x2")).asUserWidth(doc, style);
	if (xml.hasAttribute("y2")) mPoint2.y = Value::parse(xml.getAttributeValue<string>("y2")).asUserHeight(doc, style);
}

void SvgLine::renderSelf(Renderer& renderer) const {
	renderer.drawLine(*this);
}

Shape2d SvgLine::getShape() const {
	Shape2d result;
	result.moveTo(mPoint1);
	result.lineTo(mPoint2);
	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Rect
SvgRect::SvgRect(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();

	if (xml.hasAttribute("x"))
		mRect.x1 = Value::parse(xml["x"]).asUserWidth(doc, style);
	else
		mRect.x1 = 0;
	if (xml.hasAttribute("y"))
		mRect.y1 = Value::parse(xml["y"]).asUserHeight(doc, style);
	else
		mRect.y1 = 0;

	float width	 = 0;
	float height = 0;
	if (xml.hasAttribute("width")) width = Value::parse(xml["width"]).asUserWidth(doc, style);
	if (xml.hasAttribute("height")) height = Value::parse(xml["height"]).asUserHeight(doc, style);
	mRect.x2 = mRect.x1 + width;
	mRect.y2 = mRect.y1 + height;

	if (xml.hasAttribute("rx")) mRx = Value::parse(xml["rx"]);
	if (xml.hasAttribute("ry")) mRy = Value::parse(xml["ry"]);

	// See: https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/rx
	if (!xml.hasAttribute("rx")) mRx = mRy;
	if (!xml.hasAttribute("ry")) mRy = mRx;

	if (mRx.isPercent())
		mRx = Value(glm::clamp(mRx.asUser(), 0.0f, 50.0f), Value::PERCENT);
	else
		mRx = Value(glm::clamp(mRx.asUser(), 0.0f, 0.5f * width));
	if (mRy.isPercent())
		mRy = Value(glm::clamp(mRy.asUser(), 0.0f, 50.0f), Value::PERCENT);
	else
		mRy = Value(glm::clamp(mRy.asUser(), 0.0f, 0.5f * height));

	mBoundingBox = mRect;
}

void SvgRect::renderSelf(Renderer& renderer) const {
	if (mRect.getWidth() > 0 && mRect.getHeight() > 0) // Zero-width or height rectangles should never be drawn.
		renderer.drawRect(*this);
}

float SvgRect::getRx() const {
	if (mRx.isPercent())
		return mRx.asUser(1) * mRect.getWidth(); // Percentages will be converted to decimals, where 100% = 1.0f
	return mRx.asUser();
}

float SvgRect::getRy() const {
	if (mRy.isPercent())
		return mRy.asUser(1) * mRect.getHeight(); // Percentages will be converted to decimals, where 100% = 1.0f
	return mRy.asUser();
}

Shape2d SvgRect::getShape() const {
	Shape2d result;

	const float x1 = mRect.x1;
	const float y1 = mRect.y1;
	const float x2 = mRect.x2;
	const float y2 = mRect.y2;

	const float rx = getRx();
	const float ry = getRy();
	if (rx > 0 || ry > 0) {
		// Approximate rounded rectangle with a series of cubic bezier curves.
		constexpr float magic = 1.0f - 0.552284749830793398402f; // 1-(4/3*(sqrt(2)-1))
		result.moveTo(x1 + rx, y1);
		result.lineTo(x2 - rx, y1);
		result.curveTo(x2 - rx * magic, y1, x2, y1 + ry * magic, x2, y1 + ry);
		result.lineTo(x2, y2 - ry);
		result.curveTo(x2, y2 - ry * magic, x2 - rx * magic, y2, x2 - rx, y2);
		result.lineTo(x1 + rx, y2);
		result.curveTo(x1 + rx * magic, y2, x1, y2 - ry * magic, x1, y2 - ry);
		result.lineTo(x1, y1 + ry);
		result.curveTo(x1, y1 + ry * magic, x1 + rx * magic, y1, x1 + rx, y1);
		result.close();
	} else {
		result.moveTo(x1, y1);
		result.lineTo(x2, y1);
		result.lineTo(x2, y2);
		result.lineTo(x1, y2);
		result.close();
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Polygon
vector<vec2> parsePointList(const std::string& p) {
	vector<vec2> result;

	if (!p.empty()) {
		char		item[64];
		const char* s	= p.c_str();
		bool		odd = false;
		float		lastVal;
		while (*s) {
			s = getNextPathItem(s, item);
			if (!odd)
				lastVal = float(strtod(item, nullptr));
			else
				result.emplace_back(lastVal, float(strtod(item, nullptr)));
			odd = !odd;
		}
	}

	return result;
}

SvgPolygon::SvgPolygon(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	mPolyLine = PolyLine2f(parsePointList(xml.getAttributeValue<string>("points", "")));
	mPolyLine.setClosed(true);
}

void SvgPolygon::renderSelf(Renderer& renderer) const {
	renderer.drawPolygon(*this);
}

Shape2d SvgPolygon::getShape() const {
	Shape2d result;

	if (mPolyLine.getPoints().size() <= 1) return result;

	result.moveTo(mPolyLine.getPoints()[0]);
	for (auto ptIt = mPolyLine.getPoints().begin() + 1; ptIt != mPolyLine.getPoints().end(); ++ptIt)
		result.lineTo(*ptIt);

	result.close();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Polyline
SvgPolyline::SvgPolyline(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	mPolyLine = PolyLine2f(parsePointList(xml.getAttributeValue<string>("points", "")));
	mPolyLine.setClosed(false);
}

void SvgPolyline::renderSelf(Renderer& renderer) const {
	renderer.drawPolyline(*this);
}

Shape2d SvgPolyline::getShape() const {
	Shape2d result;

	if (mPolyLine.getPoints().size() <= 1) return result;

	result.moveTo(mPolyLine.getPoints()[0]);
	for (auto ptIt = mPolyLine.getPoints().begin() + 1; ptIt != mPolyLine.getPoints().end(); ++ptIt)
		result.lineTo(*ptIt);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// Group
SvgGroup::SvgGroup(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	SvgGroup::parse(xml);
}

SvgGroup::~SvgGroup() {
	for (const auto& child : mChildren)
		delete child;
}

void SvgGroup::parse(const XmlTree& xml) {
	if (!ds::approxEqual(getOpacity(), 1.0f))
		CI_LOG_W("Group '" << getId() << "' opacity of " << getOpacity()
						   << " is currently not supported. A work-around is provided, but results may vary.");

	for (XmlTree::ConstIter treeIt = xml.begin(); treeIt != xml.end(); ++treeIt) {
		SvgNode* node = create(this, *treeIt);
		if (node) mChildren.push_back(node);
	}

	if (xml.hasAttribute("clip-path")) {
		const auto value = xml.getAttributeValue<std::string>("clip-path");

		if (!strncmp(value.c_str(), "url", 3)) {
			char		id[1024];
			const char* hash	   = strchr(value.c_str(), '#');
			const char* closeParen = strchr(value.c_str(), ')');
			if ((closeParen) && (hash) && (closeParen - hash < 1024)) {
				strncpy(id, hash + 1, closeParen - hash - 1);
				id[closeParen - hash - 1] = 0;

				mStyle.setClipPath(id);
			}
		}
	}
}

SvgNode* SvgGroup::create(SvgNode* parent, const XmlTree& xml) {
	if (xml.getTag() == "clipPath") return new SvgClipPath(parent, xml);
	if (xml.getTag() == "defs") return new SvgDefs(parent, xml);
	if (xml.getTag() == "g") return new SvgGroup(parent, xml);
	if (xml.getTag() == "svg") return new SvgDoc(parent, xml);
	if (xml.getTag() == "path") return new SvgPath(parent, xml);
	if (xml.getTag() == "polygon") return new SvgPolygon(parent, xml);
	if (xml.getTag() == "polyline") return new SvgPolyline(parent, xml);
	if (xml.getTag() == "line") return new SvgLine(parent, xml);
	if (xml.getTag() == "rect") return new SvgRect(parent, xml);
	if (xml.getTag() == "circle") return new SvgCircle(parent, xml);
	if (xml.getTag() == "ellipse") return new SvgEllipse(parent, xml);
	if (xml.getTag() == "use") return new SvgUse(parent, xml);
	if (xml.getTag() == "image") return new SvgImage(parent, xml);
	if (xml.getTag() == "linearGradient") return new SvgLinearGradient(parent, xml);
	if (xml.getTag() == "radialGradient") return new SvgRadialGradient(parent, xml);
	if (xml.getTag() == "style") return new SvgStyles(parent, xml);
	if (xml.getTag() == "text") return new SvgText(parent, xml);

	// Treat <switch> tags as normal groups and parse their contents.
	if (xml.getTag() == "switch") {
		CI_LOG_W("The `switch` tag is currently not supported and will be treated as a normal group.");
		return new SvgGroup(parent, xml);
	}

	CI_LOG_W("The `" << xml.getTag() << "` tag is currently not supported or recognized.");

	return nullptr;
}

const SvgNode* SvgGroup::findNodeByIdContains(const std::string& idPartial, bool recurse) const {
	for (const auto& child : mChildren) {
		if (child->getId().find(idPartial) != string::npos) {
			return child;
		}
	}

	if (recurse) {
		for (const auto child : mChildren) {
			const auto group = dynamic_cast<SvgGroup*>(child);
			if (group) {
				const SvgNode* result = group->findNodeByIdContains(idPartial);
				if (result) return result;
			}
		}
	}

	return nullptr;
}

const SvgNode* SvgGroup::findNodeByTag(const std::string& tag, bool recurse) const {
	// see if any immediate children have tag 'tag'
	for (const auto child : mChildren) {
		if (child->getTag() == tag) {
			return child;
		}
	}

	// see if any groups contain children with tag 'tag'
	if (recurse) {
		for (const auto child : mChildren) {
			const auto group = dynamic_cast<SvgGroup*>(child);
			if (group) {
				const SvgNode* result = group->findNodeByTag(tag);
				if (result) return result;
			}
		}
	}

	return nullptr;
}

const SvgNode* SvgGroup::findNode(const std::string& id, bool recurse) const {
	// see if any immediate children are named 'id'
	for (const auto child : mChildren) {
		if (child->getId() == id) {
			return child;
		}
	}

	// see if any groups contain children named 'id'
	if (recurse) {
		for (const auto child : mChildren) {
			const auto group = dynamic_cast<SvgGroup*>(child);
			if (group) {
				const SvgNode* result = group->findNode(id);
				if (result) return result;
			}
		}
	}

	return nullptr;
}

SvgNode* SvgGroup::nodeUnderPoint(const vec2& absolutePoint, const mat3& parentInverseMatrix) const {
	mat3 invTransform = parentInverseMatrix;
	if (mSpecifiesTransform) invTransform = inverse(mTransform) * invTransform;
	const vec2 localPt = vec2(invTransform * vec3(absolutePoint, 1));

	for (auto nodeIt = mChildren.rbegin(); nodeIt != mChildren.rend(); ++nodeIt) {
		const auto group = dynamic_cast<SvgGroup*>(*nodeIt);
		if (group) {
			SvgNode* node = group->nodeUnderPoint(absolutePoint, invTransform);
			if (node) return node;
		} else {
			if ((*nodeIt)->specifiesTransform()) {
				mat3 childInvTransform = (*nodeIt)->getTransformInverse() * invTransform;
				if ((*nodeIt)->containsPoint(vec2(childInvTransform * vec3(absolutePoint, 1)))) return *nodeIt;
			} else if ((*nodeIt)->containsPoint(localPt))
				return *nodeIt;
		}
	}

	return nullptr;
}

const SvgNode* SvgGroup::findInAncestors(const std::string& elementId) const {
	if (elementId.empty()) return nullptr;

	const SvgNode* result;

	if (getId() == elementId)
		return this;
	else if ((result = findNode(elementId, true)) != nullptr)
		return result;
	else if (getParent())
		return getParent()->findInAncestors(elementId);
	else
		return nullptr;
}

const SvgNode* SvgGroup::findTagInAncestors(const std::string& elementTag) const {
	const SvgNode* result;

	if (getTag() == elementTag)
		return this;
	else if ((result = findNodeByTag(elementTag, true)) != nullptr)
		return result;
	else if (getParent())
		return getParent()->findTagInAncestors(elementTag);
	else
		return nullptr;
}

const SvgNode& SvgGroup::getChild(const std::string& id) const {
	const SvgNode* result = findNode(id, false);
	if (!result)
		throw SvgChildNotFoundExc(id);
	else
		return *result;
}

Shape2d SvgGroup::getMergedShape2d() const {
	Shape2d result;
	appendMergedShape2d(&result);
	return result;
}

void SvgGroup::appendMergedShape2d(Shape2d* appendTo) const {
	for (const auto child : mChildren) {
		const auto* group = dynamic_cast<const SvgGroup*>(child);
		if (group)
			group->appendMergedShape2d(appendTo);
		else
			appendTo->append(child->getShape().transformed(child->getTransform()));
	}
}

const SvgNode& SvgGroup::getChild(size_t index) const {
	const auto childIt = mChildren.begin();
	while (index) {
		--index;
		if (childIt == mChildren.end()) break;
	}

	if (childIt == mChildren.end()) throw SvgChildNotFoundExc("index " + to_string(index));

	return **childIt;
}

void SvgGroup::renderSelf(Renderer& renderer) const {
	renderer.pushGroup(*this, getStyle().getOpacity());

	for (auto child : mChildren) {
		Style style = child->getStyle();
		if (!renderer.visit(*child, &style)) continue;
		if (child->getStyle().isDisplayNone()) // display: none we don't even descend groups
			continue;
		if ((!child->isVisible()) &&
			(typeid(SvgGroup) != typeid(*child))) // if this isn't visible and isn't a group, just move along
			continue;
		child->startRender(renderer, style);
		child->renderSelf(renderer);
		child->finishRender(renderer, style);
	}

	renderer.popGroup();
}

Rectf SvgGroup::calcBoundingBox() const {
	bool  empty = true;
	Rectf result(0, 0, 0, 0);
	for (const auto child : mChildren) {
		Rectf childBounds = child->getBoundingBox().transformed(child->getTransform());
		// only use child area if it exists (text nodes return [0,0,0,0])
		if ((childBounds.getWidth() > 0) || (childBounds.getHeight() > 0)) {
			if (empty) {
				result = childBounds;
				empty  = false;
			} else {
				result.include(childBounds);
			}
		}
	}
	return result;
}

void SvgGroup::iterate(const std::function<void(SvgNode*)>& fn) {
	for (auto& child : mChildren) {
		fn(child);
		if (typeid(*child) == typeid(SvgGroup)) static_cast<SvgGroup*>(child)->iterate(fn);
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Use
SvgUse::SvgUse(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml)
  , mReferenced(nullptr) {
	parse(xml);
}

void SvgUse::parse(const XmlTree& xml) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();

	std::string ref;
	if (xml.hasAttribute("xlink:href"))
		ref = xml.getAttributeValue<string>("xlink:href");
	else if (xml.hasAttribute("href"))
		ref = xml.getAttributeValue<string>("href");

	vec2 translate{0};
	if (xml.hasAttribute("x")) {
		translate.x			= Value::parse(xml.getAttributeValue<std::string>("x")).asUserWidth(doc, style);
		mSpecifiesTransform = true;
	}
	if (xml.hasAttribute("y")) {
		translate.y			= Value::parse(xml.getAttributeValue<std::string>("y")).asUserHeight(doc, style);
		mSpecifiesTransform = true;
	}
	mTransform = glm::translate(mTransform, translate);

	if (ref.size() > 1) {
		if (ref[0] == '#') {
			const string elementId = ref.substr(1, string::npos);
			mReferenced			   = findInAncestors(elementId);
		}
	}
}

void SvgUse::renderSelf(Renderer& renderer) const {
	if (mReferenced) {
		Style style = mReferenced->getStyle();
		if (!renderer.visit(*mReferenced, &style)) return;
		mReferenced->startRender(renderer, style);
		mReferenced->renderSelf(renderer);
		mReferenced->finishRender(renderer, style);
	}
}

////////////////////////////////////////////////////////////////////////////////////
// PreserveAspectRatio
PreserveAspectRatio::PreserveAspectRatio(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

PreserveAspectRatio::PreserveAspectRatio(const char** sInOut) {
	parse(sInOut);
}

glm::mat3x2 PreserveAspectRatio::calcTransform(const Rectf& element, const Rectf& viewBox, bool normalized) const {
	// See: https://svgwg.org/svg2-draft/coords.html#ComputingAViewportsTransform
	glm::mat3x2 m32;

	if (viewBox.getWidth() > 0 && viewBox.getHeight() > 0) {
		m32[0][0] = element.getWidth() / viewBox.getWidth();		// scale-x
		m32[1][1] = element.getHeight() / viewBox.getHeight();		// scale-y
		if (align != ALIGN_NONE && meetOrSlice == MEET)				//
			m32[0][0] = m32[1][1] = glm::min(m32[0][0], m32[1][1]); //
		else if (align != ALIGN_NONE && meetOrSlice == SLICE)		//
			m32[0][0] = m32[1][1] = glm::max(m32[0][0], m32[1][1]); //
		m32[2][0] = element.x1 - (viewBox.x1 * m32[0][0]);			// translate-x
		m32[2][1] = element.y1 - (viewBox.y1 * m32[1][1]);			// translate-y

		if (align == ALIGN_X_MID_Y_MIN || align == ALIGN_X_MID_Y_MID || align == ALIGN_X_MID_Y_MAX)
			m32[2][0] += (element.getWidth() - viewBox.getWidth() * m32[0][0]) * 0.5f;
		else if (align == ALIGN_X_MAX_Y_MIN || align == ALIGN_X_MAX_Y_MID || align == ALIGN_X_MAX_Y_MAX)
			m32[2][0] += element.getWidth() - viewBox.getWidth() * m32[0][0];
		if (align == ALIGN_X_MIN_Y_MID || align == ALIGN_X_MID_Y_MID || align == ALIGN_X_MAX_Y_MID)
			m32[2][1] += (element.getHeight() - viewBox.getHeight() * m32[1][1]) * 0.5f;
		else if (align == ALIGN_X_MIN_Y_MAX || align == ALIGN_X_MID_Y_MAX || align == ALIGN_X_MAX_Y_MAX)
			m32[2][1] += element.getHeight() - viewBox.getHeight() * m32[1][1];
	}

	// Normalize.
	if (normalized && element.getWidth() > 0 && element.getHeight() > 0) {
		m32[0][0] = float(viewBox.getWidth()) * m32[0][0] / element.getWidth();
		m32[1][1] = float(viewBox.getHeight()) * m32[1][1] / element.getHeight();
		m32[2][0] /= element.getWidth();
		m32[2][1] /= element.getHeight();
	}

	return m32;
}

void PreserveAspectRatio::parse(const char** sInOut) {
	ds::skipSpace(sInOut);

	const auto a = ds::fetchWord(sInOut);
	if (!asciiCaseCmp(a.c_str(), "none"))
		align = ALIGN_NONE;
	else if (!asciiCaseCmp(a.c_str(), "xminymin"))
		align = ALIGN_X_MIN_Y_MIN;
	else if (!asciiCaseCmp(a.c_str(), "xmidymin"))
		align = ALIGN_X_MID_Y_MIN;
	else if (!asciiCaseCmp(a.c_str(), "xmaxymin"))
		align = ALIGN_X_MAX_Y_MIN;
	else if (!asciiCaseCmp(a.c_str(), "xminymid"))
		align = ALIGN_X_MIN_Y_MID;
	else if (!asciiCaseCmp(a.c_str(), "xmidymid"))
		align = ALIGN_X_MID_Y_MID;
	else if (!asciiCaseCmp(a.c_str(), "xmaxymid"))
		align = ALIGN_X_MAX_Y_MID;
	else if (!asciiCaseCmp(a.c_str(), "xminymax"))
		align = ALIGN_X_MIN_Y_MIN;
	else if (!asciiCaseCmp(a.c_str(), "xmidymax"))
		align = ALIGN_X_MID_Y_MAX;
	else if (!asciiCaseCmp(a.c_str(), "xmaxymax"))
		align = ALIGN_X_MAX_Y_MAX;

	ds::skipSpace(sInOut);

	const auto m = ds::fetchWord(sInOut);
	if (!asciiCaseCmp(m.c_str(), "meet"))
		meetOrSlice = MEET;
	else if (!asciiCaseCmp(m.c_str(), "slice"))
		meetOrSlice = SLICE;
}

////////////////////////////////////////////////////////////////////////////////////
// Image
SvgImage::SvgImage(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml)
  , mBounds(0, 0, 0, 0) {
	const auto doc	 = getDoc();
	const auto style = calcInheritedStyle();

	if (xml.hasAttribute("x")) mBounds.x1 = Value::parse(xml.getAttributeValue<string>("x")).asUserWidth(doc, style);
	if (xml.hasAttribute("y")) mBounds.y1 = Value::parse(xml.getAttributeValue<string>("y")).asUserHeight(doc, style);
	if (xml.hasAttribute("width"))
		mBounds.x2 = mBounds.x1 + Value::parse(xml.getAttributeValue<string>("width")).asUserWidth(doc, style);
	if (xml.hasAttribute("height"))
		mBounds.y2 = mBounds.y1 + Value::parse(xml.getAttributeValue<string>("height")).asUserHeight(doc, style);

	std::string ref;
	if (xml.hasAttribute("xlink:href"))
		ref = xml.getAttributeValue<string>("xlink:href");
	else if (xml.hasAttribute("href"))
		ref = xml.getAttributeValue<string>("href");

	if (ref.find("data:") == 0) {
		parseDataImage(ref);
	} else if (!ref.empty()) {
		const auto ext = fs::path(ref).extension().string();
		if (ext == ".svg") {
			const auto path = doc->getFilePath() / ref;
			mSvg			= SvgDoc::create(this, loadFile(path), path);
		} else
			mImage = doc->loadImage(ref);
	}

	// Calculate texture transform matrix.
	const Rectf element(0, 0, mBounds.getWidth(), mBounds.getHeight());
	const Rectf viewBox = mImage ? Rectf(mImage->getBounds()) : mSvg ? mSvg->getBounds() : Rectf{};
	if (xml.hasAttribute("preserveAspectRatio"))
		mTextureMatrix = PreserveAspectRatio(xml.getAttributeValue<string>("preserveAspectRatio"))
							 .calcTransform(element, viewBox, true);
	else
		mTextureMatrix = PreserveAspectRatio().calcTransform(element, viewBox, true);


	if (xml.hasAttribute("clip-path")) {
		const auto value = xml.getAttributeValue<std::string>("clip-path");

		if (!strncmp(value.c_str(), "url", 3)) {
			char		id[1024];
			const char* hash	   = strchr(value.c_str(), '#');
			const char* closeParen = strchr(value.c_str(), ')');
			if (closeParen && hash && closeParen - hash < 1024) {
				strncpy(id, hash + 1, closeParen - hash - 1);
				id[closeParen - hash - 1] = 0;

				mStyle.setClipPath(id);
			}
		}
	}
}

bool SvgImage::parseDataImage(const string& data) {
	mImage.reset();
	mSvg.reset();

	const size_t dataOffset = data.find("data:") + 5;
	const size_t semi		= data.find(';');
	const size_t comma		= data.find(',');
	if (semi == string::npos || comma == string::npos) return false;

	const size_t len = data.size() - comma - 1;
	const auto	 buf = make_shared<Buffer>(fromBase64(&data[comma + 1], len));

	const string mime = data.substr(dataOffset, semi - dataOffset);
	if (mime == "image/svg+xml") {
		// See also: https://www.w3.org/TR/SVG2/embedded.html#ImageElement
		mSvg = SvgDoc::createFromSvgz(DataSourceBuffer::create(buf));

		// To prevent breaking changes, use a placeholder image.
		unsigned char bytes[4] = {255, 0, 0, 255};
		mImage				   = std::make_shared<Surface8u>(bytes, 1, 1, 4, SurfaceChannelOrder::RGBA);

		return true;
	} else {
		string extension;
		if (mime == "image/png")
			extension = "png";
		else if (mime == "image/jpeg")
			extension = "jpeg";

		try {
			mImage = std::make_shared<Surface8u>(
				loadImage(DataSourceBuffer::create(buf), ImageSource::Options(), extension));
			return true;
		} catch (std::exception& exc) {
			CI_LOG_W("failed to parse data image, what: " << exc.what());
		}
	}
	return false;
}

void SvgImage::renderSelf(Renderer& renderer) const {
	if (mImage || mSvg) renderer.drawImage(*this);
}

////////////////////////////////////////////////////////////////////////////////////
// Text
SvgText::SvgText(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml)
  , mAttributes(xml) {
	for (XmlTree::ConstIter treeIt = xml.begin(); treeIt != xml.end(); ++treeIt) {
		if (treeIt->getTag().empty()) { // data!
			mSpans.push_back(std::make_shared<SvgTextSpan>(this, treeIt->getValue()));
		} else if (treeIt->getTag() == "tspan") { // tspan!
			mSpans.push_back(std::make_shared<SvgTextSpan>(this, *treeIt));
		}
	}
}

#if 0 
Shape2d Text::getShape() const
{
	Shape2d result;	
	for( vector<TextSpanRef>::const_iterator spanIt = mSpans.begin(); spanIt != mSpans.end(); ++spanIt )
		result.append( (*spanIt)->getShape() );
	
	return result;
}
#endif

vec2 SvgText::getTextPen() const {
	if (!mAttributes.mX.isSet() || !mAttributes.mY.isSet()) {
		return {};
	}

	return {mAttributes.mX.asUser(), mAttributes.mY.asUser()};
}

float SvgText::getRotation() const {
	if (mAttributes.mRotate.size() != 1) {
		return 0;
	} else
		return mAttributes.mRotate[0].asUser();
}

Value SvgText::getLetterSpacing() const {
	if (mAttributes.mLetterSpacing.size() != 1) {
		return {0};
	} else
		return mAttributes.mLetterSpacing[0];
}

void SvgText::renderSelf(Renderer& renderer) const {
	renderer.pushTextPen(vec2()); // this may be overridden by the attributes, but that's ok
	mAttributes.startRender(renderer);
	for (const auto& span : mSpans) {
		span->renderSelf(renderer);
	}
	mAttributes.finishRender(renderer);
	renderer.popTextPen();
}

////////////////////////////////////////////////////////////////////////////////////
// TextSpan
SvgTextSpan::SvgTextSpan(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml)
  , mIgnoreAttributes(false)
  , mAttributes(xml)
  , mFont(nullptr) {
	for (XmlTree::ConstIter treeIt = xml.begin(); treeIt != xml.end(); ++treeIt) {
		if (treeIt->getTag().empty()) { // data!
			mSpans.push_back(std::make_shared<SvgTextSpan>(this, treeIt->getValue()));
		} else if (treeIt->getTag() == "tspan") { // tspan!
			mSpans.push_back(std::make_shared<SvgTextSpan>(this, *treeIt));
		}
	}
}

SvgTextSpan::SvgTextSpan(SvgNode* parent, const std::string& spanString)
  : SvgNode(parent)
  , mIgnoreAttributes(true)
  , mFont(nullptr) {
	// replace all multi-char whitespace with single space
	/*size_t c = 0;
	while( str[c] ) {
		if( isspace(str[c]) ) mString += ' ';
		while( str[c] && isspace(str[c]) )
			nextCharUtf8( str.c_str(), &c );
		while( str[c] && ( ! isspace(str[c]) ) ) // this is not really correct - does not work with UTF32 code
	points that are >255 mString += nextCharUtf8( str.c_str(), &c );
	}*/
	// Technically multi-char whitespace should be reduced to single chars; needs to be revisited with unicode-aware
	// version of this method
	mString = spanString;
}

void SvgTextSpan::renderSelf(Renderer& renderer) const {
	// Style style = getStyle(); // Resolves style if needed.
	// if( !renderer.visit( *this, &style ) )
	//	return;
	// startRender( renderer, style );
	// if( !mIgnoreAttributes ) // TextSpans that are actually the contents of Text's attributes should be ignored
	//	mAttributes.startRender( renderer );
	if (!mString.empty()) {
		renderer.drawTextSpan(*this);
	}
	for (const auto& span : mSpans) {
		span->renderSelf(renderer);
	}
	// if( !mIgnoreAttributes )
	//	mAttributes.finishRender( renderer );
	// finishRender( renderer, style );
}

std::vector<std::pair<uint16_t, vec2>> SvgTextSpan::getGlyphMeasures() const {
	if (!mGlyphMeasures) {
		TextBox tbox = TextBox().font(*getFont()).text(mString);
#if defined(CINDER_ANDROID) || defined(CINDER_LINUX)
		auto tmpGlyphs = tbox.measureGlyphs();
		mGlyphMeasures = shared_ptr<std::vector<std::pair<uint16_t, vec2>>>(
			new std::vector<std::pair<uint16_t, vec2>>(tmpGlyphs.size()));
		for (size_t i = 0; i < tmpGlyphs.size(); ++i) {
			const auto& src = tmpGlyphs[i];
			auto&		dst = (*mGlyphMeasures)[i];
			dst.first		= (uint16_t)src.first;
			dst.second		= src.second;
		}
#else
		mGlyphMeasures = std::make_shared<std::vector<std::pair<uint16_t, vec2>>>(tbox.measureGlyphs());
#endif
	}

	return *mGlyphMeasures;
}

#if 0
// This is not implemented
Shape2d TextSpan::getShape() const
{
	if( ! mShape ) {
		mShape = shared_ptr<Shape2d>( new Shape2d() );
		
		if( ! mString.empty() ) {
			shared_ptr<Font> font = getFont();
			if( ! font )
				return Shape2d();
			TextBox tbox = TextBox().font( *font ).text( mString );
			vector<pair<uint16_t,vec2> > glyphs = getGlyphMeasures();
			vec2 textPen = getTextPen();
			float rotation = getRotation();
			bool shouldRotate = fabs( rotation ) > 0.0001f;
			MatrixAffine2f rotationMatrix = MatrixAffine2f::makeRotate( toRadians( rotation ) );
			for( size_t g = 0; g < glyphs.size(); ++g ) {
				MatrixAffine2f m = MatrixAffine2f::makeTranslate( textPen + vec2( glyphs[g].second.x, 0 ) );
				if( shouldRotate )
					m *= rotationMatrix;
				mShape->append( font->getGlyphShape( glyphs[g].first ).getTransform( m ) );
			}
		}

		for( vector<TextSpanRef>::const_iterator spanIt = mSpans.begin(); spanIt != mSpans.end(); ++spanIt )
			mShape->append( (*spanIt)->getShape() );
	}
		
	return *mShape;
}
#endif

// TextSpan::Atributes
SvgTextSpan::Attributes::Attributes(const XmlTree& xml) {
	if (xml.hasAttribute("x")) mX = readValue(xml["x"]);
	if (xml.hasAttribute("y")) mY = readValue(xml["y"]);
	if (xml.hasAttribute("rotate")) mRotate = readValueList(xml["rotate"], false);
	if (xml.hasAttribute("letter-spacing")) mLetterSpacing = readValueList(xml["letter-spacing"], false);
}

std::shared_ptr<Font> SvgTextSpan::getFont() const {
	if (!mFont) {
		const vector<string>& fontFamilies = getFontFamilies();
		float				  fontSize	   = getFontSize().asUser();
		for (vector<string>::const_iterator familyIt = fontFamilies.begin(); familyIt != fontFamilies.end();
			 ++familyIt) {
			try {
				mFont = std::make_shared<Font>(*familyIt, fontSize);
				break;
			} catch (Exception& exc) {
				CI_LOG_W("failed to load font with name: " << *familyIt << ", size: " << fontSize
														   << ". what: " << exc.what() << "\t - loading default font.");
				mFont = std::make_shared<Font>(Font::getDefault());
			}
		}
	}

	return mFont;
}

vec2 SvgTextSpan::getTextPen() const {
	if (mIgnoreAttributes || (!mAttributes.mX.isSet()) || (!mAttributes.mY.isSet())) {
		if (!mParent)
			return {};
		else if (typeid(*mParent) == typeid(SvgTextSpan))
			return reinterpret_cast<const SvgTextSpan*>(mParent)->getTextPen();
		else if (typeid(*mParent) == typeid(SvgText))
			return reinterpret_cast<const SvgText*>(mParent)->getTextPen();
		else
			return {};
	} else
		return {mAttributes.mX.asUser(), mAttributes.mY.asUser()};
}

void SvgTextSpan::setTextPen(const vec2& textPen) {
	if (mIgnoreAttributes) {
		if (!mParent)
			return;
		else if (typeid(*mParent) == typeid(SvgTextSpan))
			return reinterpret_cast<SvgTextSpan*>(mParent)->setTextPen(textPen);
		else if (typeid(*mParent) == typeid(SvgText))
			return reinterpret_cast<SvgText*>(mParent)->setTextPen(textPen);
	} else
		mAttributes.setTextPen(textPen);
}

float SvgTextSpan::getRotation() const {
	if (mIgnoreAttributes || (mAttributes.mRotate.size() != 1)) {
		if (!mParent)
			return 0;
		else if (typeid(*mParent) == typeid(SvgTextSpan))
			return reinterpret_cast<const SvgTextSpan*>(mParent)->getRotation();
		else if (typeid(*mParent) == typeid(SvgText))
			return reinterpret_cast<const SvgText*>(mParent)->getRotation();
		else
			return 0;
	} else
		return mAttributes.mRotate[0].asUser();
}

Value SvgTextSpan::getLetterSpacing() const {
	if (mIgnoreAttributes || (mAttributes.mLetterSpacing.size() != 1)) {
		if (!mParent)
			return 0;
		else if (typeid(*mParent) == typeid(SvgTextSpan))
			return reinterpret_cast<const SvgTextSpan*>(mParent)->getLetterSpacing();
		else if (typeid(*mParent) == typeid(SvgText))
			return reinterpret_cast<const SvgText*>(mParent)->getLetterSpacing();
		else
			return 0;
	} else
		return mAttributes.mLetterSpacing[0];
}

void SvgTextSpan::Attributes::startRender(Renderer& renderer) const {
	if (mX.isSet() && mY.isSet()) renderer.pushTextPen(vec2(mX.asUser(), mY.asUser()));
	if (mRotate.size() == 1)
		renderer.pushTextRotation(mRotate[0].asUser());
	else
		renderer.pushTextRotation(0);
}

void SvgTextSpan::Attributes::finishRender(Renderer& renderer) const {
	if (mX.isSet() && mY.isSet()) renderer.popTextPen();
	renderer.popTextRotation();
}

void SvgTextSpan::Attributes::setTextPen(const vec2& textPen) {
	mX = Value(textPen.x);
	mY = Value(textPen.y);
}

////////////////////////////////////////////////////////////////////////////////////
// Defs
SvgDefs::SvgDefs(SvgNode* parent, const XmlTree& xml)
  : SvgGroup(parent)
  , mXml(xml) {
	parse(xml);
}

const SvgNode* SvgDefs::findNode(const std::string& id, bool recurse) const {
	const SvgNode* result = SvgGroup::findNode(id, recurse);
	if (!result) {
		// see if any immediate non-instantiated children are named 'id'
		for (XmlTree::ConstIter treeIt = mXml.begin(); treeIt != mXml.end(); ++treeIt) {
			if (!treeIt->hasAttribute("id")) continue;
			if (treeIt->getAttributeValue<std::string>("id") != id) continue;

			// instantiate the requested node and return it
			SvgDefs* self = const_cast<SvgDefs*>(this);
			SvgNode* node = create(self, *treeIt);
			if (node) self->mChildren.push_back(node);

			return node;
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////
// ClipPath
SvgClipPath::SvgClipPath(SvgNode* parent, const XmlTree& xml)
  : SvgGroup(parent, xml) {
	if (xml.hasAttribute("clipPathUnits")) {
		mUseObjectBoundingBox = xml.getAttributeValue<string>("clipPathUnits") != string("userSpaceOnUse");
	}

	for (const auto& child : mChildren) {
		if (!child->isDisplayNone() && child->isVisible()) {
			mIsDisplayNone = false;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Styles
SvgStyles::SvgStyles(SvgNode* parent, const XmlTree& xml)
  : SvgNode(parent, xml) {
	const auto value = trim(xml.getValue());

	throw std::runtime_error("Not implemented");

	// css::Parser parser;
	// parser.parse(value);

	//{
	//	Style					 style;
	//	std::vector<std::string> selectors;
	//	std::string				 key;
	//	std::string				 value;

	//	css::Parser::Token token = parser.getNextToken();
	//	while (token.type != css::Parser::CSS_END) {
	//		switch (token.type) {
	//		case css::Parser::SEL_START:
	//			selectors = split(token.data, ',', true);
	//			style.clear();
	//			break;
	//		case css::Parser::SEL_END:
	//			for (const auto& selector : selectors) {
	//				const auto id = ltrim_copy(selector, ".");
	//				if (mStyleList.count(id) > 0)
	//					mStyleList.at(id) += style;
	//				else
	//					mStyleList.insert_or_assign(id, style);
	//			}
	//			break;
	//		case css::Parser::PROPERTY:
	//			key = token.data;
	//			break;
	//		case css::Parser::VALUE:
	//			value = token.data;
	//			style.parseProperty(key, value, this);
	//			break;
	//		default:
	//			break;
	//		}

	//		token = parser.getNextToken();
	//	}
	//}
}

Style SvgStyles::findStyle(const std::string& id) const {
	if (mStyleList.count(id) > 0) return mStyleList.at(id);

	return {};
}

////////////////////////////////////////////////////////////////////////////////////
// Doc
SvgDoc::SvgDoc()
  : SvgGroup(nullptr)
  , mBounds(0, 0, 0, 0) {}

SvgDoc::SvgDoc(SvgNode* parent, const XmlTree& xml)
  : SvgGroup(parent, xml)
  , mBounds(0, 0, 0, 0) {
	loadDoc(xml);
}

SvgDoc::SvgDoc(const fs::path& filePath)
  : SvgGroup(nullptr)
  , mBounds(0, 0, 0, 0) {
	loadDoc(loadFile(filePath), filePath);
}

SvgDoc::SvgDoc(const DataSourceRef& dataSource, const fs::path& filePath)
  : SvgDoc(nullptr, dataSource, filePath) {}

SvgDoc::SvgDoc(SvgNode* parent, const DataSourceRef& dataSource, const fs::path& filePath)
  : SvgGroup(parent)
  , mBounds(0, 0, 0, 0) {
	fs::path relativePath = filePath;
	if (filePath.empty()) relativePath = fs::path(dataSource->getFilePathHint());
	loadDoc(dataSource, relativePath);
}

SvgDocRef SvgDoc::create(SvgNode* parent, const XmlTree& xml) {
	return std::make_shared<SvgDoc>(parent, xml);
}

SvgDocRef SvgDoc::create(const fs::path& filePath) {
	return std::make_shared<SvgDoc>(filePath);
}

SvgDocRef SvgDoc::create(const DataSourceRef& dataSource, const fs::path& filePath) {
	return std::make_shared<SvgDoc>(dataSource, filePath);
}

SvgDocRef SvgDoc::create(SvgNode* parent, const DataSourceRef& dataSource, const fs::path& filePath) {
	return std::make_shared<SvgDoc>(parent, dataSource, filePath);
}

SvgDocRef SvgDoc::createFromSvgz(const DataSourceRef& dataSource, const fs::path& filePath) {
	fs::path relativePath = filePath;
	if (filePath.empty()) relativePath = dataSource->getFilePathHint();

	const Buffer	compressed(dataSource);
	const BufferRef decompressed = make_shared<Buffer>(decompressBuffer(compressed, false, true));

	return std::make_shared<SvgDoc>(DataSourceBuffer::create(decompressed, relativePath));
}

void SvgDoc::loadDoc(const XmlTree& xml) {
	if (xml.hasAttribute("viewBox")) {
		const auto	vbox   = xml.getAttributeValue<string>("viewBox");
		const char* vbCPtr = vbox.c_str();
		mViewBox.x1		   = ds::parseFloat(&vbCPtr);
		mViewBox.y1		   = ds::parseFloat(&vbCPtr);
		mViewBox.x2		   = mViewBox.x1 + ds::parseFloat(&vbCPtr);
		mViewBox.y2		   = mViewBox.y1 + ds::parseFloat(&vbCPtr);
	} else {
		const SvgDoc* doc = getDoc();
		if (doc)
			mViewBox = doc->mViewBox;
		else
			mViewBox = getBoundingBox().transformed(getTransform()).scaledCentered(1.1f);
	}
	if (xml.hasAttribute("x")) {
		const Value val = Value::parse(xml.getAttributeValue<string>("x"));
		if (val.isPercent())
			mBounds.x1 = val.asUser(1) * mViewBox.getWidth();
		else
			mBounds.x1 = val.asUser(100, getDpi());
	}
	if (xml.hasAttribute("y")) {
		const Value val = Value::parse(xml.getAttributeValue<string>("y"));
		if (val.isPercent())
			mBounds.y1 = val.asUser(1) * mViewBox.getHeight();
		else
			mBounds.y1 = val.asUser(100, getDpi());
	}
	if (xml.hasAttribute("width")) {
		const Value val = Value::parse(xml.getAttributeValue<string>("width"));
		if (val.isPercent())
			mBounds.x2 = mBounds.x1 + val.asUser(1) * mViewBox.getWidth();
		else
			mBounds.x2 = mBounds.x1 + val.asUser(100, getDpi());
	} else
		mBounds.x2 = mBounds.x1 + mViewBox.getWidth();
	if (xml.hasAttribute("height")) {
		const Value val = Value::parse(xml.getAttributeValue<string>("height"));
		if (val.isPercent())
			mBounds.y2 = mBounds.y1 + val.asUser(1) * mViewBox.getHeight();
		else
			mBounds.y2 = mBounds.y1 + val.asUser(100, getDpi());
	} else
		mBounds.y2 = mBounds.y1 + mViewBox.getHeight();

	parseStyle(xml);
	SvgGroup::parse(xml);

	// If no viewBox was specified, make sure we calculate the bounds now that all children have been created.
	if (ds::approxZero(mViewBox.calcArea())) {
		mBoundingBoxCached = false;
		mViewBox		   = getBoundingBox().transformed(getTransform()).scaledCentered(1.1f);
		if (ds::approxZero(mBounds.getWidth())) mBounds.x2 = mBounds.x1 + mViewBox.getWidth();
		if (ds::approxZero(mBounds.getHeight())) mBounds.y2 = mBounds.y1 + mViewBox.getHeight();
	}

	const bool needsViewBoxMapping =
		mViewBox.getWidth() > 0 && mViewBox.getHeight() > 0 && getWidth() > 0 && getHeight() > 0;
	if (needsViewBoxMapping) {
		if (xml.hasAttribute("preserveAspectRatio"))
			setTransform(PreserveAspectRatio(xml.getAttributeValue<string>("preserveAspectRatio"))
							 .calcTransform(mBounds, mViewBox));
		else
			setTransform(PreserveAspectRatio().calcTransform(mBounds, mViewBox));
	}
}

void SvgDoc::loadDoc(const DataSourceRef& source, const fs::path& filePath) {
	if (!filePath.empty()) mFilePath = filePath.parent_path();

	const auto xml = std::make_shared<XmlTree>(source, XmlTree::ParseOptions().ignoreDataChildren(false));

	loadDoc(xml->getChild("svg"));
}

shared_ptr<Surface8u> SvgDoc::loadImage(const fs::path& relativePath) const {
	if (mImageCache.find(relativePath) == mImageCache.end()) {
		try {
			if (relativePath.string().substr(0, 4) == "http") {
				mImageCache[relativePath] =
					std::make_shared<Surface8u>(ci::loadImage(loadUrl(relativePath.string(), UrlOptions())));
			} else {
#if defined(CINDER_UWP)
				fs::path fullPath = (mFilePath / relativePath);
#else
				fs::path fullPath = (mFilePath / relativePath).make_preferred();
#endif

				if (exists(fullPath)) mImageCache[relativePath] = std::make_shared<Surface8u>(ci::loadImage(fullPath));
			}
		} catch (...) {}
	}

	if (mImageCache.find(relativePath) != mImageCache.end())
		return mImageCache[relativePath];
	else
		return {};
}

SvgNode* SvgDoc::nodeUnderPoint(const vec2& pt) const {
	return SvgGroup::nodeUnderPoint(pt, mat3());
}

void SvgDoc::renderSelf(Renderer& renderer) const {
	SvgGroup::renderSelf(renderer);
}

SvgChildNotFoundExc::SvgChildNotFoundExc(const string& child) {
	setDescription("Could not find child: " + child);
}

} // namespace nvpath::svg

#pragma warning(pop)