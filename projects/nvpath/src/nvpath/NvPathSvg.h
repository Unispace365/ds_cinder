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

#pragma once

#include "NvPath.h"

#include <cinder/Cinder.h>
#include <cinder/Color.h>
#include <cinder/Exception.h>
#include <cinder/Font.h>
#include <cinder/Noncopyable.h>
#include <cinder/PolyLine.h>
#include <cinder/Shape2d.h>
#include <cinder/Surface.h>
#include <cinder/Xml.h>

#include <ds/util/float_util.h>

#include <functional>
#include <map>

namespace nvpath::svg {

//
using FillRule = enum { FILL_RULE_NONZERO, FILL_RULE_EVEN_ODD };
//
using LineCap = enum { LINE_CAP_BUTT, LINE_CAP_ROUND, LINE_CAP_SQUARE };
//
using LineJoin = enum { LINE_JOIN_MITER, LINE_JOIN_ROUND, LINE_JOIN_BEVEL };
//
using FontWeight = enum {
	WEIGHT_100,
	WEIGHT_200,
	WEIGHT_300,
	WEIGHT_400,
	WEIGHT_NORMAL = WEIGHT_400,
	WEIGHT_500,
	WEIGHT_600,
	WEIGHT_700,
	WEIGHT_BOLD = WEIGHT_700,
	WEIGHT_800,
	WEIGHT_900
};
//
using Align = enum {
	ALIGN_NONE,
	ALIGN_X_MIN_Y_MIN,
	ALIGN_X_MID_Y_MIN,
	ALIGN_X_MAX_Y_MIN,
	ALIGN_X_MIN_Y_MID,
	ALIGN_X_MID_Y_MID,
	ALIGN_X_MAX_Y_MID,
	ALIGN_X_MIN_Y_MAX,
	ALIGN_X_MID_Y_MAX,
	ALIGN_X_MAX_Y_MAX
};
//
using MeetOrSlice = enum { MEET, SLICE };
// Defines the coordinate system used by gradients and images.
using TextAnchor = enum { TEXT_ANCHOR_START, TEXT_ANCHOR_MIDDLE, TEXT_ANCHOR_END };

// Forward declarations.
class SvgCircle;
class SvgClipPath;
class SvgDefs;
class SvgDoc;
class SvgEllipse;
class ExcChildNotFound;
class SvgGradient;
class SvgGroup;
class SvgImage;
class SvgLine;
class SvgNode;
class SvgPath;
class SvgPolygon;
class SvgPolyline;
class PreserveAspectRatio;
class SvgRect;
class Style;
class SvgStyles;
class SvgText;
class SvgTextSpan;
class SvgUse;

using SvgDocRef = std::shared_ptr<SvgDoc>;

//!
static CapsStyle toCapsStyle(LineCap lineCap);
//!
static JoinStyle toJoinStyle(LineJoin lineJoin);
//!
static SpreadMethod toSpreadMethod(std::string style);
//!
static CoordinateSpace toGradientUnits(std::string style);

//!
class PreserveAspectRatio {
  public:
	PreserveAspectRatio() = default;

	explicit PreserveAspectRatio(Align align, MeetOrSlice meetOrSlice = MEET)
	  : align(align)
	  , meetOrSlice(meetOrSlice) {}

	explicit PreserveAspectRatio(const std::string& str);
	explicit PreserveAspectRatio(const char** sInOut);

	glm::mat3x2 calcTransform(const ci::Rectf& element, const ci::Rectf& viewBox, bool normalized = false) const;

	void parse(const char** sInOut);

	Align		align{ALIGN_X_MID_Y_MID};
	MeetOrSlice meetOrSlice{MEET};
};

//! SVG Value/Unit pair
class Value {
  public:
	enum Unit { USER, PX, PERCENT, PT, PC, MM, CM, INCH, EM, EX, AUTO };

	Value() = default;
	Value(float value, Unit unit = USER)
	  : mUnit(unit)
	  , mValue(value)
	  , mIsSet(true) {}

	[[nodiscard]] float asUser(float percentOf = 100, float dpi = 72, float fontSize = 12, float fontXHeight = 7) const;
	[[nodiscard]] float asUser(const SvgDoc* doc, const Style& style) const;
	[[nodiscard]] float asUserWidth(const SvgDoc* doc, const Style& style) const;
	[[nodiscard]] float asUserHeight(const SvgDoc* doc, const Style& style) const;

	float value() const { return mValue; }
	Unit  unit() const { return mUnit; }

	bool isSet() const { return mIsSet; }
	bool isUser() const { return mUnit == USER; }
	bool isPercent() const { return mUnit == PERCENT; }
	bool isPixels() const { return mUnit == PX; }
	bool isAuto() const { return mUnit == AUTO; }

	[[nodiscard]] static Value parse(const char** sInOut);
	[[nodiscard]] static Value parse(const std::string& s);

  private:
	Unit  mUnit{USER};
	float mValue{};
	bool  mIsSet{false};
};

using RenderVisitor = std::function<bool(const SvgNode&, Style*)>;

//! Base class from which Renderers are derived.
class Renderer {
  public:
	Renderer()			= default;
	virtual ~Renderer() = default;

	Renderer(const Renderer&)			 = default;
	Renderer(Renderer&&)				 = default;
	Renderer& operator=(const Renderer&) = default;
	Renderer& operator=(Renderer&&)		 = default;

	void setVisitor(const std::function<bool(const SvgNode&, Style*)>& visitor);

	virtual void start() {}
	virtual void finish() {}

	//! Clears rendering cache.
	virtual void clear() {}

	virtual void pushGroup(const SvgGroup& /*group*/, float /*opacity*/) {}
	virtual void popGroup() {}
	virtual void pushClipPath(const SvgClipPath& /* clippath */) {}
	virtual void popClipPath() {}
	virtual void drawPath(const SvgPath& /*path*/) {}
	virtual void drawPolyline(const SvgPolyline& /*polyline*/) {}
	virtual void drawPolygon(const SvgPolygon& /*polygon*/) {}
	virtual void drawLine(const SvgLine& /*line*/) {}
	virtual void drawRect(const SvgRect& /*rect*/) {}
	virtual void drawCircle(const SvgCircle& /*circle*/) {}
	virtual void drawEllipse(const SvgEllipse& /*ellipse*/) {}
	virtual void drawImage(const SvgImage& /*image*/) {}
	virtual void drawTextSpan(const SvgTextSpan& /*span*/) {}

	virtual void pushMatrix(const glm::mat3& /*m*/) {}
	virtual void popMatrix() {}
	virtual void pushStyle(const Style& /*style*/) {}
	virtual void popStyle() {}
	virtual void pushFill(const Paint& /*paint*/) {}
	virtual void popFill() {}
	virtual void pushStroke(const Paint& /*paint*/) {}
	virtual void popStroke() {}
	virtual void pushFillOpacity(float /*opacity*/) {}
	virtual void popFillOpacity() {}
	virtual void pushStrokeOpacity(float /*opacity*/) {}
	virtual void popStrokeOpacity() {}
	virtual void pushStrokeWidth(float /*width*/) {}
	virtual void popStrokeWidth() {}
	virtual void pushFillRule(FillRule /*rule*/) {}
	virtual void popFillRule() {}
	virtual void pushLineCap(LineCap /*lineCap*/) {}
	virtual void popLineCap() {}
	virtual void pushLineJoin(LineJoin /*lineJoin*/) {}
	virtual void popLineJoin() {}
	virtual void pushMiterLimit(float /*miterLimit*/) {}
	virtual void popMiterLimit() {}
	virtual void pushDashArray(const std::vector<float>& /*dashArray*/) {}
	virtual void popDashArray() {}
	virtual void pushDashOffset(float /*dashOffset*/) {}
	virtual void popDashOffset() {}
	virtual void pushTextPen(const glm::vec2& /*penPos*/) {}
	virtual void popTextPen() {}
	virtual void pushTextRotation(float /*rotation*/) {}
	virtual void popTextRotation() {}

