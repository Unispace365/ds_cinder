#include "stdafx.h"
#include "flexbox_parser.h"
#include "Poco\String.h"
#include "Poco\UTF8String.h"
#include "Poco\NumberParser.h"
#define PARSER(x) 
namespace ds::ui {
	std::unordered_map < std::string, std::function<bool(YGNodeRef, std::string, bool)>> FlexboxParser::parsers({
			{"direction",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "ltr") {
					YGNodeStyleSetDirection(node, YGDirectionLTR);
				}
				else if (value == "rtl") {
					YGNodeStyleSetDirection(node, YGDirectionRTL);
				}
				else if (value == "inherit") {
					YGNodeStyleSetDirection(node, YGDirectionInherit);
				}
				else {
					DS_LOG_ERROR("could not parse direction value.");
					return false;
				}
				return true;
				}
			},
			{"flex-direction",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "row") {
					YGNodeStyleSetFlexDirection(node, YGFlexDirectionRow);
				}
				else if (value == "row-reverse") {
					YGNodeStyleSetFlexDirection(node, YGFlexDirectionRowReverse);
				}
				else if (value == "column") {
					YGNodeStyleSetFlexDirection(node, YGFlexDirectionColumn);
				}
				else if (value == "column-reverse") {
					YGNodeStyleSetFlexDirection(node, YGFlexDirectionColumnReverse);
				}
				else {
					DS_LOG_ERROR("could not parse flex-direction value.");
					return false;
				}
				return true;
				}
			},
			{"flex-wrap",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "wrap") {
					YGNodeStyleSetFlexWrap(node, YGWrapWrap);
				}
				else if (value == "nowrap") {
					YGNodeStyleSetFlexWrap(node, YGWrapNoWrap);
				}
				else if (value == "wrap-reverse") {
					YGNodeStyleSetFlexWrap(node, YGWrapWrapReverse);
				}
				else {
					DS_LOG_ERROR("could not parse flex-wrap value.");
					return false;
				}
				return true;
				}
			},
			{"flex-flow",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				
				DS_LOG_ERROR("flex-flow is not implemented, please use flex-wrap and flex-direction.");
				return false;
				
				}
			},
			{"justify-content",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "center") {
					YGNodeStyleSetJustifyContent(node, YGJustifyCenter);
				}
				else if (value == "flex-start") {
					YGNodeStyleSetJustifyContent(node, YGJustifyFlexStart);
				}
				else if (value == "flex-end") {
					YGNodeStyleSetJustifyContent(node, YGJustifyFlexEnd);
				}
				else if (value == "space-around") {
					YGNodeStyleSetJustifyContent(node, YGJustifySpaceAround);
				}
				else if (value == "space-between") {
					YGNodeStyleSetJustifyContent(node, YGJustifySpaceBetween);
				}
				else {
					DS_LOG_ERROR("could not parse flex-wrap value");
					return false;
				}
				return true;
				}
			},
			{"align-items",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "center") {
					YGNodeStyleSetAlignItems(node, YGAlignCenter);
				}
				else if (value == "flex-start") {
					YGNodeStyleSetAlignItems(node, YGAlignFlexStart);
				}
				else if (value == "flex-end") {
					YGNodeStyleSetAlignItems(node, YGAlignFlexEnd);
				}
				else if (value == "stretch") {
					YGNodeStyleSetAlignItems(node, YGAlignStretch);
				}
				else if (value == "baseline") {
					YGNodeStyleSetAlignItems(node, YGAlignBaseline);
				}
				else {
					DS_LOG_ERROR("could not parse align-items value");
					return false;
				}
				return true;
				}
			},
			{"align-content",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "center") {
					YGNodeStyleSetAlignContent(node, YGAlignCenter);
				}
				else if (value == "flex-start") {
					YGNodeStyleSetAlignContent(node, YGAlignFlexStart);
				}
				else if (value == "flex-end") {
					YGNodeStyleSetAlignContent(node, YGAlignFlexEnd);
				}
				else if (value == "stretch") {
					YGNodeStyleSetAlignContent(node, YGAlignStretch);
				}
				else if (value == "space-between") {
					YGNodeStyleSetAlignContent(node, YGAlignSpaceBetween);
				}
				else if (value == "space-around") {
					YGNodeStyleSetAlignContent(node, YGAlignSpaceAround);
				}
				else {
					DS_LOG_ERROR("could not parse align-content value");
					return false;
				}
				return true;
				}
			},
			{"flex-grow",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
					YGNodeStyleSetFlexGrow(node, number);
				}
				else {
					DS_LOG_ERROR("could not parse flex-grow value");
					return false;
				}
				return true;
				}
			},
			{"flex-shrink",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
					YGNodeStyleSetFlexShrink(node, number);
				}
				else {
					DS_LOG_ERROR("could not parse flex-shrink value");
					return false;
				}
				return true;
				}
			},
			{"flex",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
					YGNodeStyleSetFlex(node, number);
				}
				else {
					DS_LOG_ERROR("could not parse flex value");
					return false;
				}
				return true;
				}
			},
			{"flex-basis",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (value == "auto") {
					YGNodeStyleSetFlexBasisAuto(node);
				} 
				else if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetFlexBasis(node, number);
					}
					else {
						YGNodeStyleSetFlexBasisPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse flex-basis value");
					return false;
				}
				return true;
				}
			},
			{"align-self",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "center") {
					YGNodeStyleSetAlignSelf(node, YGAlignCenter);
				}
				else if (value == "flex-start") {
					YGNodeStyleSetAlignSelf(node, YGAlignFlexStart);
				}
				else if (value == "flex-end") {
					YGNodeStyleSetAlignSelf(node, YGAlignFlexEnd);
				}
				else if (value == "stretch") {
					YGNodeStyleSetAlignSelf(node, YGAlignStretch);
				}
				else if (value == "baseline") {
					YGNodeStyleSetAlignSelf(node, YGAlignBaseline);
				}
				else {
					DS_LOG_ERROR("could not parse align-self value");
					return false;
				}
				return true;
				}
			},
			{"width",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (value == "auto") {
					YGNodeStyleSetWidthAuto(node);
				} 
				else if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetWidth(node, number);
					}
					else {
						YGNodeStyleSetWidthPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse width value");
					return false;
				}
				return true;
				}
			},
			{"height",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (value == "auto") {
					YGNodeStyleSetHeightAuto(node);
				} 
				else if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetHeight(node, number);
					}
					else {
						YGNodeStyleSetHeightPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse height value");
					return false;
				}
				return true;
				}
			},
			{"max-width",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetMaxWidth(node, number);
					}
					else {
						YGNodeStyleSetMaxWidthPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse max-width value");
					return false;
				}
				return true;
				}
			},
			{"max-height",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
			
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetMaxHeight(node, number);
					}
					else {
						YGNodeStyleSetMaxHeightPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse max-height value");
					return false;
				}
				return true;
				}
			},
			{"min-width",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetMinWidth(node, number);
					}
					else {
						YGNodeStyleSetMinWidthPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse min-width value");
					return false;
				}
				return true;
				}
			},
			{"min-height",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
			
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (!pct) {
						YGNodeStyleSetMinHeight(node, number);
					}
					else {
						YGNodeStyleSetMinHeightPercent(node, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse min-height value");
					return false;
				}
				return true;
				}
			},
			{"aspect-ratio",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				if (value == "auto") {
					YGNodeStyleSetAspectRatio(node,YGUndefined);
				} 
				else if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
						YGNodeStyleSetAspectRatio(node, number);
				}
				else {
					DS_LOG_ERROR("could not parse aspect-ratio value");
					return false;
				}
				return true;
				}
			},
			{"position-type",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				if (value == "absolute") {
					YGNodeStyleSetPositionType(node, YGPositionTypeAbsolute);
				}
				else if (value == "relative") {
					YGNodeStyleSetPositionType(node, YGPositionTypeRelative);
				}
				else {
					DS_LOG_ERROR("could not parse position-type value");
					return false;
				}
				return true;
				}
			},
			{"position-top",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPaddingPercent(node, YGEdgeTop, number);
					}
					else {
						YGNodeStyleSetPadding(node, YGEdgeTop, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-top value");
					return false;
				}
				return true;
				}
			},
			{"position-bottom",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPaddingPercent(node, YGEdgeBottom, number);
					}
					else {
						YGNodeStyleSetPadding(node, YGEdgeBottom, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-bottom value");
					return false;
				}
				return true;
				}
			},
			{"position-left",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPaddingPercent(node, YGEdgeLeft, number);
					}
					else {
						YGNodeStyleSetPadding(node, YGEdgeLeft, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-left value");
					return false;
				}
				return true;
				}
			},
			{"position-right",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPaddingPercent(node, YGEdgeRight, number);
					}
					else {
						YGNodeStyleSetPadding(node, YGEdgeRight, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-right value");
					return false;
				}
				return true;
				}
			},
			{"margin-top",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetMarginPercent(node, YGEdgeTop, number);
					}
					else {
						YGNodeStyleSetMargin(node, YGEdgeTop, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse margin-top value");
					return false;
				}
				return true;
				}
			},
			{"margin-bottom",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetMarginPercent(node, YGEdgeBottom, number);
					}
					else {
						YGNodeStyleSetMargin(node, YGEdgeBottom, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse margin-bottom value");
					return false;
				}
				return true;
				}
			},
			{"margin-left",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetMarginPercent(node, YGEdgeLeft, number);
					}
					else {
						YGNodeStyleSetMargin(node, YGEdgeLeft, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse margin-left value");
					return false;
				}
				return true;
				}
			},
			{"margin-right",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetMarginPercent(node, YGEdgeRight, number);
					}
					else {
						YGNodeStyleSetMargin(node, YGEdgeRight, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse margin-right value");
					return false;
				}
				return true;
				}
			},
			{"border-top",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
						YGNodeStyleSetBorder(node, YGEdgeTop, number);
				}
				else {
					DS_LOG_ERROR("could not parse border-top value");
					return false;
				}
				return true;
				}
			},
			{"border-bottom",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
						YGNodeStyleSetBorder(node, YGEdgeBottom, number);
				}
				else {
					DS_LOG_ERROR("could not parse border-bottom value");
					return false;
				}
				return true;
				}
			},
			{"border-left",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
						YGNodeStyleSetBorder(node, YGEdgeLeft, number);
				}
				else {
					DS_LOG_ERROR("could not parse border-left value");
					return false;
				}
				return true;
				}
			},
			{"border-right",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (!pct && Poco::NumberParser::tryParseFloat(value, number)) {
						YGNodeStyleSetBorder(node, YGEdgeRight, number);
				}
				else {
					DS_LOG_ERROR("could not parse border-right value");
					return false;
				}
				return true;
				}
			},
			{"position-top",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPositionPercent(node, YGEdgeTop, number);
					}
					else {
						YGNodeStyleSetPosition(node, YGEdgeTop, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-top value");
					return false;
				}
				return true;
				}
			},
			{"position-bottom",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPositionPercent(node, YGEdgeBottom, number);
					}
					else {
						YGNodeStyleSetPosition(node, YGEdgeBottom, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-bottom value");
					return false;
				}
				return true;
				}
			},
			{"position-left",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPositionPercent(node, YGEdgeLeft, number);
					}
					else {
						YGNodeStyleSetPosition(node, YGEdgeLeft, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-left value");
					return false;
				}
				return true;
				}
			},
			{"position-right",
			[](YGNodeRef node,std::string value,bool pct)->bool {
				double number;
				
				if (Poco::NumberParser::tryParseFloat(value, number)) {
					if (pct) {
						YGNodeStyleSetPositionPercent(node, YGEdgeRight, number);
					}
					else {
						YGNodeStyleSetPosition(node, YGEdgeRight, number);
					}
				}
				else {
					DS_LOG_ERROR("could not parse position-right value");
					return false;
				}
				return true;
				}
			},
			
		});

	bool FlexboxParser::parseProperty(std::string property, std::string value, YGNodeRef node) {

		auto [cleanedValue, isPercent] = cleanValue(value);

		if (parsers.find(property) != parsers.end()) {
			return parsers[property](node, cleanedValue, isPercent);
		}
		else {
			DS_LOG_ERROR("Could not find parser for property named '" << property << "'");
		}
	}

	
	std::tuple<std::string, bool> FlexboxParser::cleanValue(std::string value)
	{
		auto original = value;
		Poco::removeInPlace(value, '\n');
		Poco::trimInPlace(value);
		Poco::UTF8::toLowerInPlace(value);
		

		bool isPercent = (value.length()>1 && value.compare(value.length()-1,1,"%") == 0);
		bool isPixel = (value.length()>2 && value.compare(value.length() - 2, 2, "px") == 0);
		if (isPercent) value = value.substr(0, value.length() - 1);
		if (isPixel) value = value.substr(0, value.length() - 2);
		return { value,isPercent };
	}


}