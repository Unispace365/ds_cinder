#include "stdafx.h"

#include "interface_xml_importer.h"

#include "stylesheet_parser.h"

#include <ds/app/engine/engine.h>
#include <ds/app/event_registry.h>
#include <ds/app/event.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/gradient_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/image_with_thumbnail.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/control/control_check_box.h>
#include <ds/ui/control/control_slider.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/centered_scroll_area.h>
#include <ds/ui/scroll/scroll_list.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/soft_keyboard/soft_keyboard_builder.h>
#include <ds/ui/sprite/border.h>
#include <ds/ui/sprite/circle.h>
#include <ds/ui/sprite/circle_border.h>
#include <ds/ui/sprite/donut_arc.h>
#include <ds/ui/sprite/dashed_line.h>
#include <ds/util/string_util.h>
#include <ds/util/color_util.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/markdown_to_pango.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include <Poco/Path.h>

#include <typeinfo>
#include <iostream>
#include <fstream>

//#include "exprtk.hpp"

#include "ds/math/fparser.hh"


namespace {

static std::unordered_map<std::string, std::string> VARIABLE_MAP;
static std::unordered_map<std::string, ds::ui::XmlImporter::XmlPreloadData>	 PRELOADED_CACHE;
static bool AUTO_CACHE = false;

// Get the setting if we're caching or not and run it just before server setup
// That way we can clear the cache each time the server setup runs
class Init {
public:
	Init() {
		ds::App::AddServerSetup([](ds::Engine& e) {
			AUTO_CACHE = e.getEngineSettings().getBool("xml_importer:cache");
			PRELOADED_CACHE.clear();

			VARIABLE_MAP.clear();

			VARIABLE_MAP["world_width"] = std::to_string(e.getWorldWidth());
			VARIABLE_MAP["world_height"] = std::to_string(e.getWorldHeight());
			VARIABLE_MAP["world_size"] = ds::unparseVector(ci::vec2(e.getWorldWidth(), e.getWorldHeight()));
			VARIABLE_MAP["anim_dur"] = std::to_string(e.getAnimDur());

			e.getAppSettings().forEachSetting([](const ds::cfg::Settings::Setting& theSetting) {
				ds::ui::XmlImporter::addVariable(theSetting.mName, theSetting.mRawValue);
			});

		});
	}
	void					doNothing() { }
};
Init						INIT;


}