	bool visit(const SvgNode& node, Style* style) const {
		if (mVisitor) return (*mVisitor)(node, style);
		return true;
	}

	struct Stacks {
		std::vector<glm::mat3>			matrix;
		std::vector<Paint>				fill;
		std::vector<Paint>				stroke;
		std::vector<float>				fillOpacity;
		std::vector<float>				strokeOpacity;
		std::vector<float>				groupOpacity;
		std::vector<float>				strokeWidth;
		std::vector<FillRule>			fillRule;
		std::vector<LineCap>			lineCap;
		std::vector<LineJoin>			lineJoin;
		std::vector<float>				miterLimit;
		std::vector<std::vector<float>> dashArray;
		std::vector<float>				dashOffset;
		std::vector<glm::vec2>			textPen;
		std::vector<float>				textRotation;
		std::vector<const SvgClipPath*> clipPath;

		void clear() {
			matrix.clear();
			fill.clear();
			stroke.clear();
			fillOpacity.clear();
			strokeOpacity.clear();
			groupOpacity.clear();
			strokeWidth.clear();
			fillRule.clear();
			lineCap.clear();
			lineJoin.clear();
			miterLimit.clear();
			dashArray.clear();
			dashOffset.clear();
			textPen.clear();
			textRotation.clear();
			clipPath.clear();
		}

		void defaults() {
			clear();

			matrix.emplace_back();
			fill.emplace_back(ci::Color::black());
			stroke.emplace_back();
			fillOpacity.push_back(1.0f);
			strokeOpacity.push_back(1.0f);
			groupOpacity.push_back(1.0f);
			strokeWidth.push_back(1.0f);
			fillRule.push_back(FILL_RULE_NONZERO);
			lineCap.push_back(LINE_CAP_BUTT);
			lineJoin.push_back(LINE_JOIN_MITER);
			miterLimit.push_back(4.0f);
			dashArray.emplace_back();
			dashOffset.push_back(0.0f);
			textPen.emplace_back(0.0f, 0.0f);
			textRotation.push_back(0.0f);
		}

		//! Work-around for missing support of group opacity: this combines all opacities into a single value.
		[[nodiscard]] float calcGroupOpacity() const {
			float result = 1;
			for (const auto opacity : groupOpacity)
				result *= opacity;
			return result;
		}
	};

  protected:
	// this is a shared_ptr to work around a bug in Clang 4.0
	std::shared_ptr<std::function<bool(const SvgNode&, Style*)>> mVisitor;

	friend class SvgNode;
};

//! SVG Style for a node. Corresponds to SVG Styling: http://www.w3.org/TR/SVG/styling.html
class Style {
  public:
	Style();
	Style(const ci::XmlTree& xml, const SvgNode* parent);

	//! Returns a Style set appropriately for global defaults
	static Style makeGlobalDefaults();
	//! Marks all styles as unspecified
	void clear();

	bool				specifiesColor() const { return mSpecifiesColor; }
	void				unspecifyColor() { mSpecifiesColor = false; }
	const ci::ColorA8u& getColor() const { return mColor; }
	void				setColor(const ci::ColorA8u& color) {
		   mSpecifiesColor = true;
		   mColor		   = color;
	}
	[[nodiscard]] static const ci::ColorA8u& getColorDefault();

	bool		 specifiesFill() const { return mSpecifiesFill; }
	void		 unspecifyFill() { mSpecifiesFill = false; }
	const Paint& getFill() const { return mFill; }
	void		 setFill(const Paint& fill) {
		mSpecifiesFill = true;
		mFill		   = fill;
	}
	[[nodiscard]] static const Paint& getFillDefault();

	bool		 specifiesStroke() const { return mSpecifiesStroke; }
	void		 unspecifyStroke() { mSpecifiesStroke = false; }
	const Paint& getStroke() const { return mStroke; }
	void		 setStroke(const Paint& stroke) {
		mSpecifiesStroke = true;
		mStroke			 = stroke;
	}
	[[nodiscard]] static const Paint& getStrokeDefault();

	bool  specifiesOpacity() const { return mSpecifiesOpacity; }
	void  unspecifyOpacity() { mSpecifiesOpacity = false; }
	float getOpacity() const { return mOpacity; }
	void  setOpacity(float opacity) {
		 mSpecifiesOpacity = true;
		 mOpacity		   = opacity;
	}
	[[nodiscard]] static float getOpacityDefault() { return 1.0f; }

	bool  specifiesStrokeOpacity() const { return mSpecifiesStrokeOpacity; }
	void  unspecifyStrokeOpacity() { mSpecifiesStrokeOpacity = false; }
	float getStrokeOpacity() const { return mStrokeOpacity; }
	void  setStrokeOpacity(float strokeOpacity) {
		 mSpecifiesStrokeOpacity = true;
		 mStrokeOpacity			 = strokeOpacity;
	}
	[[nodiscard]] static float getStrokeOpacityDefault() { return 1.0f; }

	bool  specifiesFillOpacity() const { return mSpecifiesFillOpacity; }
	void  unspecifyFillOpacity() { mSpecifiesFillOpacity = false; }
	float getFillOpacity() const { return mFillOpacity; }
	void  setFillOpacity(float fillOpacity) {
		 mSpecifiesFillOpacity = true;
		 mFillOpacity		   = fillOpacity;
	}
	[[nodiscard]] static float getFillOpacityDefault() { return 1.0f; }

	bool  specifiesStrokeWidth() const { return mSpecifiesStrokeWidth; }
	void  unspecifyStrokeWidth() { mSpecifiesStrokeWidth = false; }
	float getStrokeWidth() const { return mStrokeWidth; }
	void  setStrokeWidth(float strokeWidth) {
		 mSpecifiesStrokeWidth = true;
		 mStrokeWidth		   = strokeWidth;
	}
	[[nodiscard]] static float getStrokeWidthDefault() { return 1.0f; }

	bool	 specifiesFillRule() const { return mSpecifiesFillRule; }
	void	 unspecifyFillRule() { mSpecifiesFillRule = false; }
	FillRule getFillRule() const { return mFillRule; }
	void	 setFillRule(FillRule fillRule) {
		mSpecifiesFillRule = true;
		mFillRule		   = fillRule;
	}
	[[nodiscard]] static FillRule getFillRuleDefault() { return FILL_RULE_NONZERO; }

	bool	specifiesLineCap() const { return mSpecifiesLineCap; }
	void	unspecifyLineCap() { mSpecifiesLineCap = false; }
	LineCap getLineCap() const { return mLineCap; }
	void	setLineCap(LineCap lineCap) {
		   mSpecifiesLineCap = true;
		   mLineCap			 = lineCap;
	}
	[[nodiscard]] static LineCap getLineCapDefault() { return LINE_CAP_BUTT; }

	bool	 specifiesLineJoin() const { return mSpecifiesLineJoin; }
	void	 unspecifyLineJoin() { mSpecifiesLineJoin = false; }
	LineJoin getLineJoin() const { return mLineJoin; }
	void	 setLineJoin(LineJoin lineJoin) {
		mSpecifiesLineJoin = true;
		mLineJoin		   = lineJoin;
	}
	[[nodiscard]] static LineJoin getLineJoinDefault() { return LINE_JOIN_MITER; }

	bool  specifiesMiterLimit() const { return mSpecifiesMiterLimit; }
	void  unspecifyMiterLimit() { mSpecifiesMiterLimit = false; }
	float getMiterLimit() const { return mMiterLimit; }
	void  setMiterLimit(float miterLimit) {
		 mSpecifiesMiterLimit = true;
		 mMiterLimit		  = miterLimit;
	}
	[[nodiscard]] static float getMiterLimitDefault() { return 4; }

	bool					  specifiesDashArray() const { return mSpecifiesDashArray; }
	void					  unspecifyDashArray() { mSpecifiesDashArray = false; }
	const std::vector<float>& getDashArray() const { return mDashArray; }
	void					  setDashArray(const std::vector<float>& dashArray) {
		 mSpecifiesDashArray = true;
		 mDashArray			 = dashArray;
	}
	[[nodiscard]] static const std::vector<float>& getDashArrayDefault() {
		static std::vector<float> sNone;
		return sNone;
	}

	bool  specifiesDashOffset() const { return mSpecifiesDashOffset; }
	void  unspecifyDashOffset() { mSpecifiesDashOffset = false; }
	float getDashOffset() const { return mDashOffset; }
	void  setDashOffset(float dashOffset) {
		 mSpecifiesDashOffset = true;
		 mDashOffset		  = dashOffset;
	}
	[[nodiscard]] static float getDashOffsetDefault() { return 0; }

	// clip path
	bool			   specifiesClipPath() const { return mSpecifiesClipPath; }
	void			   unspecifyClipPath() { mSpecifiesClipPath = false; }
	const std::string& getClipPath() const { return mClipPathId; }
	void			   setClipPath(const std::string& clipPath) {
		  mSpecifiesClipPath = true;
		  mClipPathId		 = clipPath;
	}

	// stops
	bool				specifiesStopColor() const { return mSpecifiesStopColor; }
	void				unspecifyStopColor() { mSpecifiesStopColor = false; }
	const ci::ColorA8u& getStopColor() const { return mStopColor; }
	void				setStopColor(const ci::ColorA8u& stopColor) {
		   mSpecifiesStopColor = true;
		   mStopColor		   = stopColor;
	}
	[[nodiscard]] static ci::ColorA8u getStopColorDefault() { return {0, 0, 0, 255}; }

	bool  specifiesStopOpacity() const { return mSpecifiesStopOpacity; }
	void  unspecifyStopOpacity() { mSpecifiesStopOpacity = false; }
	float getStopOpacity() const { return mStopOpacity; }
	void  setStopOpacity(float stopOpacity) {
		 mSpecifiesStopOpacity = true;
		 mStopOpacity		   = stopOpacity;
	}
	[[nodiscard]] static float getStopOpacityDefault() { return 1; }

	// fonts
	bool							specifiesFontFamilies() const { return mSpecifiesFontFamilies; }
	void							unspecifyFontFamilies() { mSpecifiesFontFamilies = false; }
	const std::vector<std::string>& getFontFamilies() const { return mFontFamilies; }
	std::vector<std::string>&		getFontFamilies() { return mFontFamilies; }
	void							setFontFamily(const std::string& family) {
		   mSpecifiesFontFamilies = true;
		   mFontFamilies.clear();
		   mFontFamilies.push_back(family);
	}
	void setFontFamilies(const std::vector<std::string>& families) {
		mSpecifiesFontFamilies = true;
		mFontFamilies		   = families;
	}
	[[nodiscard]] static const std::vector<std::string>& getFontFamiliesDefault();

	bool  specifiesFontSize() const { return mSpecifiesFontSize; }
	void  unspecifyFontSize() { mSpecifiesFontSize = false; }
	Value getFontSize() const { return mFontSize; }
	void  setFontSize(const Value& fontSize) {
		 mSpecifiesFontSize = true;
		 mFontSize			= fontSize;
	}
	[[nodiscard]] static Value getFontSizeDefault() { return {12}; }

	bool	   specifiesFontWeight() const { return mSpecifiesFontWeight; }
	void	   unspecifyFontWeight() { mSpecifiesFontWeight = false; }
	FontWeight getFontWeight() const { return mFontWeight; }
	void	   setFontWeight(FontWeight weight) {
		  mSpecifiesFontWeight = true;
		  mFontWeight		   = weight;
	}
	[[nodiscard]] static FontWeight getFontWeightDefault() { return WEIGHT_NORMAL; }

	bool	   specifiesTextAnchor() const { return mSpecifiesTextAnchor; }
	void	   unspecifyTextAnchor() { mSpecifiesTextAnchor = false; }
	TextAnchor getTextAnchor() const { return mTextAnchor; }
	void	   setTextAnchor(TextAnchor anchor) {
		  mSpecifiesTextAnchor = true;
		  mTextAnchor		   = anchor;
	}
	[[nodiscard]] static TextAnchor getTextAnchorDefault() { return TEXT_ANCHOR_START; }

	bool specifiesVisible() const { return mSpecifiesVisible; }
	bool isVisible() const { return mVisible; }
	void setVisible(bool visible) {
		mSpecifiesVisible = true;
		mVisible		  = visible;
	}
	void unspecifyVisible() { mSpecifiesVisible = false; }

	bool isDisplayNone() const { return mDisplayNone; }
	void setDisplayNone(bool displayNone) { mDisplayNone = displayNone; }

	void startRender(Renderer& renderer, const SvgNode* node) const;
	void finishRender(Renderer& renderer, const SvgNode* node) const;

	void parseClassAttribute(const std::string& stylePropertyString, const SvgNode* parent);
	void parseStyleAttribute(const std::string& stylePropertyString, const SvgNode* parent);
	bool parseProperty(const std::string& key, const std::string& value, const SvgNode* parent);

	Paint parsePaint(const char* value, bool* specified, const SvgNode* parent) const;

	// Attempts to find a Style definition in one of its parents for the value stored in mId.
	void resolve(const SvgNode* node) const;

	bool operator==(const Style& other) const;
	bool operator!=(const Style& other) const { return !(*this == other); }

	// Merges the right-hand Style into the left-hand one.
	void operator+=(const Style& other);
	// Merges the two styles.
	Style operator+(const Style& other) const;

  protected:
	bool  mSpecifiesOpacity;
	float mOpacity;
	bool  mSpecifiesFillOpacity, mSpecifiesStrokeOpacity;
	float mFillOpacity, mStrokeOpacity;

	bool			   mSpecifiesColor;
	ci::ColorA8u	   mColor;
	bool			   mSpecifiesFill, mSpecifiesStroke;
	mutable Paint	   mFill, mStroke; // Paint might need to be resolved from a 'const' method.
	bool			   mSpecifiesStrokeWidth;
	float			   mStrokeWidth;
	bool			   mSpecifiesFillRule;
	FillRule		   mFillRule;
	bool			   mSpecifiesLineCap;
	LineCap			   mLineCap;
	bool			   mSpecifiesLineJoin;
	LineJoin		   mLineJoin;
	bool			   mSpecifiesMiterLimit;
	float			   mMiterLimit;
	bool			   mSpecifiesDashArray;
	std::vector<float> mDashArray;
	bool			   mSpecifiesDashOffset;
	float			   mDashOffset;
	bool			   mSpecifiesClipPath;
	std::string		   mClipPathId;
	bool			   mSpecifiesStopColor;
	ci::ColorA8u	   mStopColor;
	bool			   mSpecifiesStopOpacity;
	float			   mStopOpacity;