namespace ds {
namespace ui {

static const std::string INVALID_VALUE = "UNACCEPTABLE!!!!";


std::string XmlImporter::getGradientColorsAsString(ds::ui::Gradient* grad){
	if(!grad) return "";
	std::stringstream ss;
	ss << ds::unparseColor(grad->getColorTL()) << ", " << ds::unparseColor(grad->getColorTR()) << ", " << ds::unparseColor(grad->getColorBR()) << ", " << ds::unparseColor(grad->getColorBL());
	return ss.str();
}

namespace {
static const ci::vec3				DEFAULT_SIZE = ci::vec3(0.0f, 0.0f, 1.0f);
static const ci::vec3				DEFAULT_CENTER = ci::vec3(0.0f, 0.0f, 0.0f);
static const ci::vec3				DEFAULT_POS = ci::vec3(0.0f, 0.0f, 0.0f);
static const ci::vec3				DEFAULT_ROT = ci::vec3(0.0f, 0.0f, 0.0f);
static const ci::vec3				DEFAULT_SCALE = ci::vec3(1.0f, 1.0f, 1.0f);
static const ci::Color				DEFAULT_COLOR = ci::Color(1.0f, 1.0f, 1.0f);
static const float					DEFAULT_OPACITY = 1.0f;
static const bool					DEFAULT_VISIBLE = true;
static const bool					DEFAULT_TRANSPARENT = true;
static const bool					DEFAULT_ENABLED = false;
static const bool					DEFAULT_CLIPPING = false;
static const bool					DEFAULT_CHECKBOUNDS = false;
static const ds::ui::BlendMode		DEFAULT_BLENDMODE = ds::ui::NORMAL;
static const float					DEFAULT_LAYOUT_PAD = 0.0f;
static const float					DEFAULT_LAYOUT_SPACING = 0.0f;
static const ci::vec2				DEFAULT_LAYOUT_SIZEFUDGE = ci::vec2();
static const int					DEFAULT_LAYOUT_ALIGN_USERTYPE = 0;
static const ds::ui::LayoutSprite::LayoutType DEFAULT_LAYOUT_TYPE = ds::ui::LayoutSprite::kLayoutNone;
static const ds::ui::LayoutSprite::ShrinkType DEFAULT_SHRINK_TYPE = ds::ui::LayoutSprite::kShrinkNone;
}


// There's a lotta dynamic casts here and such, but this is pretty much a dev-only task.
// Don't expect this to ever really be called in production.
void XmlImporter::getSpriteProperties(ds::ui::Sprite& sp, ci::XmlTree& xml){
	if(!sp.getSpriteName(false).empty()) xml.setAttribute("name", ds::utf8_from_wstr(sp.getSpriteName()));
	if(sp.getSize() != DEFAULT_SIZE) xml.setAttribute("size", unparseVector(sp.getSize()));
	if(sp.getColor() != DEFAULT_COLOR) xml.setAttribute("color", unparseColor(sp.getColor()));
	if(sp.getOpacity() != DEFAULT_OPACITY) xml.setAttribute("opacity", sp.getOpacity());
	if(sp.getPosition() != DEFAULT_POS) xml.setAttribute("position", unparseVector(sp.getPosition()));
	if(sp.getRotation() != DEFAULT_ROT) xml.setAttribute("rotation", unparseVector(sp.getRotation()));
	if(sp.getScale() != DEFAULT_SCALE) xml.setAttribute("scale", unparseVector(sp.getScale()));
	if(sp.getCenter() != DEFAULT_CENTER) xml.setAttribute("center", unparseVector(sp.getCenter()));
	if(sp.getClipping() != DEFAULT_CLIPPING) xml.setAttribute("clipping", unparseBoolean(sp.getClipping()));
	if(sp.getBlendMode() != DEFAULT_BLENDMODE) xml.setAttribute("blend_mode", ds::ui::getStringForBlendMode(sp.getBlendMode()));
	if(sp.isEnabled() != DEFAULT_ENABLED) xml.setAttribute("enable", unparseBoolean(sp.isEnabled()));
	if(!sp.getMultiTouchConstraints().isEmpty()) xml.setAttribute("multitouch", getMultitouchStringForBitMask(sp.getMultiTouchConstraints()));
	if(sp.getTransparent() != DEFAULT_TRANSPARENT) xml.setAttribute("transparent", unparseBoolean(sp.getTransparent()));
	if(!sp.getAnimateOnScript().empty()) xml.setAttribute("animate_on", sp.getAnimateOnScript());
	if(sp.mLayoutTPad != DEFAULT_LAYOUT_PAD) xml.setAttribute("t_pad", sp.mLayoutTPad);
	if(sp.mLayoutBPad != DEFAULT_LAYOUT_PAD) xml.setAttribute("b_pad", sp.mLayoutBPad);
	if(sp.mLayoutLPad != DEFAULT_LAYOUT_PAD) xml.setAttribute("l_pad", sp.mLayoutLPad);
	if(sp.mLayoutRPad != DEFAULT_LAYOUT_PAD) xml.setAttribute("r_pad", sp.mLayoutRPad);
	if(sp.mLayoutFudge != DEFAULT_LAYOUT_SIZEFUDGE) xml.setAttribute("layout_fudge", unparseVector(sp.mLayoutFudge));
	if(sp.mLayoutSize != DEFAULT_LAYOUT_SIZEFUDGE) xml.setAttribute("layout_size", unparseVector(sp.mLayoutSize));
	if(sp.mLayoutUserType != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_size_mode", LayoutSprite::getLayoutSizeModeString(sp.mLayoutUserType));
	if(sp.mLayoutVAlign != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_v_align", LayoutSprite::getLayoutVAlignString(sp.mLayoutVAlign));
	if(sp.mLayoutHAlign != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_h_align", LayoutSprite::getLayoutHAlignString(sp.mLayoutHAlign));
	if(sp.getCornerRadius() > 0.0f) xml.setAttribute("corner_radius", sp.getCornerRadius());

	ds::ui::LayoutSprite* ls = dynamic_cast<ds::ui::LayoutSprite*>(&sp);
	if(ls){
		if(ls->getLayoutType() != DEFAULT_LAYOUT_TYPE) xml.setAttribute("layout_type", LayoutSprite::getLayoutTypeString(ls->getLayoutType()));
		if(ls->getSpacing() != DEFAULT_LAYOUT_SPACING) xml.setAttribute("layout_spacing", ls->getSpacing());
		if(ls->getShrinkToChildren() != DEFAULT_SHRINK_TYPE) xml.setAttribute("shrink_to_children", LayoutSprite::getShrinkToChildrenString(ls->getShrinkToChildren()));
		if(ls->getOverallAlignment() != DEFAULT_SHRINK_TYPE) xml.setAttribute("overall_alignment", LayoutSprite::getLayoutVAlignString(ls->getOverallAlignment()));
	}

	ds::ui::Text* txt = dynamic_cast<ds::ui::Text*>(&sp);
	if(txt){
		if(!txt->getText().empty()) xml.setAttribute("text", txt->getTextAsString());
		if(txt->getConfigName().empty()){
			xml.setAttribute("font_name", txt->getFontFileName());
			xml.setAttribute("font_size", txt->getFontSize());
		} else {
			xml.setAttribute("font", txt->getConfigName());
		}

		if(txt->getConfigName().empty()){
			xml.setAttribute("font_leading", txt->getLeading());
		}

		xml.setAttribute("resize_limit", unparseVector(ci::vec2(txt->getResizeLimitWidth(), txt->getResizeLimitHeight())));
		if(txt->getAlignment() != ds::ui::Alignment::kLeft) xml.setAttribute("text_align", LayoutSprite::getLayoutHAlignString(txt->getAlignment()));
	}

	ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(&sp);
	if(img){
		if(!img->getImageFilename().empty()) xml.setAttribute("filename", ds::Environment::contract(img->getImageFilename()));
		if(img->getCircleCrop()) xml.setAttribute("circle_crop", "true");
	}

	ds::ui::ImageButton* imgB = dynamic_cast<ds::ui::ImageButton*>(&sp);
	if(imgB){
		if(imgB->getNormalImagePath() == imgB->getHighImagePath()){
			if(!imgB->getNormalImagePath().empty()) xml.setAttribute("filename", ds::Environment::contract(imgB->getNormalImagePath()));
		} else {
			if(!imgB->getNormalImagePath().empty()) xml.setAttribute("up_image", ds::Environment::contract(imgB->getNormalImagePath()));
			if(!imgB->getHighImagePath().empty()) xml.setAttribute("down_image", ds::Environment::contract(imgB->getHighImagePath()));
		}
		xml.setAttribute("up_image_color", unparseColor(imgB->getNormalImageColor()));
		xml.setAttribute("down_image_color", unparseColor(imgB->getHighImageColor()));
		xml.setAttribute("btn_touch_padding", imgB->getPad());
	}

	ds::ui::Gradient* grad = dynamic_cast<ds::ui::Gradient*>(&sp);
	if(grad){
		xml.setAttribute("gradientColors", getGradientColorsAsString(grad));
	}

	ds::ui::Circle* circ = dynamic_cast<ds::ui::Circle*>(&sp);
	if(circ){
		xml.setAttribute("filled", unparseBoolean(circ->getFilled()));
		xml.setAttribute("radius", circ->getRadius());
		xml.setAttribute("line_width", circ->getLineWidth());
	}

	ds::ui::Border* border = dynamic_cast<ds::ui::Border*>(&sp);
	if(border){
		if(border->getBorderWidth() != 0.0f) xml.setAttribute("border_width", border->getBorderWidth());
	}

	ds::ui::CircleBorder* circleBorder = dynamic_cast<ds::ui::CircleBorder*>(&sp);
	if(circleBorder){
		if(circleBorder->getBorderWidth() != 0.0f) xml.setAttribute("border_width", circleBorder->getBorderWidth());
	}

	// scroll list
	// scroll area
}

ci::XmlTree XmlImporter::createXmlFromSprite(ds::ui::Sprite& sprite){
	ci::XmlTree newXml = ci::XmlTree(getSpriteTypeForSprite(&sprite), "");
	getSpriteProperties(sprite, newXml);

	auto sprids = sprite.getChildren();
	for(auto it = sprids.begin(); it < sprids.end(); ++it){
		if(!(*it)->mExportWithXml) continue;
		auto xmlly = createXmlFromSprite(*(*it));
		newXml.push_back(xmlly);
	}

	return newXml;
}

std::string XmlImporter::parseExpression(const std::string& theExpr) {

	std::string returny = theExpr;

	FunctionParser fparser;
	fparser.AddConstant("pi", 3.1415926535897932);
	int res = fparser.Parse(theExpr, "");
	if(res > -1) {
		DS_LOG_WARNING("XmlImporter::parseExpression() error parsing: " << fparser.ErrorMsg());
		return "0.0";
	}

	double* vals = {};
	double theResult = fparser.Eval(vals);
	returny = std::to_string(theResult);

	/* Previous parsing library
	 - was really great, but huge compile time hit
	exprtk::expression<double> expr;
	exprtk::parser<double> parser;
	parser.compile(theExpr, expr);

	returny = std::to_string(expr.value());
	*/

	if(returny == "nan") {
		DS_LOG_WARNING("XmlImporter: Experession didn't parse to a number! Using 0.0");
		return "0.0";
	}

	DS_LOG_VERBOSE(2, "XmlImporter: Parsed expression: " << theExpr << " into " << returny);

	return returny;

}

std::string XmlImporter::parseAllExpressions(const std::string& value) {

	auto exprFindy = value.find("#expr{");

	if(exprFindy != std::string::npos) {

		auto theReplacement = value;
		while(exprFindy != std::string::npos) {

			
			std::string beforeReplace = theReplacement.substr(0, exprFindy);


			std::string exprAndAfter = theReplacement.substr(exprFindy);

			auto bracketOpen = exprAndAfter.find("{");
			auto bracketClose = exprAndAfter.find("}");
			if(bracketClose < bracketOpen || bracketClose == std::string::npos) {
				DS_LOG_WARNING("XmlImporter::parseAllExpressions() syntax error with brackets in " << value);
				return beforeReplace + "0.0";
			}

			std::string theExpr = exprAndAfter.substr(bracketOpen + 1, bracketClose - bracketOpen - 1);
			std::string afterExpr = exprAndAfter.substr(bracketClose + 1);

			theExpr = parseExpression(theExpr);

			theReplacement = beforeReplace + theExpr + afterExpr;

			exprFindy = theReplacement.find("#expr{");

		}

		DS_LOG_VERBOSE(3, "XmlImporter::parseAllExpressions from " << value << " into " << theReplacement);

		return theReplacement;

		auto beforeString = value.substr(0, exprFindy);

	} else {
		auto findy = value.find("#expr");
		if(findy != std::string::npos) {
			auto theExpr = value.substr(findy + 5);
			return parseExpression(theExpr);
		}
	}

	return value;
}

std::string XmlImporter::replaceVariables(const std::string& value) {
	if(value.find("$_") != std::string::npos) {
		/// keep track of parses, cause it could get circular
		unsigned int numTries = 0;

		auto theReplacement = value;
		int maxTries = 100000;
		while(numTries < maxTries) {

			auto newReplacement = replaceSingleVariable(theReplacement);

			/// nothing was replaced, we're done
			if(newReplacement == theReplacement) {
				break;
			}

			theReplacement = newReplacement;

			/// No more parameters, skipsies
			if(newReplacement.find("$_") == std::string::npos) {
				break;
			}

			numTries++;
		}

		if(numTries == maxTries) {
			DS_LOG_WARNING("XmlImporter::replaceVariables() tried max tries, that means you have a circular references in your parameters");
		}

		return theReplacement;
		
	}
	return value;
}

std::string XmlImporter::replaceSingleVariable(const std::string& value) {

	auto theStart = value.find("$_");

	if(theStart == std::string::npos) return value;

	auto theEnd = value.find(" ", theStart);

	auto commaEnd = value.find(",", theStart);
	if(commaEnd < theEnd) theEnd = commaEnd;

	auto semiEnd = value.find(";", theStart);
	if(semiEnd < theEnd) theEnd = semiEnd;

	auto brackSta = value.find("{", theStart);
	if(brackSta < theEnd) theEnd = brackSta;

	auto brackEnd = value.find("}", theStart);
	if(brackEnd < theEnd) theEnd = brackEnd;

	auto period = value.find(".", theStart);
	if(period < theEnd) theEnd = period;

	auto quot = value.find("'", theStart);
	if(quot < theEnd) theEnd = quot;

	if(theEnd == std::string::npos) theEnd = value.size();

	auto beforeString = value.substr(0, theStart);
	auto paramName = value.substr(theStart + 2, theEnd - theStart - 2); // ditch the $_
	auto endString = value.substr(theEnd);

	//std::cout << "RSP: " << value << std::endl << "\tBEFORE:" << beforeString << std::endl <<  "\tPARAM:" << paramName << std::endl << "\tAFTER:" << endString << std::endl << "\tInd:" << theStart << " " << theEnd << std::endl;

	auto findy = VARIABLE_MAP.find(paramName);
	if(findy != VARIABLE_MAP.end()) {
		std::string replacement = findy->second;

		return beforeString + replacement + endString;
	} else {
		DS_LOG_WARNING("XmlImporter::replaceSingleVariable() parameter not found! Name=" << paramName);
		return beforeString + endString;
	}

	return value;
}

void XmlImporter::addVariable(const std::string& varName, const std::string& varValue) {
	if(varName.empty()) {
		DS_LOG_WARNING("XmlImporter: No variable name specified when adding variable");
		return;
	}

	VARIABLE_MAP[varName] = varValue;
}

void XmlImporter::setSpriteProperty(ds::ui::Sprite &sprite, ci::XmlTree::Attr &attr, const std::string &referer) {
	std::string property = attr.getName();
	setSpriteProperty(sprite, property, attr.getValue(), referer);
}

void XmlImporter::setSpriteProperty(ds::ui::Sprite &sprite, const std::string& property, const std::string& theValue, const std::string &referer) {
	//Cache the engine for all our color calls
	ds::ui::SpriteEngine& engine = sprite.getEngine();



	DS_LOG_VERBOSE(4, "XmlImporter: setSpriteProperty, prop=" << property << " value=" << theValue << " referer=" << referer);

	// This is a pretty long "case switch" (well, effectively a case switch).
	// It seems like it'd be slow, but in practice, it's relatively fast.
	// The slower parts of this are the actual functions that are called (particularly text setResizeLimit())
	// So be sure that this is actually performing slowly before considering a refactor.

	std::string value = replaceVariables(theValue);
	value = parseAllExpressions(value);

	if(property == "name"){
		sprite.setSpriteName(ds::wstr_from_utf8(value));
	} else if(property == "class") {
		// Do nothing, this is handled by css parsers, if any
	} else if(property == "width") {
		sprite.setSize(ds::string_to_float(value), sprite.getHeight());
	} else if(property == "height") {
		sprite.setSize(sprite.getWidth(), ds::string_to_float(value));
	} else if(property == "depth") {
		sprite.setSizeAll(sprite.getWidth(), sprite.getHeight(), ds::string_to_float(value));
	} else if(property == "size") {
		ci::vec3 v = parseVector(value);
		sprite.setSize(v.x, v.y);
	} else if(property == "color") {
		sprite.setTransparent(false);
		sprite.setColorA(parseColor(value, engine));
	} else if(property == "opacity") {
		sprite.setOpacity(ds::string_to_float(value));
	} else if(property == "position") {
		sprite.setPosition(parseVector(value));
	} else if(property == "rotation") {
		sprite.setRotation(parseVector(value));
	} else if(property == "scale") {
		sprite.setScale(parseVector(value));
	} else if(property == "center") {
		sprite.setCenter(parseVector(value));
	} else if(property == "clipping") {
		sprite.setClipping(parseBoolean(value));
	} else if(property == "blend_mode") {
		sprite.setBlendMode(ds::ui::getBlendModeByString(value));
	} else if(property == "enable"){
		sprite.enable(parseBoolean(value));
	} else if(property == "multitouch"){
		sprite.enableMultiTouch(parseMultitouchMode(value));
	} else if(property == "transparent"){
		sprite.setTransparent(parseBoolean(value));
	} else if(property == "animate_on"){
		sprite.setAnimateOnScript(value);
	} else if(property == "corner_radius"){
		sprite.setCornerRadius(ds::string_to_float(value));
	} else if(property == "t_pad") {
		sprite.mLayoutTPad = ds::string_to_float(value);
	} else if(property == "b_pad") {
		sprite.mLayoutBPad = ds::string_to_float(value);
	} else if(property == "l_pad") {
		sprite.mLayoutLPad = ds::string_to_float(value);
	} else if(property == "r_pad") {
		sprite.mLayoutRPad = ds::string_to_float(value);
	} else if(property == "pad_all") {
		auto pad = ds::string_to_float(value);
		sprite.mLayoutLPad = pad;
		sprite.mLayoutTPad = pad;
		sprite.mLayoutRPad = pad;
		sprite.mLayoutBPad = pad;
	} else if(property == "padding") {
		auto pads = ds::split(value, ", ", true);
		auto count = pads.size();
		sprite.mLayoutLPad = count > 0 ? ds::string_to_float(pads[0]) : 0.0f;
		sprite.mLayoutTPad = count > 1 ? ds::string_to_float(pads[1]) : 0.0f;
		sprite.mLayoutRPad = count > 2 ? ds::string_to_float(pads[2]) : 0.0f;
		sprite.mLayoutBPad = count > 3 ? ds::string_to_float(pads[3]) : 0.0f;
	} else if(property == "layout_size_mode"){
		auto sizeMode = value;
		if(sizeMode == "fixed"){
			sprite.mLayoutUserType = LayoutSprite::kFixedSize;
		} else if(sizeMode == "flex"){
			sprite.mLayoutUserType = LayoutSprite::kFlexSize;
		} else if(sizeMode == "stretch"){
			sprite.mLayoutUserType = LayoutSprite::kStretchSize;
		} else if(sizeMode == "fill"){
			sprite.mLayoutUserType = LayoutSprite::kFillSize;
		} else {
			DS_LOG_WARNING("layout_size_mode set to an invalid value of " << sizeMode);
		}
	} else if(property == "layout_v_align"){
		auto alignMode = value;
		if(alignMode == "top"){
			sprite.mLayoutVAlign = LayoutSprite::kTop;
		} else if(alignMode == "middle"){
			sprite.mLayoutVAlign = LayoutSprite::kMiddle;
		} else if(alignMode == "bottom"){
			sprite.mLayoutVAlign = LayoutSprite::kBottom;
		} else {
			DS_LOG_WARNING("layout_v_align set to an invalid value of " << alignMode);
		}
	} else if(property == "layout_h_align"){
		auto alignMode = value;
		if(alignMode == "left"){
			sprite.mLayoutHAlign = LayoutSprite::kLeft;
		} else if(alignMode == "center"){
			sprite.mLayoutHAlign = LayoutSprite::kCenter;
		} else if(alignMode == "right"){
			sprite.mLayoutHAlign = LayoutSprite::kRight;
		} else {
			DS_LOG_WARNING("layout_h_align set to an invalid value of " << alignMode);
		}
	} else if(property == "layout_fudge"){
		sprite.mLayoutFudge = ci::vec2(parseVector(value));
	} else if(property == "layout_size"){
		sprite.mLayoutSize = ci::vec2(parseVector(value));
	} else if(property == "on_tap_event"){
		sprite.setTapCallback([value](ds::ui::Sprite* bs, const ci::vec3& pos){
			XmlImporter::dispatchStringEvents(value, bs, pos);
		});
	} else if(property == "layout_fixed_aspect"){
		sprite.mLayoutFixedAspect = parseBoolean(value);
	}
	else if(property == "shader") {
		using namespace boost::filesystem;
		boost::filesystem::path fullShaderPath(filePathRelativeTo(referer, value));
		sprite.setBaseShader(fullShaderPath.parent_path().string(), fullShaderPath.filename().string());
	}

	// LayoutSprite specific (the other layout stuff could apply to any sprite)
	else if(property == "layout_type"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		if(layoutSprite){
			auto layoutType = value;
			if(layoutType == "vert"){
				layoutSprite->setLayoutType(LayoutSprite::kLayoutVFlow);
			} else if(layoutType == "horiz"){
				layoutSprite->setLayoutType(LayoutSprite::kLayoutHFlow);
			} else if(layoutType == "size"){
				layoutSprite->setLayoutType(LayoutSprite::kLayoutSize);
			} else {
				layoutSprite->setLayoutType(LayoutSprite::kLayoutNone);
			}
		} else {
			DS_LOG_WARNING("Couldn't set layout_type, as this sprite is not a LayoutSprite.");
		}
	}

	else if(property == "layout_spacing"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		if(layoutSprite){
			layoutSprite->setSpacing(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Couldn't set layout_type, as this sprite is not a LayoutSprite.");
		}
	}
	else if(property == "overall_alignment"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		if(layoutSprite){
			auto alignMode = value;
			if(alignMode == "left" || alignMode == "top"){
				layoutSprite->setOverallAlignment(LayoutSprite::kLeft);
			} else if(alignMode == "center" || alignMode == "middle"){
				layoutSprite->setOverallAlignment(LayoutSprite::kCenter);
			} else if(alignMode == "right" || alignMode == "bottom"){
				layoutSprite->setOverallAlignment(LayoutSprite::kRight);
			} else {
				DS_LOG_WARNING("overall_alignment set to an invalid value of " << alignMode);
			}
		} else {
			DS_LOG_WARNING("Couldn't set overall_alignment, as this sprite is not a LayoutSprite.");
		}
	}
	else if(property == "shrink_to_children"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		if(layoutSprite){
			auto shrinkMode = value;
			if(shrinkMode == "" || shrinkMode == "false" || shrinkMode == "none"){
				layoutSprite->setShrinkToChildren(LayoutSprite::kShrinkNone);
			} else if(shrinkMode == "width"){
				layoutSprite->setShrinkToChildren(LayoutSprite::kShrinkWidth);
			} else if(shrinkMode == "height"){
				layoutSprite->setShrinkToChildren(LayoutSprite::kShrinkHeight);
			} else if(shrinkMode == "true" || shrinkMode == "both"){
				layoutSprite->setShrinkToChildren(LayoutSprite::kShrinkBoth);
			}
		} else {
			DS_LOG_WARNING("Couldn't set shrink_to_children, as this sprite is not a LayoutSprite.");
		}
	}

	// Text specific attributes
	else if(property == "font") {
		// Try to set the font
		auto text = dynamic_cast<Text*>(&sprite);
		if(text) {
			auto cfg = text->getEngine().getEngineCfg().getText(value);
			cfg.configure(*text);
		} else {
			auto controlBox = dynamic_cast<ControlCheckBox*>(&sprite);
			if(controlBox){
				controlBox->setLabelTextConfig(value);
			} else {
				DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
			}
			
		}
	} else if(property == "font_name"){
		auto text = dynamic_cast<Text*>(&sprite);
		if(text) {
			text->setFont(value, text->getFontSize());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
		
	} else if(property == "resize_limit") {
		// Try to set the resize limit
		auto text = dynamic_cast<Text*>(&sprite);
		if(text) {
			auto v = parseVector(value);
			text->setResizeLimit(v.x, v.y);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "text_align"){
		auto text = dynamic_cast<Text*>(&sprite);
		if(text){
			std::string alignString = value;
			if(alignString == "right"){
				text->setAlignment(ds::ui::Alignment::kRight);
			} else if(alignString == "center"){
				text->setAlignment(ds::ui::Alignment::kCenter);
			} else if(alignString == "justify"){
				text->setAlignment(ds::ui::Alignment::kJustify);
			} else {
				text->setAlignment(ds::ui::Alignment::kLeft);
			}
		}

	} else if (property == "text") {
		auto text = dynamic_cast<Text*>(&sprite);
		if(text) {
			text->setText(value);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "markdown") {
		auto text = dynamic_cast<Text*>(&sprite);
		if(text) {
			text->setText(ds::ui::markdown_to_pango(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_size") {
		// Try to set the font size
		auto text = dynamic_cast<Text*>(&sprite);
		if (text) {
			text->setFontSize(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_leading"){
		auto text = dynamic_cast<Text*>(&sprite);
		if(text){
			text->setLeading(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_letter_spacing"){
		auto text = dynamic_cast<Text*>(&sprite);
		if(text){
			text->setLetterSpacing(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_color"){
		auto text = dynamic_cast<Text*>(&sprite);
		if(text){
			text->setColor(parseColor(value, text->getEngine()));
		} else {			
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
			
		}
	}

	// Image properties
	else if(property == "on_click_event"){
		auto imgBtn = dynamic_cast<ImageButton*>(&sprite);
		auto sprBtn = dynamic_cast<SpriteButton*>(&sprite);
		auto layBtn = dynamic_cast<LayoutButton*>(&sprite);
		if(imgBtn){
			imgBtn->setClickFn([imgBtn, value]{
				dispatchStringEvents(value, imgBtn, imgBtn->getGlobalPosition());
			});
		} else if(sprBtn){
			sprBtn->setClickFn([sprBtn, value]{
				dispatchStringEvents(value, sprBtn, sprBtn->getGlobalPosition());
			});
		} else if(layBtn){
			layBtn->setClickFn([layBtn, value]{
				dispatchStringEvents(value, layBtn, layBtn->getGlobalPosition());
			});
		}
	}
	else if(property == "filename" || property == "src" || property == "filename_cache" || property=="src_cache") {
		auto imgBtn = dynamic_cast<ImageButton*>(&sprite);
		auto image = dynamic_cast<Image *>(&sprite);
		int flags = 0;
		if(property == "filename_cache" || property == "src_cache") flags = ds::ui::Image::IMG_CACHE_F;
		std::string filePath = value;
		if(!value.empty()) filePath = filePathRelativeTo(referer, value);
		if(image) {
			image->setImageFile(filePath, flags);
		} else if(imgBtn){
			imgBtn->setNormalImage(filePath, flags);
			imgBtn->setHighImage(filePath, flags);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}
	else if(property == "circle_crop") {
		auto image = dynamic_cast<Image *>(&sprite);
		if(image) {
			image->setCircleCrop(parseBoolean(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}


	// Image Button properties
	else if(property == "down_image") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setHighImage(filePathRelativeTo(referer, value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "up_image") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setNormalImage(filePathRelativeTo(referer, value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "down_image_color"){
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setHighImageColor(parseColor(value, engine));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}		
	} else if(property == "up_image_color"){
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setNormalImageColor(parseColor(value, engine));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}

	} else if(property == "btn_touch_padding") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setTouchPad(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}

	// Gradient sprite properties
	else if(property == "colorTop"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsV(parseColor(value, engine), gradient->getColorBL());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorBot"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsV(gradient->getColorTL(), parseColor(value, engine));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorLeft"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsH(parseColor(value, engine), gradient->getColorTR());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorRight"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsH(gradient->getColorTL(), parseColor(value, engine));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "gradientColors"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			auto colors = ds::split(value, ", ", true);
			if(colors.size() > 3){
				auto colorOne = parseColor(colors[0], engine);
				auto colorTwo = parseColor(colors[1], engine);
				auto colorThr = parseColor(colors[2], engine);
				auto colorFor = parseColor(colors[3], engine);
				gradient->setColorsAll(colorOne, colorTwo, colorThr, colorFor);
			} else {
				DS_LOG_WARNING("Not enough colors supplied for gradientColors");
			}
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}
	// Scroll sprite properties
	else if(property == "scroll_list_layout"){
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		if(scrollList){
			auto vec = parseVector(value);
			scrollList->setLayoutParams(vec.x, vec.y, vec.z, true);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}else if(property == "scroll_list_animate"){
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		if(scrollList){
			auto vec = parseVector(value);
			scrollList->setAnimateOnParams(vec.x, vec.y);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}else if(property == "scroll_fade_colors"){
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		ds::ui::ScrollArea* scrollArea = nullptr;
		if(scrollList){
			scrollArea = scrollList->getScrollArea();
		}

		if(!scrollArea){
			scrollArea = dynamic_cast<ds::ui::ScrollArea*>(&sprite);
		}

		if(scrollArea){
			auto colors = ds::split(value, ", ", true);
			if(colors.size() > 1){
				auto colorOne = parseColor(colors[0],engine);
				auto colorTwo = parseColor(colors[1],engine);
				scrollArea->setFadeColors(colorOne, colorTwo);
			} else {
				DS_LOG_WARNING("Not enough colors specified for scroll_fade_colors ");
			}
		} else {
			DS_LOG_WARNING("Couldn't set scroll_fade_colors for this sprite ");
		}
	}else if (property == "scroll_area_vert"){
		auto scrollArea = dynamic_cast<ds::ui::ScrollArea*>(&sprite);
		if (scrollArea){
			auto vec = parseBoolean(value);
			scrollArea->setVertical(vec);
		}
		else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}
	else if (property == "scroll_fade_size") {
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		ds::ui::ScrollArea* scrollArea = nullptr;
		if (scrollList) {
			scrollArea = scrollList->getScrollArea();
		}

		if (!scrollArea) {
			scrollArea = dynamic_cast<ds::ui::ScrollArea*>(&sprite);
		}

		if (scrollArea) {
			scrollArea->setUseFades(true);
			scrollArea->setFadeHeight(ds::string_to_float(value));
		}
		else {
			DS_LOG_WARNING("Couldn't set scroll_fade_size for this sprite ");
		}
	}

	// Border sprite properties
	else if(property == "border_width"){
		auto border = dynamic_cast<Border*>(&sprite);
		if(border){
			border->setBorderWidth(ds::string_to_float(value));
		} else {
			auto circle_border = dynamic_cast<CircleBorder*>(&sprite);
			if(circle_border){
				circle_border->setBorderWidth(ds::string_to_float(value));
			} else {
				DS_LOG_WARNING("Trying to set border_width on a non-border sprite of type: " << typeid(sprite).name());
			}
		}
	}
	
	// Circle sprite properties
	else if(property == "filled"){
		auto circle = dynamic_cast<Circle*>(&sprite);
		if(circle){
			circle->setFilled(parseBoolean(value));
		} else {
			DS_LOG_WARNING("Trying to set filled on a non-circle sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "radius"){
		auto circle = dynamic_cast<Circle*>(&sprite);
		if(circle){
			circle->setRadius(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set radius on a non-circle sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "attach_state" || property == "sprite_link"){
		// This is a special function to apply children to a highlight or normal state of a sprite button, so ignore it.
		return;
	}
	else if (property == "line_width")
	{
		auto circle = dynamic_cast<Circle*>(&sprite);
		if (circle){
			circle->setLineWidth(ds::string_to_float(value));
		}
		else {
			DS_LOG_WARNING("Trying to set line width on a non-circle sprite of type: " << typeid(sprite).name());
		}
	}
	else if(property == "donut_width"){
		auto donut = dynamic_cast<DonutArc*>(&sprite);
		if(donut){
			donut->setDonutWidth(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set donut_inner_radius on a non-donut sprite of type: " << typeid(sprite).name());
		}
	} 
	else if(property == "donut_percent"){
		auto donut = dynamic_cast<DonutArc*>(&sprite);
		if(donut){
			donut->setPercent(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set donut_percent on a non-donut sprite of type: " << typeid(sprite).name());
		}
	}
	else if(property == "dash_length") {
		auto dashed = dynamic_cast<DashedLine*>(&sprite);
		if(dashed) {
			dashed->setDashLength(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set dash_length on a non-dashed line sprite of type " << typeid(sprite).name());
		}
	} else if(property == "dash_space_inc") {
		auto dashed = dynamic_cast<DashedLine*>(&sprite);
		if(dashed) {
			dashed->setSpaceIncrement(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set dash_space_increment on a non-dashed line sprite of type " << typeid(sprite).name());
		}		
	}
	

	/// Check box properties
	else if(property == "check_box_true_label"){
		auto checkBox = dynamic_cast<ControlCheckBox*>(&sprite);
		if(checkBox){
			checkBox->setTrueLabel(ds::wstr_from_utf8(value));
		}
	}else if(property == "check_box_false_label"){
		auto checkBox = dynamic_cast<ControlCheckBox*>(&sprite);
		if(checkBox){
			checkBox->setFalseLabel(ds::wstr_from_utf8(value));
		}
	} else if(property == "check_box_touch_pad"){
		auto checkBox = dynamic_cast<ControlCheckBox*>(&sprite);
		if(checkBox){
			checkBox->setTouchPadding(ds::string_to_float(value));
		}
	} else if(property == "check_box_box_pad"){
		auto checkBox = dynamic_cast<ControlCheckBox*>(&sprite);
		if(checkBox){
			checkBox->setBoxPadding(ds::string_to_float(value));
		}
	} else if(property == "check_box_label_pad"){
		auto checkBox = dynamic_cast<ControlCheckBox*>(&sprite);
		if(checkBox){
			checkBox->setLabelPadding(ds::string_to_float(value));
		}
	}

	else if(property == "model") {
		auto& ud = sprite.getUserData();
		ud.setString(property, value);
	}
	// fallback to engine-registered properites last
	else if(engine.setRegisteredSpriteProperty(property, sprite, value, referer)){
		return;
	}

	else {		 
		DS_LOG_WARNING("Unknown Sprite property: " << property << " in " << referer);
	}
}
void XmlImporter::dispatchStringEvents(const std::string& value, ds::ui::Sprite* bs, const ci::vec3& pos) {
	DS_LOG_VERBOSE(4, "XmlImporter: dispatchStringEvents value=" << value);

	auto leadingBracket = value.find("{");
	if(leadingBracket == 0){
		auto events = ds::split(value, "},{", true);
		for(auto it : events){
			ds::replace(it, "{", "");
			ds::replace(it, "}", "");
			dispatchSingleEvent(it, bs, pos);
		}
	} else {
		dispatchSingleEvent(value, bs, pos);
	}
	
}

void XmlImporter::dispatchSingleEvent(const std::string& value, ds::ui::Sprite* bs, const ci::vec3& globalPos) {
	DS_LOG_VERBOSE(4, "XmlImporter: dispatchSingleEvent value=" << value);

	auto tokens = ds::split(value, "; ", true);
	if(!tokens.empty()){
		std::string eventName = tokens.front();
		ds::Event* eventy = ds::event::Registry::get().getEventCreator(eventName)();
		if(eventy->mWhat < 1){
			DS_LOG_WARNING("XmlImporter::dispatchSingleEvent() Event not defined: " << eventName);
		}

		for(int i = 1; i < tokens.size(); i++){
			auto colony = tokens[i].find(":");
			if(colony != std::string::npos){
				std::string paramType = tokens[i].substr(0, colony);
				std::string paramValue = tokens[i].substr(colony + 1);

				if(paramType.empty() || paramValue.empty()) continue;

				if(paramType == "data"){
					eventy->mUserStringData = paramValue;
				} else if(paramType == "id"){
					eventy->mUserId = ds::string_to_int(paramValue);
				} else if(paramType == "user_size"){
					eventy->mUserSize = parseVector(paramValue);
				}
			}
		}
		eventy->mSpriteOriginator = bs;
		eventy->mEventOrigin = globalPos;
		bs->getEngine().getNotifier().notify(eventy);
	}
}

XmlImporter::~XmlImporter() {
	BOOST_FOREACH( auto s, mStylesheets ) {
		delete s;
	}
}

bool XmlImporter::preloadXml(const std::string& filename, XmlPreloadData& outData) {
	DS_LOG_VERBOSE(3, "XmlImporter: preloadXml filename=" << filename);

	outData.mFilename = filename;
	try {
		if(!ds::safeFileExistsCheck(filename)) {
			DS_LOG_WARNING("XmlImporter file doesn't exist: " << filename);
			return false;
		}
		outData.mXmlTree = ci::XmlTree(cinder::loadFile(filename));
	} catch(ci::XmlTree::Exception &e) {
		DS_LOG_WARNING("XmlImporter doc " << filename << " not loaded!");
		if(e.what()) {
			DS_LOG_WARNING("XmlImporter load exception: " << e.what());
		}
		return false;
	}
	// Catch rapidxml::parse_errors too
	catch(std::exception &e) {
		DS_LOG_WARNING("XmlImporter doc " << filename << " not loaded!");
		if(e.what()) {
			DS_LOG_WARNING("XmlImporter load exception: " << e.what());
		}
		return false;
	}

	// Load the stylesheets
	auto iter = outData.mXmlTree.find("link");
	while(iter != outData.mXmlTree.end()) {
		auto node = *iter;
		iter++;
		if(node.getAttributeValue<std::string>("rel", "") != "stylesheet") {
			DS_LOG_WARNING("<link> tag specified without rel=\"stylesheet\", ignoring...");
			continue;
		}
		std::string stylesheet_file = node.getAttributeValue<std::string>("href", "");
		if(stylesheet_file == "") {
			DS_LOG_WARNING("<link> tag specified without an href attribute, ignoring...");
			continue;
		}

		// Load stylesheet file relative to the XML file
		stylesheet_file = filePathRelativeTo(filename, stylesheet_file);

		Stylesheet *s = new Stylesheet();
		if(!s->loadFile(stylesheet_file, filename)) {
			DS_LOG_WARNING("Error loading stylesheet: " << stylesheet_file);
			delete s;
		} else
			outData.mStylesheets.push_back(s);
	}

	// If automatically caching, add this to the cache
	if(AUTO_CACHE){
		PRELOADED_CACHE[filename] = outData;
	}

	return true;
}

void XmlImporter::setAutoCache(const bool doCaching){
	AUTO_CACHE = doCaching;
	if(!AUTO_CACHE){
		PRELOADED_CACHE.clear();
	}
}

bool XmlImporter::loadXMLto(ds::ui::Sprite* parent, const std::string& filename, NamedSpriteMap &map, SpriteImporter customImporter, const std::string& prefixName, const bool mergeFirstChild) {
	DS_LOG_VERBOSE(3, "XmlImporter: loadXMLto filename=" << filename << " prefix=" << prefixName);

	XmlImporter xmlImporter(parent, filename, map, customImporter, prefixName);

	XmlPreloadData preloadData;

	// if auto caching, look up the xml in the static cache
	bool cachedAlready = false;
	if(AUTO_CACHE){
		auto xmlIt = PRELOADED_CACHE.find(filename);
		if(xmlIt != PRELOADED_CACHE.end()){
			cachedAlready = true;
			preloadData = xmlIt->second;
		}
	}

	// we don't have this in our cache, so look it up
	if(!cachedAlready){
		preloadData.mFilename = filename;
		if(!preloadXml(filename, preloadData)){
			return false;
		}
	}

	// copy each stylesheet, cause the xml importer will delete it's copies when it destructs
	for(auto it = preloadData.mStylesheets.begin(); it < preloadData.mStylesheets.end(); ++it){
		Stylesheet* ss = new Stylesheet();
		ss->mRules = (*it)->mRules;
		ss->mReferer = (*it)->mReferer;
		xmlImporter.mStylesheets.push_back(ss);
	}

	return xmlImporter.load(preloadData.mXmlTree, mergeFirstChild);
}

bool XmlImporter::loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& preloadData, NamedSpriteMap &map, SpriteImporter customImporter, const std::string& prefixName, const bool mergeFirstChild) {
	DS_LOG_VERBOSE(3, "XmlImporter: loadXMLto preloaded filename=" << preloadData.mFilename << " prefix=" << prefixName);
	XmlImporter xmlImporter(parent, preloadData.mFilename, map, customImporter, prefixName);

	// copy each stylesheet, cause the xml importer will delete it's copies when it destructs
	for(auto it = preloadData.mStylesheets.begin(); it < preloadData.mStylesheets.end(); ++it){
		Stylesheet* ss = new Stylesheet();
		ss->mRules = (*it)->mRules;
		ss->mReferer = (*it)->mReferer;
		xmlImporter.mStylesheets.push_back(ss);
	}

	return xmlImporter.load(preloadData.mXmlTree, mergeFirstChild);
}


bool XmlImporter::load(ci::XmlTree &xml, const bool mergeFirstChild) {
	if (!xml.hasChild("interface")) {
		DS_LOG_WARNING( "No interface found in xml file: " << mXmlFile );
		return false;
	}

	auto interface = xml.getChild( "interface" );
	auto& sprites = interface.getChildren();
	size_t count = sprites.size();
	if ( count < 1 ) {
		DS_LOG_WARNING( "No sprites found in xml file: " << mXmlFile );
		return false;
	}

	bool mergeFirst = mergeFirstChild;

	BOOST_FOREACH( auto &xmlNode, sprites ) {
		readSprite(mTargetSprite, xmlNode, mergeFirst);
		mergeFirst = false;
	}

	for(auto it : mSpriteLinks){
		auto findy = mNamedSpriteMap.find(it.second);
		if(findy != mNamedSpriteMap.end()){
			if(it.first && findy->second){
				EntryField* ef = dynamic_cast<EntryField*>(it.first);
				SoftKeyboard* sfk = dynamic_cast<SoftKeyboard*>(findy->second);
				if(ef && sfk){
					sfk->setKeyPressFunction([ef](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
						if(ef){
							ef->keyPressed(character, keyType);
						}
					});
				}
			}
		}
	}

	return true;
}

struct SelectorMatchChecker : public boost::static_visitor<bool> {
	SelectorMatchChecker( const std::vector< std::string > &classesToCheck, const std::string &idToCheck )
		: mClassesToCheck(classesToCheck)
		, mIdToCheck(idToCheck)
	{}
	bool operator()(const ds::ui::stylesheets::IdSelector &s) const {
		return mIdToCheck == s.selector;
	}
	bool operator()(const ds::ui::stylesheets::ClassSelector &s) const {
		return (std::find( mClassesToCheck.begin(), mClassesToCheck.end(), s.selector ) != mClassesToCheck.end() );
	}

	const std::vector<std::string> &mClassesToCheck;
	const std::string &mIdToCheck;
};

static void applyStylesheet(const Stylesheet &stylesheet, ds::ui::Sprite &sprite, const std::string &name, const std::string &classes) {

	DS_LOG_VERBOSE(3, "XmlImporter: applyStylesheet stylesheet=" << stylesheet.mReferer << " name=" << name << " classes=" << classes);

	BOOST_FOREACH( auto &rule, stylesheet.mRules ) {
		auto classes_vec = ds::split(classes, " ", true );
		bool matches_rule = false;
		BOOST_FOREACH( auto &matcher, rule.matchers ) {

			// Iterate through .class_rules and #name(id)_rules
			// ALL the sub-matchers have to match for this matcher to match
			bool all_submatchers_match = true;
			BOOST_FOREACH( auto &selector, matcher ) {
				if (! boost::apply_visitor( SelectorMatchChecker(classes_vec, name), selector) ) {
					all_submatchers_match = false;
					break;
				}
			}
			matches_rule = all_submatchers_match;
			if (matches_rule) break;
		}

		if (matches_rule) {
			BOOST_FOREACH( auto &prop, rule.properties ) {
				cinder::XmlTree::Attr attr(nullptr, prop.property_name, prop.property_value );
				XmlImporter::setSpriteProperty( sprite, attr, stylesheet.mReferer );
			}
		}
	}
}


std::string XmlImporter::getSpriteTypeForSprite(ds::ui::Sprite* sp){
	if(dynamic_cast<ds::ui::LayoutSprite*>(sp)) return "layout";
	if(dynamic_cast<ds::ui::SpriteButton*>(sp)) return "sprite_button";
	if(dynamic_cast<ds::ui::LayoutButton*>(sp)) return "layout_button";
	if(dynamic_cast<ds::ui::ImageButton*>(sp)) return "image_button";
	if(dynamic_cast<ds::ui::ImageWithThumbnail*>(sp)) return "image_with_thumbnail";
	if(dynamic_cast<ds::ui::Image*>(sp)) return "image";
	if(dynamic_cast<ds::ui::Text*>(sp)) return "text";
	if(dynamic_cast<ds::ui::ScrollBar*>(sp)) return "scroll_bar";
	if(dynamic_cast<ds::ui::ScrollList*>(sp)) return "scroll_list";
	if(dynamic_cast<ds::ui::ScrollArea*>(sp)) return "scroll_area";
	if(dynamic_cast<ds::ui::CenteredScrollArea*>(sp)) return "centered_scroll_area";
	if(dynamic_cast<ds::ui::Circle*>(sp)) return "circle";
	if(dynamic_cast<ds::ui::Border*>(sp)) return "border";
	if(dynamic_cast<ds::ui::CircleBorder*>(sp)) return "circle_border";
	if(dynamic_cast<ds::ui::Gradient*>(sp)) return "gradient";
	if(dynamic_cast<ds::ui::SoftKeyboard*>(sp)) return "soft_keyboard";
	if(dynamic_cast<ds::ui::EntryField*>(sp)) return "entry_field";
	if(dynamic_cast<ds::ui::ControlSlider*>(sp)){
		auto slider = dynamic_cast<ds::ui::ControlSlider*>(sp);
		if(slider->getIsVertical()){
			return "control_slider_vertical";
		} else {
			return "control_slider_horizontal";				
		}
		return "control_slider";
	}
	if(dynamic_cast<ds::ui::ControlCheckBox*>(sp)) return "control_check_box";
	return "sprite";
}

// NOTE! If you add a sprite below, please add it above! Thanks, byeeee!
ds::ui::Sprite* XmlImporter::createSpriteByType(ds::ui::SpriteEngine& engine, const std::string& type, const std::string& value) {
	DS_LOG_VERBOSE(4, "XmlImporter: createSpriteByType type=" << type << " value=" << value);

	ds::ui::Sprite* spriddy = nullptr;

	if(type == "sprite") {
		spriddy = new ds::ui::Sprite(engine);
	} else if(type == "image") {
		auto image = new ds::ui::Image(engine);
		std::string relative_file = value;
		boost::trim(relative_file);
		if(relative_file != "") {
			setSpriteProperty(*image, ci::XmlTree::Attr(nullptr, "filename", relative_file), nullptr);
		}
		spriddy = image;
	} else if(type == "image_with_thumbnail") {
		auto image = new ds::ui::ImageWithThumbnail(engine);
		spriddy = image;
	} else if(type == "text" || type == "multiline_text") {
		auto text = new ds::ui::Text(engine);
		auto content = value;
		boost::trim(content);
		text->setText(content);
		spriddy = text;
	} else if(type == "image_button") {
		auto content = value;
		boost::trim(content);
		float touchPad = 0.0f;
		if(content.size() > 0) touchPad = (float)atof(content.c_str());
		auto imgButton = new ds::ui::ImageButton(engine, "", "", touchPad);
		spriddy = imgButton;
	} else if(type == "sprite_button"){
		spriddy = new ds::ui::SpriteButton(engine);
	} else if(type == "layout_button"){
		spriddy = new ds::ui::LayoutButton(engine);
	} else if(type == "gradient"){
		auto gradient = new ds::ui::GradientSprite(engine);
		spriddy = gradient;
	} else if(type == "layout"){
		auto layoutSprite = new ds::ui::LayoutSprite(engine);
		spriddy = layoutSprite;
	} else if(type == "border"){
		spriddy = new ds::ui::Border(engine);
	} else if(type == "circle"){
		spriddy = new ds::ui::Circle(engine);
	} else if(type == "circle_border"){
		spriddy = new ds::ui::CircleBorder(engine);
	} else if(type == "scroll_list" || type == "scroll_list_vertical"){
		spriddy = new ds::ui::ScrollList(engine);
	} else if(type == "scroll_list_horizontal"){
		spriddy = new ds::ui::ScrollList(engine, false);
	} else if(type == "scroll_area"){
		spriddy = new ds::ui::ScrollArea(engine, 0.0f, 0.0f);
	} else if(type == "centered_scroll_area"){
		spriddy = new ds::ui::CenteredScrollArea(engine, 0.0f, 0.0f);
	} else if(type == "control_check_box"){
		spriddy = new ds::ui::ControlCheckBox(engine);
	} else if(type == "control_slider" || type == "control_slider_horizontal"){
		spriddy = new ds::ui::ControlSlider(engine, false);
	} else if(type == "control_slider_vertical"){
		spriddy = new ds::ui::ControlSlider(engine, true);
	} else if(type == "donut_arc") {
		spriddy = new ds::ui::DonutArc(engine);
	} else if(type == "dashed_line"){
		spriddy = new DashedLine(engine);
	} else if(type == "scroll_bar"){
		spriddy = new ds::ui::ScrollBar(engine);
	} else if(type == "soft_keyboard"){
		SoftKeyboardSettings sks;
		auto tokens = ds::split(value, "; ", true);
		std::string keyboardType = "standard";
		for(auto it : tokens){
			auto colony = it.find(":");
			if(colony != std::string::npos){
				std::string paramType = it.substr(0, colony);
				std::string paramValue = it.substr(colony + 1);
				if(paramType.empty() || paramValue.empty())continue;

				if(paramType == "type"){
					keyboardType = paramValue;
				} else if(paramType == "key_up_text_config"){
					sks.mKeyUpTextConfig = paramValue;
				} else if(paramType == "key_dn_text_config"){
					sks.mKeyDnTextConfig = paramValue;
				} else if(paramType == "key_up_color"){
					sks.mKeyUpColor = parseColor(paramValue, engine);
				} else if(paramType == "key_down_color"){
					sks.mKeyDownColor = parseColor(paramValue, engine);
				} else if(paramType == "key_text_offset"){
					sks.mKeyTextOffset = ci::vec2(parseVector(paramValue));
				} else if(paramType == "key_touch_padding"){
					sks.mKeyTouchPadding = ds::string_to_float(paramValue);
				} else if(paramType == "key_initial_position"){
					sks.mKeyInitialPosition = ci::vec2(parseVector(paramValue));
				} else if(paramType == "key_scale"){
					sks.mKeyScale = ds::string_to_float(paramValue);
				} else if(paramType == "img_letter_up"){
					sks.mKeyLetterUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_letter_dn"){
					sks.mKeyLetterDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_number_up"){
					sks.mKeyNumberUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_number_dn"){
					sks.mKeyNumberDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_space_up"){
					sks.mKeySpaceUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_space_dn"){
					sks.mKeySpaceDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_enter_up"){
					sks.mKeyEnterUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_enter_dn"){
					sks.mKeyEnterDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_delete_up"){
					sks.mKeyDeleteUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_delete_dn"){
					sks.mKeyDeleteDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_shift_up"){
					sks.mKeyShiftUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_shift_dn"){
					sks.mKeyShiftDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_tab_up"){
					sks.mKeyTabUpImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_tab_dn"){
					sks.mKeyTabDnImage = ds::Environment::expand(paramValue);
				} else if(paramType == "img_up_none"){
					if(parseBoolean(paramValue)){
						sks.mKeyLetterUpImage = "";
						//sks.mKeyLetterDnImage = "";
						sks.mKeyNumberUpImage = "";
						//sks.mKeyNumberDnImage = "";
						sks.mKeySpaceUpImage = "";
						//sks.mKeySpaceDnImage = "";
						sks.mKeyEnterUpImage = "";
						//sks.mKeyEnterDnImage = "";
						sks.mKeyDeleteUpImage = "";
						//sks.mKeyDeleteDnImage = "";
						sks.mKeyShiftUpImage = "";
						//sks.mKeyShiftDnImage = "";
						sks.mKeyTabUpImage = "";
						//sks.mKeyTabDnImage = "";
					}
				} else if(paramType == "email_mode"){
					sks.mEmailMode = parseBoolean(paramValue);
				}
			}
		}
		if(keyboardType == "lowercase"){
			spriddy = SoftKeyboardBuilder::buildLowercaseKeyboard(engine, sks);
		} else if(keyboardType == "pinpad"){
			spriddy = SoftKeyboardBuilder::buildPinPadKeyboard(engine, sks);
		} else if(keyboardType == "pincode"){
			spriddy = SoftKeyboardBuilder::buildPinCodeKeyboard(engine, sks);
		} else if(keyboardType == "extended"){
			spriddy = SoftKeyboardBuilder::buildExtendedKeyboard(engine, sks);
		} else {
			spriddy = SoftKeyboardBuilder::buildStandardKeyboard(engine, sks);
		}
	} else if(type == "entry_field"){
		EntryFieldSettings efs;
		auto tokens = ds::split(value, "; ", true);
		for(auto it : tokens){
			auto colony = it.find(":");
			if(colony != std::string::npos){
				std::string paramType = it.substr(0, colony);
				std::string paramValue = it.substr(colony + 1);

				if(paramType.empty() || paramValue.empty())continue;
				if(paramType == "text_config"){
					efs.mTextConfig = paramValue;
				} else if(paramType == "cursor_size"){
					efs.mCursorSize = ci::vec2(parseVector(paramValue));
				} else if(paramType == "field_size"){
					efs.mFieldSize = ci::vec2(parseVector(paramValue));
				} else if(paramType == "cursor_offset"){
					efs.mCursorOffset = ci::vec2(parseVector(paramValue));
				} else if(paramType == "cursor_color"){
					efs.mCursorColor = parseColor(paramValue, engine);
				} else if(paramType == "blink_rate"){
					efs.mBlinkRate = ds::string_to_float(paramValue);
				} else if(paramType == "animate_rate"){
					efs.mAnimationRate = ds::string_to_float(paramValue);
				} else if(paramType == "password_mode"){
					efs.mPasswordMode = parseBoolean(paramValue);
				} else if(paramType == "text_offset"){
					efs.mTextOffset = ci::vec2(parseVector(paramValue));
				}
			}
		}

		auto ef =  new EntryField(engine, efs);
		ef->focus();
		spriddy = ef;
	
	}

	return spriddy;
}

bool XmlImporter::readSprite(ds::ui::Sprite* parent, std::unique_ptr<ci::XmlTree>& node, const bool mergeFirstSprite){
	if(!parent){
		DS_LOG_WARNING("No parent sprite specified when reading a sprite from xml file=" << mXmlFile);
		return false;
	}

	std::string type = node->getTag();
	std::string value = node->getValue();
	auto &engine = parent->getEngine();

	DS_LOG_VERBOSE(6, "XmlImporter: readSprite type=" << type << " value=" << value);

	if(type == "xml"){
		std::string xmlPath = filePathRelativeTo(mXmlFile, node->getAttributeValue<std::string>("src", ""));
		if(xmlPath.empty()){
			DS_LOG_WARNING("XmlImporter: Recursive XML: Specify a src parameter to load xml in " << mXmlFile);
			return false;
		}
		if(xmlPath == mXmlFile){
			DS_LOG_WARNING("XmlImporter: Recursive XML: You cannot load the same xml from the same xml in " << mXmlFile);
			return false;
		}

		// Apply dot naming scheme
		std::string spriteName = node->getAttributeValue<std::string>("name", "");
		if(!mNamePrefix.empty()){
			std::stringstream ss; 
			ss << mNamePrefix << "." << spriteName;
			spriteName = ss.str();
		}

		if(!XmlImporter::loadXMLto(parent, xmlPath, mNamedSpriteMap, mCustomImporter, spriteName, mergeFirstSprite)){
			return false;
		}

		// Use child nodes of the "xml" node to set children properties
		BOOST_FOREACH(auto &newNode, node->getChildren()) {
			if(newNode->getTag() == "property"){
				std::string childSpriteName = newNode->getAttributeValue<std::string>("name", "");
				std::stringstream ss;
				ss << spriteName << "." << childSpriteName;
				childSpriteName = ss.str();
				
				ds::ui::Sprite* spriddy = nullptr;
				
				for(auto it = mNamedSpriteMap.begin(); it != mNamedSpriteMap.end(); ++it){
					if(it->first == childSpriteName){
						spriddy = it->second;
					}
				}

				if(spriddy){
					BOOST_FOREACH(auto &attr, newNode->getAttributes()) {
						if(attr.getName() == "name") continue; // don't overwrite the name
						setSpriteProperty(*spriddy, attr, xmlPath);
					}
				} else {
					DS_LOG_WARNING("XmlImporter: Recursive XML: Couldn't find a child with the name " << childSpriteName << " to apply properties to");
				}
			} else {
				DS_LOG_WARNING("XmlImporter: Recursive XML: Regular children are not supported in recursive xml. Tagname=" << newNode->getTag());
			}
		}


	} else {
		ds::ui::Sprite* spriddy = nullptr;
		if(mergeFirstSprite){
			auto parentType = getSpriteTypeForSprite(parent);
			if(parentType == type){
				spriddy = parent;
			}
		}

		if(!spriddy){
			spriddy = createSpriteByType(engine, type, value);
		}

		if(!spriddy){
			spriddy = engine.createSpriteImporter(type);
		}

		if(!spriddy && mCustomImporter) {
			spriddy = mCustomImporter(type, *node);
		}

		if(!spriddy) {
			DS_LOG_WARNING("Error creating sprite! Type=" << type);
			return false;
		}

		BOOST_FOREACH(auto &sprite, node->getChildren()) {
			readSprite(spriddy, sprite, false);
		}

		std::string linkValue = node->getAttributeValue<std::string>("sprite_link", "");

		if(!linkValue.empty()){
			mSpriteLinks[spriddy] = linkValue;
		}

		ds::ui::ScrollArea* parentScroll = dynamic_cast<ds::ui::ScrollArea*>(parent);
		ds::ui::SpriteButton* spriteButton = dynamic_cast<ds::ui::SpriteButton*>(parent);
		ds::ui::LayoutButton* layoutButton = dynamic_cast<ds::ui::LayoutButton*>(parent);
		if(parentScroll){
			parentScroll->addSpriteToScroll(spriddy);
		} else if(spriteButton || layoutButton){
			std::string attachState = node->getAttributeValue<std::string>("attach_state", "");
			if(attachState.empty()){
				parent->addChildPtr(spriddy);
			} else if(attachState == "normal"){
				if(spriteButton){
					spriteButton->getNormalSprite().addChildPtr(spriddy);
				} else if(layoutButton){
					layoutButton->getNormalSprite().addChildPtr(spriddy);
				}
			} else if(attachState == "high"){
				if(spriteButton){
					spriteButton->getHighSprite().addChildPtr(spriddy);
				} else if(layoutButton){
					layoutButton->getHighSprite().addChildPtr(spriddy);
				}
			}
			if(spriteButton){
				spriteButton->showUp();
			} else if(layoutButton){
				layoutButton->showUp();
			}
		} else if(parent != spriddy){
			parent->addChildPtr(spriddy);
		}

		// Get sprite name and classes
		std::string sprite_name = node->getAttributeValue<std::string>("name", "");
		std::string sprite_classes = node->getAttributeValue<std::string>("class", "");

		// Apply stylesheet(s)
		BOOST_FOREACH(auto stylesheet, mStylesheets) {
			applyStylesheet(*stylesheet, *spriddy, sprite_name, sprite_classes);
		}

		// Set properties from xml attributes, overwriting those from the stylesheet(s)
		BOOST_FOREACH(auto &attr, node->getAttributes()) {
			setSpriteProperty(*spriddy, attr, mXmlFile);
		}

		// Put sprite in named sprites map
		if(sprite_name != "") {

			if(!mNamePrefix.empty()){
				std::stringstream ss;
				ss << mNamePrefix << "." << sprite_name;
				sprite_name = ss.str();
			}

			spriddy->setSpriteName(ds::wstr_from_utf8(sprite_name));

			if(mNamedSpriteMap.find(sprite_name) != mNamedSpriteMap.end()) {
				DS_LOG_WARNING("Interface xml file contains duplicate sprites named:" << sprite_name << ", only the first one will be identified.");
			} else {
				mNamedSpriteMap.insert(std::make_pair(sprite_name, spriddy));
			}
		}
	}


	return true;
}

} // namespace ui
} // namespace ds