	// cached clip-path
	mutable const SvgClipPath* mClipPath = nullptr;

	// fonts
	bool					 mSpecifiesFontFamilies, mSpecifiesFontSize, mSpecifiesFontWeight, mSpecifiesTextAnchor;
	std::vector<std::string> mFontFamilies;
	Value					 mFontSize;
	FontWeight				 mFontWeight;
	TextAnchor				 mTextAnchor;

	// visibility
	bool mSpecifiesVisible, mVisible, mDisplayNone;
};

//! Base class for an element of an SVG Document
class SvgNode {
  public:
	explicit SvgNode(SvgNode* parent)
	  : mParent(parent)
	  , mSpecifiesTransform(false)
	  , mBoundingBoxCached(false) {
		mUuid = nextUuid();
	}
	virtual ~SvgNode() = default;

	SvgNode(const SvgNode&)			   = delete;
	SvgNode(SvgNode&&)				   = delete;
	SvgNode& operator=(const SvgNode&) = delete;
	SvgNode& operator=(SvgNode&&)	   = delete;

	//! Returns the unique id for this node.
	size_t getUuid() const { return mUuid; }
	//! Returns the svg::Doc this Node is an element of
	[[nodiscard]] const SvgDoc* getDoc() const;
	//! Returns the immediate parent of this node
	const SvgNode* getParent() const { return mParent; }
	//! Returns the tag of this Node when present (e.g. 'svg').
	const std::string& getTag() const { return mTag; }
	//! Returns the ID of this Node when present.
	const std::string& getId() const { return mId; }
	//! Returns a DOM-style path to this node.
	[[nodiscard]] std::string getDomPath() const;
	//! Returns the style elements defined on this Node but not inherited from ancestors.
	[[nodiscard]] const Style& getStyle() const {
		mStyle.resolve(this);
		return mStyle;
	}
	//! Sets the style defined on this Node but not inherited from ancestors.
	void setStyle(const Style& style) { mStyle = style; }
	//! Returns the node's Style, including attributes inherited from its ancestors for attributes it does not
	//! specify
	[[nodiscard]] Style calcInheritedStyle() const;

	//! Returns the ClipPath for this node. Returns NULL on failure.
	[[nodiscard]] virtual const SvgClipPath* getClipPath() const;
	//! Returns the ClipPath for the specified \a style. Returns NULL on failure.
	[[nodiscard]] virtual const SvgClipPath* getClipPath(const Style& style) const;

	//! Returns whether the point \a pt is inside of the Node's shape.
	virtual bool containsPoint(const glm::vec2& /*pt*/) const { return false; }

	//! Renders the node and its descendants.
	void render(Renderer& renderer) const;

	//! Finds the node with ID \a elementId amongst this Node's ancestors. Returns NULL on failure.
	[[nodiscard]] virtual const SvgNode* findInAncestors(const std::string& elementId) const;
	//! Finds the svg::Paint node with ID \a elementId amongst this Node's ancestors. Returns a default svg::Paint
	//! instance on failure.
	[[nodiscard]] Paint findPaintInAncestors(const std::string& paintName) const;
	//! Recursively searches for a <tag> node and returns the first it finds. Returns NULL on failure.
	[[nodiscard]] virtual const SvgNode* findTagInAncestors(const std::string& tag) const;

	//! Returns whether this Node specifies a transformation
	bool specifiesTransform() const { return mSpecifiesTransform; }
	//! Returns the local transformation of this node. Returns identity if the Node's transform isn't specified.
	glm::mat3 getTransform() const { return mTransform; }
	//! Sets the local transformation of this node.
	void setTransform(const glm::mat3& transform) {
		mTransform			= transform;
		mSpecifiesTransform = true;
	}
	//! Removes the local transformation of this node, effectively making it the identity matrix.
	void unspecifyTransform() { mSpecifiesTransform = false; }
	//! Returns the inverse of the local transformation of this node. Returns identity if the Node's transform isn't
	//! specified.
	[[nodiscard]] glm::mat3 getTransformInverse() const {
		return (mSpecifiesTransform) ? inverse(mTransform) : glm::mat3();
	}
	//! Returns the absolute transformation of this node, which includes inherited transformations.
	[[nodiscard]] glm::mat3 getTransformAbsolute() const;
	//! Returns the inverse of the absolute transformation of this node, which includes inherited transformations.
	[[nodiscard]] glm::mat3 getTransformAbsoluteInverse() const { return inverse(getTransformAbsolute()); }

	//! Returns the local bounding box of the Node. Calculated and cached the first time it is requested.
	[[nodiscard]] ci::Rectf getBoundingBox() const {
		if (!mBoundingBoxCached) {
			mBoundingBox	   = calcBoundingBox();
			mBoundingBoxCached = true;
		}
		return mBoundingBox;
	}
	//! Returns the absolute bounding box of the Node. Calculated and cached the first time it is requested.
	[[nodiscard]] ci::Rectf getBoundingBoxAbsolute() const {
		return getBoundingBox().transformed(getTransformAbsolute());
	}

	//! Returns a Shape2d representing the node in local coordinates. Not supported for Text.
	virtual ci::Shape2d getShape() const { return {}; }
	//! Returns a Shape2d representing the node in absolute coordinates. Not supported for Text.
	[[nodiscard]] ci::Shape2d getShapeAbsolute() const { return getShape().transformed(getTransformAbsolute()); }

	//! Returns node's color, or the first among its ancestors when it has none
	const ci::ColorA8u& getColor() const;
	//! Returns node's fill, or the first among its ancestors when it has none
	const Paint& getFill() const;
	//! Returns node's stroke, or the first among its ancestors when it has none
	const Paint& getStroke() const;
	//! Returns node's opacity, or the first among its ancestors when it has none
	float getOpacity() const;
	//! Returns node's fill opacity, or the first among its ancestors when it has none
	float getFillOpacity() const;
	//! Returns node's stroke opacity, or the first among its ancestors when it has none
	float getStrokeOpacity() const;
	//! Returns node's fill rule, or the first among its ancestors when it has none
	FillRule getFillRule() const;
	//! Returns node's line cap, or the first among its ancestors when it has none
	LineCap getLineCap() const;
	//! Returns node's line join, or the first among its ancestors when it has none
	LineJoin getLineJoin() const;
	//! Returns node's miter limit, or the first among its ancestors when it has none
	float getMiterLimit() const;
	//! Returns node's dash array, or the first among its ancestors when it has none
	const std::vector<float>& getDashArray() const;
	//! Returns node's dash offset, or the first among its ancestors when it has none
	float getDashOffset() const;
	//! Returns node's stop color, or the first among its ancestors when it has none
	ci::ColorA8u getStopColor() const;
	//! Returns node's stop opacity, or the first among its ancestors when it has none
	float getStopOpacity() const;
	//! Returns node's stroke width, or the first among its ancestors when it has none
	float getStrokeWidth() const;
	//! Returns node's font families, or the first among its ancestors when it has none
	const std::vector<std::string>& getFontFamilies() const;
	//! Returns node's font size, or the first among its ancestors when it has none
	Value getFontSize() const;
	//! Returns node's text anchor, or the first among its ancestors when it has none
	TextAnchor getTextAnchor() const;
	//! Returns whether this Node is visible, or the first among its ancestors when unspecified
	virtual bool isVisible() const;
	//! Returns whether the Display property of this Node is set to 'None', preventing rendering of the node and its
	//! children
	virtual bool isDisplayNone() const { return getStyle().isDisplayNone(); }
	//! Returns whether this type of node directly renders anything. Everything but groups and gradients.
	virtual bool isDrawable() const { return true; }

	//
	static size_t nextUuid() {
		static size_t uuid = 0;
		return ++uuid;
	}

  protected:
	SvgNode(SvgNode* parent, const ci::XmlTree& xml);

	void		 startRender(Renderer& renderer, const Style& style) const;
	void		 finishRender(Renderer& renderer, const Style& style) const;
	virtual void renderSelf(Renderer& renderer) const = 0;

	virtual ci::Rectf calcBoundingBox() const { return {0, 0, 0, 0}; }

	static Paint	 parsePaint(const char* value, bool* specified, const SvgNode* parentNode);
	static glm::mat3 parseTransform(const std::string& value);
	static bool		 parseTransformComponent(const char** c, glm::mat3* result);

	static std::string findStyleValue(const std::string& styleString, const std::string& key);
	void			   parseStyle(const ci::XmlTree& xml);

	size_t			  mUuid;
	SvgNode*		  mParent;
	std::string		  mTag;
	std::string		  mId;
	bool			  mSpecifiesTransform;
	glm::mat3		  mTransform;
	mutable bool	  mBoundingBoxCached;
	mutable ci::Rectf mBoundingBox;
	Style			  mStyle; // Try avoiding directly accessing this variable, use getStyle() instead if possible.

  private:
	void firstStartRender(Renderer& renderer) const;

	friend class SvgGroup;
	friend class SvgUse;
};

//! Base class for SVG Gradients. See SVG Gradients: http://www.w3.org/TR/SVG/pservers.html#Gradients
class SvgGradient : public SvgNode {
  public:
	SvgGradient(SvgNode* parent, const ci::XmlTree& xml);

	class Stop {
	  public:
		Stop() = default;
		Stop(const SvgNode* parent, const ci::XmlTree& xml);

		bool operator<(const Stop& rhs) const { return offset < rhs.offset; }

		float		 offset{0}; // normalized 0-1
		ci::ColorA8u color{0, 0, 0, 255};
		float		 opacity{1};
		bool		 specifiesColor{true};
		bool		 specifiesOpacity{true};
	};

	bool useObjectBoundingBox() const { return mUseObjectBoundingBox; }

	SpreadMethod getSpreadMethod() const { return mSpreadMethod; }

	virtual Paint::Type getType() const { return Paint::Type::NONE; }

	//!
	virtual Paint asPaint() const;

	virtual void parse(const ci::XmlTree& xml);

  protected:
	void renderSelf(Renderer& /*renderer*/) const override {}

	void copyAttributesFrom(const SvgGradient& rhs);

	std::vector<Stop> mStops;
	bool			  mUseObjectBoundingBox{true};
	bool			  mSpecifiesSpreadMethod{false};
	SpreadMethod	  mSpreadMethod{SpreadMethod::PAD};
};

//! SVG Linear gradient
class SvgLinearGradient : public SvgGradient {
  public:
	SvgLinearGradient(SvgNode* parent, const ci::XmlTree& xml);

	bool isDrawable() const override { return false; }

	Paint::Type getType() const override { return Paint::Type::LINEAR_GRADIENT; }

	Paint asPaint() const override;

	void parse(const ci::XmlTree& xml) override;

  protected:
	void copyAttributesFrom(const SvgLinearGradient& rhs);

	Value mX1{0, Value::PERCENT};
	Value mY1{0, Value::PERCENT};
	Value mX2{100, Value::PERCENT};
	Value mY2{0, Value::PERCENT};

	friend class SvgGradient;
};

//! SVG Radial gradient
class SvgRadialGradient : public SvgGradient {
  public:
	SvgRadialGradient(SvgNode* parent, const ci::XmlTree& xml);

	bool isDrawable() const override { return false; }

	Paint::Type getType() const override { return Paint::Type::RADIAL_GRADIENT; }

	Paint asPaint() const override;

	void parse(const ci::XmlTree& xml) override;

  protected:
	void copyAttributesFrom(const SvgRadialGradient& rhs);

	Value mCx{50, Value::PERCENT};
	Value mCy{50, Value::PERCENT};
	Value mR{50, Value::PERCENT};
	Value mFx{mCx};
	Value mFy{mCy};
	Value mFr{0, Value::PERCENT};

	friend class SvgGradient;
};

class Svg : public Renderer {
  public:
	Svg() = default;

	explicit Svg(const ci::DataSourceRef& src);
	explicit Svg(const SvgDocRef& svg);

	float			 getWidth() const { return mBounds.getWidth(); }
	float			 getHeight() const { return mBounds.getHeight(); }
	ci::vec2		 getSize() const { return mBounds.getSize(); }
	const ci::Rectf& getBounds() const { return mBounds; }

	float getOpacity() const { return mOpacity; }
	void  setOpacity(float opacity) { mOpacity = opacity; }

	//! Clears rendering cache.
	void clear() override;

  private:
	// svg::Renderer callbacks.

	void start() override;
	void finish() override;
	void pushGroup(const SvgGroup& group, float opacity) override;
	void popGroup() override;
	void pushClipPath(const SvgClipPath& clippath) override;
	void popClipPath() override;
	void drawPath(const SvgPath&) override;
	void drawPolyline(const SvgPolyline&) override;
	void drawPolygon(const SvgPolygon&) override;
	void drawLine(const SvgLine&) override;
	void drawRect(const SvgRect&) override;
	void drawCircle(const SvgCircle&) override;
	void drawEllipse(const SvgEllipse&) override;
	void drawImage(const SvgImage&) override;
	void drawTextSpan(const SvgTextSpan&) override { /* currently not implemented */
	}
	void pushMatrix(const glm::mat3&) override;
	void popMatrix() override;
	void pushFill(const Paint&) override;
	void popFill() override;
	void pushStroke(const Paint&) override;
	void popStroke() override;
	void pushFillOpacity(float) override;
	void popFillOpacity() override;
	void pushStrokeOpacity(float) override;
	void popStrokeOpacity() override;
	void pushStrokeWidth(float) override;
	void popStrokeWidth() override;
	void pushFillRule(FillRule) override;
	void popFillRule() override;
	void pushLineCap(LineCap) override;
	void popLineCap() override;
	void pushLineJoin(LineJoin) override;
	void popLineJoin() override;
	void pushMiterLimit(float miterLimit) override;
	void popMiterLimit() override;
	void pushDashArray(const std::vector<float>& dashArray) override;
	void popDashArray() override;
	void pushDashOffset(float dashOffset) override;
	void popDashOffset() override;
	void pushTextPen(const ci::vec2&) override;
	void popTextPen() override;
	void pushTextRotation(float) override;
	void popTextRotation() override;

	//! Returns whether the current style has fill or stroke enabled.
	bool shouldRender() const { return !(mStacks.fill.back().isNone() && mStacks.stroke.back().isNone()); }
	//! Generates a draw call for visible paths.
	void render(const Path& path);
	//! Fills the path with the specified \a paint and \a opacity.
	void fill(GLuint pathId, const Paint& paint, float opacity);
	//! Strokes the path with the specified \a paint and \a opacity.
	void stroke(GLuint pathId, const Paint& paint, float opacity);
	//! Strokes the instances with the specified \a paint and \a opacity.
	void fillInstanced(GLuint baseId, GLsizei count, const glm::mat3x2* transforms, const uint32_t* indices,
					   const Paint& paint, float opacity);
	//! Strokes the instances with the specified \a paint and \a opacity.
	void strokeInstanced(GLuint baseId, GLsizei count, const glm::mat3x2* transforms, const uint32_t* indices,
						 const Paint& paint, float opacity);

	//! Returns whether the path with the specified \a uuid exists and returns a pointer to the path if it exists.
	const Path* findPath(size_t uuid) const;
	//! Caches a clone of the path using the specified \a uuid and \a path. Returns a pointer to the new path.
	const Path* insertPath(size_t uuid, const Path& path, bool isClipPath = false);
	//! Caches a path using the specified \a uuid and SVG \a path. Returns a pointer to the new path.
	const Path* insertPath(size_t uuid, const std::string& path, bool isClipPath = false);
	//! Caches a path using the specified \a uuid and \a path. Returns a pointer to the new path.
	const Path* insertPath(size_t uuid, const ci::Path2d& path, bool isClipPath = false);
	//! Caches a path using the specified \a uuid and \a shape. Returns a pointer to the new path.
	const Path* insertPath(size_t uuid, const ci::Shape2d& shape, bool isClipPath = false);

	//! Returns the stencil mask based on the current fill rule.
	GLuint getStencilMask() const { return mStacks.fillRule.back() == FILL_RULE_EVEN_ODD ? 0x01 : 0xFF; }

	SvgDocRef                                        mDoc;
	ci::gl::Context*                                 mCtx = nullptr;
	Stacks                                           mStacks;
	ci::Rectf                                        mBounds{0, 0, 0, 0};
	Paints                                           mPaints;
	std::unordered_map<GLuint, ci::gl::Texture2dRef> mTextures;
	std::unordered_map<GLuint, Path>                 mPaths;
	float                                            mOpacity{1};
};

//! SVG Circle element: http://www.w3.org/TR/SVG/shapes.html#CircleElement
class SvgCircle : public SvgNode {
  public:
	SvgCircle(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgCircle(SvgNode* parent, const ci::XmlTree& xml);

	glm::vec2 getCenter() const { return mCenter; }
	void	  setCenter(const glm::vec2& center) { mCenter = center; }
	float	  getRadius() const { return mRadius; }
	void	  setRadius(float radius) { mRadius = radius; }

	bool isVisible() const override {
		// A value of zero disables rendering of the element.
		if (mRadius <= 0) return false;
		return SvgNode::isVisible();
	}

	[[nodiscard]] bool containsPoint(const glm::vec2& pt) const override {
		return distance2(pt, mCenter) < mRadius * mRadius;
	}

	[[nodiscard]] ci::Shape2d getShape() const override;

  protected:
	void renderSelf(Renderer& renderer) const override;

	[[nodiscard]] ci::Rectf calcBoundingBox() const override {
		return {mCenter.x - mRadius, mCenter.y - mRadius, mCenter.x + mRadius, mCenter.y + mRadius};
	}

	glm::vec2 mCenter{0};
	float	  mRadius{1};
};

//! SVG Ellipse element: http://www.w3.org/TR/SVG/shapes.html#EllipseElement
class SvgEllipse : public SvgNode {
  public:
	explicit SvgEllipse(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgEllipse(SvgNode* parent, const ci::XmlTree& xml);

	glm::vec2 getCenter() const { return mCenter; }
	void	  setCenter(const glm::vec2& center) { mCenter = center; }
	float	  getRadiusX() const { return mRadiusX; }
	void	  setRadiusX(float radiusX) { mRadiusX = radiusX; }
	float	  getRadiusY() const { return mRadiusY; }
	void	  setRadiusY(float radiusY) { mRadiusY = radiusY; }

	bool isVisible() const override {
		// A value of zero disables rendering of the element.
		if (mRadiusX <= 0 || mRadiusY <= 0) return false;
		return SvgNode::isVisible();
	}

	bool containsPoint(const glm::vec2& pt) const override;

	ci::Shape2d getShape() const override;

  protected:
	void renderSelf(Renderer& renderer) const override;

	ci::Rectf calcBoundingBox() const override {
		return {mCenter.x - mRadiusX, mCenter.y - mRadiusY, mCenter.x + mRadiusX, mCenter.y + mRadiusY};
	}

	glm::vec2 mCenter{0};
	float	  mRadiusX{1}, mRadiusY{1};
};

//! SVG Path element: http://www.w3.org/TR/SVG/paths.html#PathElement
class SvgPath : public SvgNode {
  public:
	SvgPath(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgPath(SvgNode* parent, const ci::XmlTree& xml);

	const ci::Shape2d& getShape2d() const { return mPath; }
	void			   appendShape2d(ci::Shape2d* appendTo) const;

	bool containsPoint(const glm::vec2& pt) const override { return mPath.contains(pt); }

	ci::Shape2d getShape() const override { return mPath; }
	void		setShape(const ci::Shape2d& shape) { mPath = shape; }

  protected:
	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override { return mPath.calcPreciseBoundingBox(); }

	ci::Shape2d mPath;
};

//! SVG Line element: http://www.w3.org/TR/SVG/shapes.html#LineElement
class SvgLine : public SvgNode {
  public:
	SvgLine(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgLine(SvgNode* parent, const ci::XmlTree& xml);

	const glm::vec2& getPoint1() const { return mPoint1; }
	const glm::vec2& getPoint2() const { return mPoint2; }

	ci::Shape2d getShape() const override;

  protected:
	void renderSelf(Renderer& renderer) const override;

	ci::Rectf calcBoundingBox() const override {
		return {std::min(mPoint1.x, mPoint2.x), std::min(mPoint1.y, mPoint2.y), std::max(mPoint1.x, mPoint2.x),
				std::max(mPoint1.y, mPoint2.y)};
	}

	glm::vec2 mPoint1{0}, mPoint2{0};
};

//! SVG Rect element: http://www.w3.org/TR/SVG/shapes.html#RectElement
class SvgRect : public SvgNode {
  public:
	SvgRect(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgRect(SvgNode* parent, const ci::XmlTree& xml);

	const ci::Rectf& getRect() const { return mRect; }
	void			 setRect(const ci::Rectf& rect) { mRect = rect; }
	void			 setWidth(float width) { mRect = ci::Rectf(mRect.x1, mRect.y1, mRect.x1 + width, mRect.y2); }
	void			 setHeight(float height) { mRect = ci::Rectf(mRect.x1, mRect.y1, mRect.x2, mRect.y1 + height); }

	float getRx() const;
	float getRy() const;

	bool isVisible() const override {
		// A value of zero disables rendering of the element.
		if (mRect.getWidth() <= 0 || mRect.getHeight() <= 0) return false;
		return SvgNode::isVisible();
	}

	bool containsPoint(const glm::vec2& pt) const override { return mRect.contains(pt); }

	ci::Shape2d getShape() const override;

  protected:
	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override { return mRect; }

	ci::Rectf mRect;
	Value	  mRx{0}; // Defaults to 'auto'.
	Value	  mRy{0}; // Defaults to 'auto'.
};

//! SVG Polygon Element: http://www.w3.org/TR/SVG/shapes.html#PolygonElement
class SvgPolygon : public SvgNode {
  public:
	explicit SvgPolygon(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgPolygon(SvgNode* parent, const ci::XmlTree& xml);

	const ci::PolyLine2f& getPolyLine() const { return mPolyLine; }
	ci::PolyLine2f&		  getPolyLine() { return mPolyLine; }

	bool containsPoint(const glm::vec2& pt) const override { return mPolyLine.contains(pt); }

	ci::Shape2d getShape() const override;

  protected:
	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override { return ci::Rectf(mPolyLine.getPoints()); }

	ci::PolyLine2f mPolyLine;
};

//! SVG Polyline Element: http://www.w3.org/TR/SVG/shapes.html#PolylineElement
class SvgPolyline : public SvgNode {
  public:
	explicit SvgPolyline(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgPolyline(SvgNode* parent, const ci::XmlTree& xml);

	const ci::PolyLine2f& getPolyLine() const { return mPolyLine; }
	ci::PolyLine2f&		  getPolyLine() { return mPolyLine; }

	bool containsPoint(const glm::vec2& pt) const override { return mPolyLine.contains(pt); }

	ci::Shape2d getShape() const override;

  protected:
	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override { return ci::Rectf(mPolyLine.getPoints()); }

	ci::PolyLine2f mPolyLine;
};

//! SVG Use Element, which instantiates a different element: http://www.w3.org/TR/SVG/struct.html#UseElement
class SvgUse : public SvgNode {
  public:
	SvgUse(SvgNode* parent, const ci::XmlTree& xml);

	bool isDrawable() const override { return false; }

	bool isDisplayNone() const override {
		return SvgNode::isDisplayNone() || (mReferenced ? mReferenced->isDisplayNone() : false);
	}

	ci::Shape2d getShape() const override {
		if (mReferenced) return mReferenced->getShape();
		return {};
	}

  protected:
	void renderSelf(Renderer& renderer) const override;

	ci::Rectf calcBoundingBox() const override {
		if (mReferenced) return mReferenced->getBoundingBox();
		return {0, 0, 0, 0};
	}

	void parse(const ci::XmlTree& xml);

	const SvgNode* mReferenced = nullptr;
};

//! SVG Image Element. Represents an unpremultiplied bitmap. http://www.w3.org/TR/SVG/struct.html#ImageElement
class SvgImage : public SvgNode {
  public:
	SvgImage()
	  : SvgNode(nullptr) {}
	SvgImage(SvgNode* parent, const ci::XmlTree& xml);

	operator bool() const { return bool(mImage) || bool(mSvg); }

	const ci::Rectf& getRect() const { return mBounds; }

	std::shared_ptr<ci::Surface8u> getSurface() const { return mImage; }
	std::shared_ptr<SvgDoc>		   getSvg() const { return mSvg; }

	bool containsPoint(const glm::vec2& pt) const override { return mBounds.contains(pt); }

	//! Returns a transformation matrix for the texture coordinates.
	const glm::mat3& getTextureMatrix() const { return mTextureMatrix; }

  protected:
	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override { return mBounds; }

	bool parseDataImage(const std::string& data);

	ci::Rectf					   mBounds;
	std::shared_ptr<SvgDoc>		   mSvg;
	std::shared_ptr<ci::Surface8u> mImage;
	glm::mat3					   mTextureMatrix;
};

using TextSpanRef = std::shared_ptr<SvgTextSpan>;

//! SVG tspan Element. Generally owned by a svg::Text Node. http://www.w3.org/TR/SVG/text.html#TSpanElement
class SvgTextSpan : public SvgNode {
  public:
	class Attributes {
	  public:
		Attributes() = default;
		Attributes(const ci::XmlTree& xml);

		void startRender(Renderer& renderer) const;
		void finishRender(Renderer& renderer) const;

		void setTextPen(const glm::vec2& textPen);

		Value			   mX{0}, mY{0};
		float			   mDx{}, mDy{};
		std::vector<Value> mRotate;
		float			   mTextLength{};
		float			   mLengthAdjust{};
		std::vector<Value> mLetterSpacing;
	};

	SvgTextSpan(SvgNode* parent, const ci::XmlTree& xml);
	SvgTextSpan(SvgNode* parent, const std::string& spanString);

	const std::string&		  getString() const { return mString; }
	void					  setString(const std::string& s) { mString = s; }
	std::shared_ptr<ci::Font> getFont() const;
	//! Returns a vector of glyph IDs and positions for the string, ignoring rotation. Cached and lazily calculated.
	std::vector<std::pair<uint16_t, glm::vec2>> getGlyphMeasures() const;
	glm::vec2									getTextPen() const;
	void										setTextPen(const glm::vec2& textPen);
	float										getRotation() const;
	Value										getLetterSpacing() const;

	std::vector<TextSpanRef>&		getSpans() { return mSpans; }
	const std::vector<TextSpanRef>& getSpans() const { return mSpans; }

  protected:
	void renderSelf(Renderer& renderer) const override;

	bool		mIgnoreAttributes; // TextSpans that are actually the contents of Text's attributes should be ignored
	Attributes	mAttributes;
	std::string mString;
	mutable std::shared_ptr<ci::Font>									 mFont;
	mutable std::shared_ptr<std::vector<std::pair<uint16_t, glm::vec2>>> mGlyphMeasures;
	mutable std::shared_ptr<ci::Shape2d>								 mShape;

	std::vector<TextSpanRef> mSpans;

	friend class SvgText;
};

//! SVG Text element. http://www.w3.org/TR/SVG/text.html#TextElement
class SvgText : public SvgNode {
  public:
	SvgText(SvgNode* parent, const ci::XmlTree& xml);

	glm::vec2 getTextPen() const;
	void	  setTextPen(const glm::vec2& textPen) { mAttributes.setTextPen(textPen); }
	float	  getRotation() const;
	Value	  getLetterSpacing() const;

	std::vector<TextSpanRef>&		getSpans() { return mSpans; }
	const std::vector<TextSpanRef>& getSpans() const { return mSpans; }

  protected:
	void renderSelf(Renderer& renderer) const override;

	SvgTextSpan::Attributes	 mAttributes;
	std::vector<TextSpanRef> mSpans;
};

//! Represents a group of SVG elements. http://www.w3.org/TR/SVG/struct.html#Groups
class SvgGroup : public SvgNode, private ci::Noncopyable {
  public:
	explicit SvgGroup(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgGroup(SvgNode* parent, const ci::XmlTree& xml);
	~SvgGroup() override;

	//! Recursively searches for a child element of type <tt>svg::T</tt> named \a id. Returns NULL on failure to
	//! find the object or if it is not of type T.
	template <typename T>
	const T* find(const std::string& id) const {
		return dynamic_cast<const T*>(findNode(id));
	}
	//! Recursively searches for a child element of type <tt>svg::T</tt> named \a id. Returns NULL on failure to
	//! find the object or if it is not of type T.
	template <typename T>
	T* find(const std::string& id) {
		return dynamic_cast<T*>(findNode(id));
	}
	//! Recursively searches for a child element named \a id. Returns NULL on failure.
	virtual const SvgNode* findNode(const std::string& id, bool recurse = true) const;
	//! Recursively searches for a child element named \a id. Returns NULL on failure.
	SvgNode* findNode(const std::string& id, bool recurse = true) {
		return const_cast<SvgNode*>(const_cast<const SvgGroup*>(this)->findNode(id, recurse));
	}
	//! Recursively searches for a child element of type <tt>svg::T</tt> whose name contains \a idPartial. Returns
	//! NULL on failure to find the object or if it is not of type T.
	template <typename T>
	const T* findByIdContains(const std::string& idPartial) const {
		return dynamic_cast<const T*>(findNodeByIdContains(idPartial));
	}
	//! Recursively searches for a child element whose name contains \a idPartial. Returns NULL on failure.
	//! (null_ptr later?)
	const SvgNode* findNodeByIdContains(const std::string& idPartial, bool recurse = true) const;
	//! Recursively searches for a child element with the specified \a tag. Returns NULL on failure.
	virtual const SvgNode* findNodeByTag(const std::string& tag, bool recurse = true) const;
	//! Finds the node with ID \a elementId amongst this Node's ancestors. Returns NULL on failure.
	const SvgNode* findInAncestors(const std::string& elementId) const override;
	//! Recursively searches for a <tag> node and returns the first it finds. Returns NULL on failure.
	const SvgNode* findTagInAncestors(const std::string& elementTag) const override;
	//! Returns a reference to the child named \a id. Throws svg::ExcChildNotFound if not found.
	const SvgNode& getChild(const std::string& id) const;
	//! Returns a reference to the child named \a id. Throws svg::ExcChildNotFound if not found.
	SvgNode& getChild(const std::string& id) {
		return const_cast<SvgNode&>(const_cast<const SvgGroup*>(this)->getChild(id));
	}
	//! Returns a reference to the child named \a id. Throws svg::ExcChildNotFound if not found.
	const SvgNode& operator/(const std::string& id) const { return getChild(id); }

	//! Returns the merged Shape2d for all children of the group
	ci::Shape2d getShape() const override { return getMergedShape2d(); }

	//! Appends the merged Shape2d for the group to \a appentTo.
	void appendMergedShape2d(ci::Shape2d* appendTo) const;

	//! Returns a reference to the list of the Group's children.
	const std::list<SvgNode*>& getChildren() const { return mChildren; }
	//! Returns a reference to the list of the Group's children.
	std::list<SvgNode*>& getChildren() { return mChildren; }
	//! Returns a reference to the child at \a index. Throws svg::ExcChildNotFound if \a index is out of range.
	const SvgNode& getChild(size_t index) const;
	//! Returns a reference to the child at \a index. Throws svg::ExcChildNotFound if \a index is out of range.
	SvgNode& getChild(size_t index) { return const_cast<SvgNode&>(const_cast<const SvgGroup*>(this)->getChild(index)); }

	//! Recursively iterates all Nodes in Group or Doc, passing each to \a fn to be optionally manipulated
	virtual void iterate(const std::function<void(SvgNode*)>& fn);

	//!
	static SvgNode* create(SvgNode* parent, const ci::XmlTree& xml);

	bool isDrawable() const override { return false; }

  protected:
	SvgNode*	nodeUnderPoint(const glm::vec2& absolutePoint, const glm::mat3& parentInverseMatrix) const;
	ci::Shape2d getMergedShape2d() const;

	void	  renderSelf(Renderer& renderer) const override;
	ci::Rectf calcBoundingBox() const override;

	virtual void parse(const ci::XmlTree& xml);

	std::list<SvgNode*> mChildren;

	friend class SvgNode;
};

//!
class SvgDefs : public SvgGroup {
  public:
	SvgDefs(SvgNode* parent)
	  : SvgGroup(parent) {}
	SvgDefs(SvgNode* parent, const ci::XmlTree& xml);

	const SvgNode* findNode(const std::string& id, bool recurse) const override;

  protected:
	void renderSelf(Renderer& renderer) const override { /* never render */
	}

	ci::Rectf calcBoundingBox() const override { return {0, 0, 0, 0}; }

	ci::XmlTree mXml;
};

//! SVG ClipPath element: https://www.w3.org/TR/SVG/render.html#ClippingAndMasking
class SvgClipPath : public SvgGroup {
  public:
	explicit SvgClipPath(SvgNode* parent)
	  : SvgGroup(parent) {}
	SvgClipPath(SvgNode* parent, const ci::XmlTree& xml);

	bool useObjectBoundingBox() const { return mUseObjectBoundingBox; }

	//! Returns whether all children are set to 'display="none"'.
	bool isDisplayNone() const override { return mIsDisplayNone; }

  protected:
	void renderSelf(Renderer& renderer) const override { /* never render */
	}

	bool mUseObjectBoundingBox = false;
	bool mIsDisplayNone		   = true;
};

//!
class SvgStyles : public SvgNode {
  public:
	explicit SvgStyles(SvgNode* parent)
	  : SvgNode(parent) {}
	SvgStyles(SvgNode* parent, const ci::XmlTree& xml);

	bool   empty() const { return mStyleList.empty(); }
	size_t size() const { return mStyleList.size(); }

	Style findStyle(const std::string& id) const;

	void renderSelf(Renderer& renderer) const override {}

  protected:
	std::unordered_map<std::string, Style> mStyleList;
};

//! Represents an SVG Document. See SVG Document Structure http://www.w3.org/TR/SVG/struct.html
class SvgDoc : public SvgGroup {
  public:
	SvgDoc();
	SvgDoc(SvgNode* parent, const ci::XmlTree& xml);
	SvgDoc(const ci::fs::path& filePath);
	SvgDoc(const ci::DataSourceRef& dataSource, const ci::fs::path& filePath = ci::fs::path());
	SvgDoc(SvgNode* parent, const ci::DataSourceRef& dataSource, const ci::fs::path& filePath = ci::fs::path());

	static SvgDocRef create(SvgNode* parent, const ci::XmlTree& xml);
	static SvgDocRef create(const ci::fs::path& filePath);
	static SvgDocRef create(const ci::DataSourceRef& dataSource, const ci::fs::path& filePath = ci::fs::path());
	static SvgDocRef create(SvgNode* parent, const ci::DataSourceRef& dataSource,
							const ci::fs::path& filePath = ci::fs::path());
	static SvgDocRef createFromSvgz(const ci::DataSourceRef& dataSource, const ci::fs::path& filePath = ci::fs::path());

	//! Returns the file path of the document. Can be relative or empty.
	const ci::fs::path& getFilePath() const { return mFilePath; }

	//! Returns the width of the document in pixels
	float getWidth() const { return mBounds.getWidth(); }
	//! Returns the height of the document in pixels
	float getHeight() const { return mBounds.getHeight(); }
	//! Returns the size of the document in pixels
	glm::vec2 getSize() const { return {getWidth(), getHeight()}; }
	//! Returns the aspect ratio of the Doc (width / height)
	float getAspectRatio() const { return getWidth() / getHeight(); }
	//! Returns the bounds of the Doc (0,0,width,height)
	const ci::Rectf& getBounds() const { return mBounds; }

	//! Returns the document's dots-per-inch. Currently hardcoded to 72.
	static float getDpi() { return 72.0f; }

	//! Returns the top-most Node which contains \a pt. Returns NULL if no Node contains the point.
	SvgNode* nodeUnderPoint(const glm::vec2& pt) const;

	//! Utility function to load an image relative to the document. Caches results.
	std::shared_ptr<ci::Surface8u> loadImage(const ci::fs::path& relativePath) const;

  private:
	void loadDoc(const ci::XmlTree& xml);
	void loadDoc(const ci::DataSourceRef& source, const ci::fs::path& filePath);

	void renderSelf(Renderer& renderer) const override;

	mutable std::map<ci::fs::path, std::shared_ptr<ci::Surface8u>> mImageCache;

	ci::fs::path mFilePath;
	ci::Rectf	 mBounds;
	ci::Rectf	 mViewBox;
};

//! SVG Exception base-class
class SvgExc : public ci::Exception {};

class SvgValueExc : public SvgExc {};

class SvgFloatParseExc : public SvgExc {};

class SvgPathParseExc : public SvgExc {};

class SvgTransformParseExc : public SvgExc {};

class SvgChildNotFoundExc : public SvgExc {
  public:
	SvgChildNotFoundExc(const std::string& child);
};


} // namespace nvpath::svg