#include "interface_xml_importer.h"

#include "stylesheet_parser.h"

#include <ds/app/engine/engine.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/gradient_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/image_with_thumbnail.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_list.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/sprite/border.h>
#include <ds/ui/sprite/circle.h>
#include <ds/ui/sprite/circle_border.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/util/string_util.h>
#include <ds/debug/logger.h>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <Poco/Path.h>

#include <typeinfo>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

namespace {
static std::unordered_map<std::string, ds::ui::XmlImporter::XmlPreloadData>	 PRELOADED_CACHE;
static bool AUTO_CACHE = false;
}


namespace ds {
namespace ui {

static const std::string INVALID_VALUE = "UNACCEPTABLE!!!!";

//HACK!
static std::string sCurrentFile;



std::string XmlImporter::RGBToHex(ci::Color theColor){
	return RGBToHex((int)(theColor.r * 255.0f), (int)(theColor.g * 255.0f), (int)(theColor.b * 255.0f));
}

std::string XmlImporter::RGBToHex(int rNum, int gNum, int bNum){
	std::string result;
	char r[255];
	sprintf_s(r, "%.2X", rNum);
	result.append(r);
	char g[255];
	sprintf_s(g, "%.2X", gNum);
	result.append(g);
	char b[255];
	sprintf_s(b, "%.2X", bNum);
	result.append(b);
	return result;
}

// Grabs a color from the engine's supplied color list
static ci::ColorA retrieveColorFromEngine(const std::string &color, const ds::ui::SpriteEngine& engine){
	return engine.getColors().getColorFromName(color);
}

// Color format: #AARRGGBB OR #RRGGBB OR AARRGGBB OR RRGGBB. Example: ff0033 or #9933ffbb
static ci::ColorA parseHexColor( const std::string &color ) {

	std::string s = color;

	if (boost::starts_with( s, "#" ))
		boost::erase_head( s, 1 );

	std::stringstream converter(s);
	unsigned int value;
	converter >> std::hex >> value;

	float a = (s.length() > 6)
		?  ((value >> 24) & 0xFF) / 255.0f
		: 1.0f;
	float r = ((value >> 16) & 0xFF) / 255.0f;
	float g = ((value >> 8) & 0xFF) / 255.0f;
	float b = ((value) & 0xFF) / 255.0f;

	return ci::ColorA(r, g, b, a);
}

static ci::ColorA parseColor(const std::string &color, const ds::ui::SpriteEngine& engine){
	std::string s = color;

	//If we have colors in our engine, and this isn't a hex value
	if( !engine.getColors().empty() && !boost::starts_with(s, "#")){
		return retrieveColorFromEngine(color, engine);
	}

	return parseHexColor(color);

}

static std::string unparseColor(const ci::Color& color){
	// TODO: look up engine colors
	return XmlImporter::RGBToHex(color);
}

// Example: size="400, 400" the space after the comma is required to read the second and third token
ci::Vec3f parseVector( const std::string &s ) {
	auto tokens = ds::split( s, ", ", true );
	ci::Vec3f v;
	v.x = tokens.size() > 0 ? ds::string_to_float(tokens[0]) : 0.0f;
	v.y = tokens.size() > 1 ? ds::string_to_float(tokens[1]) : 0.0f;
	v.z = tokens.size() > 2 ? ds::string_to_float(tokens[2]) : 0.0f;

	return v;
}

static std::string unparseVector(const ci::Vec3f& v){
	std::stringstream ss;
	ss << v.x << ", " << v.y << ", " << v.z;
	return ss.str();
}

static std::string unparseVector(const ci::Vec2f& v){
	std::stringstream ss;
	ss << v.x << ", " << v.y;
	return ss.str();
}

static bool parseBoolean( const std::string &s ) {
	return (s == "true" || s == "TRUE" || s == "yes" || s == "YES" || s == "on" || s == "ON" ) ? true : false;
}

static std::string unparseBoolean(const bool b){
	if(b) return "true";
	return "false";
}

static ds::ui::BlendMode parseBlendMode( const std::string &s ) {
	using namespace ds::ui;
	if (boost::iequals( s, "normal" ))			return NORMAL;
	else if (boost::iequals( s, "multiply" ))	return MULTIPLY;
	else if (boost::iequals( s, "screen" ))		return SCREEN;
	else if (boost::iequals( s, "add" ))		return ADD;
	else if (boost::iequals( s, "subtract" ))	return SUBTRACT;
	else if (boost::iequals( s, "lighten" ))	return LIGHTEN;
	else if (boost::iequals( s, "darken" ))		return DARKEN;
	return NORMAL;
}

// TODO: add the rest of the permutations, if you want em
const ds::BitMask XmlImporter::parseMultitouchMode(const std::string& s){
	using namespace ds::ui;
	if(boost::iequals(s, "info"))				return MULTITOUCH_INFO_ONLY;
	else if(boost::iequals(s, "pos"))			return MULTITOUCH_CAN_POSITION;
	else if(boost::iequals(s, "all"))			return MULTITOUCH_NO_CONSTRAINTS;
	else if(boost::iequals(s, "scale"))			return MULTITOUCH_CAN_SCALE;
	else if(boost::iequals(s, "pos_x"))			return MULTITOUCH_CAN_POSITION_X;
	else if(boost::iequals(s, "pos_y"))			return MULTITOUCH_CAN_POSITION_Y;
	else if(boost::iequals(s, "pos_scale"))		return MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_SCALE;
	else if(boost::iequals(s, "pos_rotate"))	return MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_ROTATE;
	else if(boost::iequals(s, "rotate"))		return MULTITOUCH_CAN_ROTATE;
	return MULTITOUCH_INFO_ONLY;
}

const std::string XmlImporter::getMultitouchStringForBitMask(const ds::BitMask& s){
	using namespace ds::ui;
	if(s & MULTITOUCH_INFO_ONLY){
		return "info";
	} else if(s & MULTITOUCH_NO_CONSTRAINTS){
		return "all";
	} else if(s & MULTITOUCH_CAN_POSITION){
		if(s & MULTITOUCH_CAN_ROTATE){
			return "pos_rotate";
		}
		if(s & MULTITOUCH_CAN_SCALE){
			return "pos_scale";
		}

		return "pos";
	} else if(s & MULTITOUCH_CAN_ROTATE){
		return "rotate";
	} else if(s & MULTITOUCH_CAN_POSITION_X){
		return "pos_x";
	} else if(s & MULTITOUCH_CAN_POSITION_Y){
		return "pos_y";
	} else if(s & MULTITOUCH_CAN_SCALE){
		return "scale";
	}

	return "info";
}

std::string XmlImporter::getLayoutSizeModeString(const int sizeMode){
	std::string sizeString = "fixed";
	if(sizeMode == ds::ui::LayoutSprite::kFlexSize)	sizeString = "flex";
	else if(sizeMode == ds::ui::LayoutSprite::kStretchSize) sizeString = "stretch";
	else if(sizeMode == ds::ui::LayoutSprite::kFillSize) sizeString = "fill";
	return sizeString;
}

std::string XmlImporter::getLayoutVAlignString(const int vAlign){
	std::string sizeString = "top";
	if(vAlign == ds::ui::LayoutSprite::kMiddle)	sizeString = "middle";
	else if(vAlign == ds::ui::LayoutSprite::kBottom)	sizeString = "bottom";
	return sizeString;
}

std::string XmlImporter::getLayoutHAlignString(const int vAlign){
	std::string sizeString = "left";
	if(vAlign == ds::ui::LayoutSprite::kCenter)	sizeString = "center";
	else if(vAlign == ds::ui::LayoutSprite::kRight)	sizeString = "right";
	return sizeString;
}

std::string XmlImporter::getLayoutTypeString(const ds::ui::LayoutSprite::LayoutType& propertyValue){
	std::string sizeString = "none";
	if(propertyValue == ds::ui::LayoutSprite::kLayoutVFlow)	sizeString = "vert";
	else if(propertyValue == ds::ui::LayoutSprite::kLayoutHFlow) sizeString = "horiz";
	else if(propertyValue == ds::ui::LayoutSprite::kLayoutSize)	sizeString = "size";
	return sizeString;
}

std::string XmlImporter::getShrinkToChildrenString(const ds::ui::LayoutSprite::ShrinkType& propertyValue){
	std::string sizeString = "none";
	if(propertyValue == ds::ui::LayoutSprite::kShrinkWidth)	sizeString = "width";
	else if(propertyValue == ds::ui::LayoutSprite::kShrinkHeight) sizeString = "height";
	else if(propertyValue == ds::ui::LayoutSprite::kShrinkBoth)	sizeString = "both";
	return sizeString;
}

std::string XmlImporter::getGradientColorsAsString(ds::ui::Gradient* grad){
	if(!grad) return "";
	std::stringstream ss;
	ss << unparseColor(grad->getColorTL()) << ", " << unparseColor(grad->getColorTR()) << ", " << unparseColor(grad->getColorBR()) << ", " << unparseColor(grad->getColorBL());
	return ss.str();
}

namespace {
static const ci::Vec3f				DEFAULT_SIZE = ci::Vec3f(0.0f, 0.0f, 1.0f);
static const ci::Vec3f				DEFAULT_CENTER = ci::Vec3f(0.0f, 0.0f, 0.0f);
static const ci::Vec3f				DEFAULT_POS = ci::Vec3f(0.0f, 0.0f, 0.0f);
static const ci::Vec3f				DEFAULT_ROT = ci::Vec3f(0.0f, 0.0f, 0.0f);
static const ci::Vec3f				DEFAULT_SCALE = ci::Vec3f(1.0f, 1.0f, 1.0f);
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
static const ci::Vec2f				DEFAULT_LAYOUT_SIZEFUDGE = ci::Vec2f::zero();
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
	if(sp.mLayoutUserType != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_size_mode", getLayoutSizeModeString(sp.mLayoutUserType));
	if(sp.mLayoutVAlign != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_v_align", getLayoutVAlignString(sp.mLayoutVAlign));
	if(sp.mLayoutHAlign != DEFAULT_LAYOUT_ALIGN_USERTYPE) xml.setAttribute("layout_h_align", getLayoutHAlignString(sp.mLayoutHAlign));

	ds::ui::LayoutSprite* ls = dynamic_cast<ds::ui::LayoutSprite*>(&sp);
	if(ls){
		if(ls->getLayoutType() != DEFAULT_LAYOUT_TYPE) xml.setAttribute("layout_type", getLayoutTypeString(ls->getLayoutType()));
		if(ls->getSpacing() != DEFAULT_LAYOUT_SPACING) xml.setAttribute("layout_spacing", ls->getSpacing());
		if(ls->getShrinkToChildren() != DEFAULT_SHRINK_TYPE) xml.setAttribute("shrink_to_children", getShrinkToChildrenString(ls->getShrinkToChildren()));
		if(ls->getOverallAlignment() != DEFAULT_SHRINK_TYPE) xml.setAttribute("overall_alignment", getLayoutVAlignString(ls->getOverallAlignment()));
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
	}
	ds::ui::MultilineText* mtxt = dynamic_cast<ds::ui::MultilineText*>(&sp);
	if(mtxt){
		if(mtxt->getConfigName().empty()){
			xml.setAttribute("font_leading", txt->getLeading());
		}

		xml.setAttribute("resize_limit", unparseVector(ci::Vec2f(mtxt->getResizeLimitWidth(), mtxt->getResizeLimitHeight())));
		if(mtxt->getAlignment() != ds::ui::Alignment::kLeft) xml.setAttribute("text_align", getLayoutHAlignString(mtxt->getAlignment()));
	}

	ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(&sp);
	if(img){
		// TODO: make relative paths to xml
		if(!img->getImageFilename().empty()) xml.setAttribute("filename", img->getImageFilename());
		if(img->getCircleCrop()) xml.setAttribute("circle_crop", "true");
	}

	// TODO: parse Image button stuff

	ds::ui::Gradient* grad = dynamic_cast<ds::ui::Gradient*>(&sp);
	if(grad){
		xml.setAttribute("gradientColors", getGradientColorsAsString(grad));
	}

	ds::ui::Circle* circ = dynamic_cast<ds::ui::Circle*>(&sp);
	if(circ){
		xml.setAttribute("filled", unparseBoolean(circ->getFilled()));
		xml.setAttribute("radius", circ->getRadius());
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

static std::string filePathRelativeTo(const std::string &base, const std::string &relative) {
	using namespace boost::filesystem;
	boost::system::error_code e;
	std::string ret = canonical( path(relative), path(base).parent_path(), e ).string();
	if (e.value() != boost::system::errc::success) {
		DS_LOG_WARNING( "Trying to use bad relative file path: " << relative << ": " << e.message() );
	}
	return ret;
}

// a little convenience
static float getFloatFromString(const std::string& value){
	float floatValue = 0.0f;
	ds::string_to_value(value, floatValue);
	return floatValue;
}

void XmlImporter::setSpriteProperty(ds::ui::Sprite &sprite, ci::XmlTree::Attr &attr, const std::string &referer) {
	std::string property = attr.getName();
	setSpriteProperty(sprite, property, attr.getValue(), referer);
}

void XmlImporter::setSpriteProperty(ds::ui::Sprite &sprite, const std::string& property, const std::string& value, const std::string &referer) {
	//Cache the engine for all our color calls
	const ds::ui::SpriteEngine& engine = sprite.getEngine();

	// This is a pretty long "case switch" (well, effectively a case switch).
	// It seems like it'd be slow, but in practice, it's relatively fast.
	// The slower parts of this are the actual functions that are called (particularly multilinetext setResizeLimit())
	// So be sure that this is actually performing slowly before considering a refactor.

	if(property == "name" || property == "class") {
		// Do nothing, these are handled elsewhere
	} else if(property == "width") {
		sprite.setSize(ds::string_to_float(value), sprite.getHeight());
	} else if(property == "height") {
		sprite.setSize(sprite.getWidth(), ds::string_to_float(value));
	} else if(property == "depth") {
		sprite.setSizeAll(sprite.getWidth(), sprite.getHeight(), ds::string_to_float(value));
	} else if(property == "size") {
		ci::Vec3f v = parseVector(value);
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
		sprite.setBlendMode(parseBlendMode(value));
	} else if(property == "enable"){
		sprite.enable(parseBoolean(value));
	} else if(property == "multitouch"){
		sprite.enableMultiTouch(parseMultitouchMode(value));
	} else if(property == "transparent"){
		sprite.setTransparent(parseBoolean(value));
	} else if(property == "animate_on"){
		sprite.setAnimateOnScript(value);
	} else if(property == "t_pad") {
		sprite.mLayoutTPad = ds::string_to_float(value);
	} else if(property == "b_pad") {
		sprite.mLayoutBPad = ds::string_to_float(value);
	} else if(property == "l_pad") {
		sprite.mLayoutLPad = ds::string_to_float(value);
	} else if(property == "r_pad") {
		sprite.mLayoutRPad = ds::string_to_float(value);
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
		sprite.mLayoutFudge = parseVector(value).xy();
	} else if(property == "layout_size"){
		sprite.mLayoutSize = parseVector(value).xy();
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
	
	// Text, MultilineText specific attributes
	else if(property == "font") {
		// Try to set the font
		auto text = dynamic_cast<Text *>(&sprite);
		if(text) {
			auto cfg = text->getEngine().getEngineCfg().getText(value);
			cfg.configure(*text);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_name"){
		auto text = dynamic_cast<Text *>(&sprite);
		if(text) {
			text->setFont(value, text->getFontSize());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "resize_limit") {
		// Try to set the resize limit
		auto text = dynamic_cast<Text *>(&sprite);
		if(text) {
			auto v = parseVector(value);
			text->setResizeLimit(v.x, v.y);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "text_align"){
		auto text = dynamic_cast<MultilineText*>(&sprite);
		if(text){
			std::string alignString = value;
			if(alignString == "right"){
				text->setAlignment(ds::ui::Alignment::kRight);
			} else if(alignString == "center"){
				text->setAlignment(ds::ui::Alignment::kCenter);
			} else {
				text->setAlignment(ds::ui::Alignment::kLeft);
			}
		}
	} else if (property == "text") {
		// Try to set content
		auto text = dynamic_cast<Text *>(&sprite);
		if (text) {
			text->setText(value);
		}
		else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if (property == "font_size") {
		// Try to set the font size
		auto text = dynamic_cast<Text *>(&sprite);
		if (text) {
			text->setFontSize(ds::string_to_float(value));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "font_leading"){
		auto text = dynamic_cast<MultilineText*>(&sprite);
		if(text){
			text->setLeading(ds::string_to_float(value));
		}
	}

	// Image properties
	else if(property == "filename" || property == "src") {
		auto image = dynamic_cast<Image *>(&sprite);
		if(image) {
			image->setImageFile(filePathRelativeTo(referer, value));
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

	else if(property == "scroll_list_layout"){
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		if(scrollList){
			auto vec = parseVector(value);
			scrollList->setLayoutParams(vec.x, vec.y, vec.z, true);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}

	else if(property == "scroll_list_animate"){
		auto scrollList = dynamic_cast<ds::ui::ScrollList*>(&sprite);
		if(scrollList){
			auto vec = parseVector(value);
			scrollList->setAnimateOnParams(vec.x, vec.y);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}

	else if(property == "scroll_fade_colors"){
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
	}

	else {
		DS_LOG_WARNING("Unknown Sprite property: " << property << " in " << referer);
	}
}

XmlImporter::~XmlImporter() {
	BOOST_FOREACH( auto s, mStylesheets ) {
		delete s;
	}
}

bool XmlImporter::preloadXml(const std::string& filename, XmlPreloadData& outData){
	outData.mFilename = filename;
	try {
		outData.mXmlTree = ci::XmlTree(cinder::loadFile(filename));
	} catch(ci::XmlTree::Exception &e) {
		DS_LOG_WARNING("XML doc not loaded! oh no: " << e.what());
		return false;
	}
	// Catch rapidxml::parse_errors too
	catch(std::exception &e) {
		DS_LOG_WARNING("XML doc not loaded! oh no: " << e.what());
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
}

bool XmlImporter::loadXMLto(ds::ui::Sprite* parent, const std::string& filename, NamedSpriteMap &map, SpriteImporter customImporter) {

	XmlImporter xmlImporter( parent, filename, map, customImporter );

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

	return xmlImporter.load(preloadData.mXmlTree);
}

bool XmlImporter::loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& preloadData, NamedSpriteMap &map, SpriteImporter customImporter ){
	XmlImporter xmlImporter(parent, preloadData.mFilename, map, customImporter);

	// copy each stylesheet, cause the xml importer will delete it's copies when it destructs
	for(auto it = preloadData.mStylesheets.begin(); it < preloadData.mStylesheets.end(); ++it){
		Stylesheet* ss = new Stylesheet();
		ss->mRules = (*it)->mRules;
		ss->mReferer = (*it)->mReferer;
		xmlImporter.mStylesheets.push_back(ss);
	}

	return xmlImporter.load(preloadData.mXmlTree);
}


bool XmlImporter::load( ci::XmlTree &xml ) {
	if (!xml.hasChild("interface")) {
		DS_LOG_WARNING( "No interface found in xml file: " << mXmlFile );
		return false;
	}

	auto interface = xml.getChild( "interface" );
	auto& sprites = interface.getChildren();
	int count = sprites.size();
	if ( count < 1 ) {
		DS_LOG_WARNING( "No sprites found in xml file: " << mXmlFile );
		return false;
	}

	BOOST_FOREACH( auto &xmlNode, sprites ) {
		readSprite(mTargetSprite, xmlNode );
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

static void applyStylesheet( const Stylesheet &stylesheet, ds::ui::Sprite &sprite, const std::string &name, const std::string &classes) {
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
	if(dynamic_cast<ds::ui::ImageButton*>(sp)) return "image_button";
	if(dynamic_cast<ds::ui::ImageWithThumbnail*>(sp)) return "image_with_thumbnail";
	if(dynamic_cast<ds::ui::Image*>(sp)) return "image";
	if(dynamic_cast<ds::ui::MultilineText*>(sp)) return "multiline_text";
	if(dynamic_cast<ds::ui::Text*>(sp)) return "text";
	if(dynamic_cast<ds::ui::ScrollBar*>(sp)) return "scroll_bar";
	if(dynamic_cast<ds::ui::ScrollList*>(sp)) return "scroll_list";
	if(dynamic_cast<ds::ui::ScrollArea*>(sp)) return "scroll_area";
	if(dynamic_cast<ds::ui::Circle*>(sp)) return "circle";
	if(dynamic_cast<ds::ui::Border*>(sp)) return "border";
	if(dynamic_cast<ds::ui::CircleBorder*>(sp)) return "circle_border";
	if(dynamic_cast<ds::ui::Gradient*>(sp)) return "gradient";
	return "sprite";
}

// NOTE! If you add a sprite below, please add it above! Thanks, byeeee!
ds::ui::Sprite* XmlImporter::createSpriteByType(ds::ui::SpriteEngine& engine, const std::string& type, const std::string& value){
	ds::ui::Sprite* spriddy = nullptr;

	if (type == "sprite") {
		spriddy = new ds::ui::Sprite(engine);
	}
	else if (type == "image") {
		auto image = new ds::ui::Image(engine);
		std::string relative_file = value;
		boost::trim(relative_file);
		if (relative_file != "") {
			setSpriteProperty(*image, ci::XmlTree::Attr(nullptr, "filename", relative_file), nullptr);
		}
		spriddy = image;
	}
	else if (type == "image_with_thumbnail") {
		auto image = new ds::ui::ImageWithThumbnail(engine);
		spriddy = image;
	}
	else if (type == "text") {
		auto text = new ds::ui::Text(engine);
		auto content = value;
		boost::trim(content);
		text->setText(content);
		spriddy = text;
	}
	else if (type == "multiline_text") {
		auto text = new ds::ui::MultilineText(engine);
		auto content = value;
		boost::trim(content);
		text->setText(content);
		spriddy = text;
	}
	else if(type == "image_button") {
		auto content = value;
		boost::trim(content);
		float touchPad = 0.0f;
		if(content.size() > 0) touchPad = (float)atof(content.c_str());
		auto imgButton = new ds::ui::ImageButton(engine, "", "", touchPad);
		spriddy = imgButton;
	}
	else if(type == "sprite_button"){
		spriddy = new ds::ui::SpriteButton(engine);
	}
	else if(type == "gradient"){
		auto gradient = new ds::ui::GradientSprite(engine);
		spriddy = gradient;
	}
	else if(type == "layout"){
		auto layoutSprite = new ds::ui::LayoutSprite(engine);
		spriddy = layoutSprite;
	}
	else if(type == "border"){
		spriddy = new ds::ui::Border(engine);
	}
	else if(type == "circle"){
		spriddy = new ds::ui::Circle(engine);
	}
	else if(type == "circle_border"){
		spriddy = new ds::ui::CircleBorder(engine);
	}
	else if(type == "scroll_list"){
		spriddy = new ds::ui::ScrollList(engine);
	} 
	else if(type == "scroll_area"){
		spriddy = new ds::ui::ScrollArea(engine, 0.0f, 0.0f);
	}
	else if(type == "scroll_bar"){
		spriddy = new ds::ui::ScrollBar(engine);
	}

	return spriddy;
}

bool XmlImporter::readSprite(ds::ui::Sprite* parent, std::unique_ptr<ci::XmlTree>& node) {
	std::string type = node->getTag();
	std::string value = node->getValue();
	auto &engine = parent->getEngine();

	ds::ui::Sprite* spriddy = createSpriteByType(engine, type, value);
	
	if (!spriddy && mCustomImporter) {
		spriddy = mCustomImporter(type, *node);
	}

	if (!spriddy) {
		DS_LOG_WARNING("Error creating sprite! Type=" << type);
		return false;
	}

	BOOST_FOREACH(auto &sprite, node->getChildren()) {
		readSprite(spriddy, sprite);
	}

	ds::ui::ScrollArea* parentScroll = dynamic_cast<ds::ui::ScrollArea*>(parent);
	if(parentScroll){
		parentScroll->addSpriteToScroll(spriddy);
	} else {
		parent->addChildPtr(spriddy);
	}

	// Get sprite name and classes
	std::string sprite_name = node->getAttributeValue<std::string>("name", "");
	std::string sprite_classes = node->getAttributeValue<std::string>("class", "");

	// Put sprite in named sprites map
	if (sprite_name != "") {
		spriddy->setSpriteName(ds::wstr_from_utf8(sprite_name));

		if (mNamedSpriteMap.find(sprite_name) != mNamedSpriteMap.end()) {
			DS_LOG_WARNING("Interface xml file contains duplicate sprites named:" << sprite_name << ", only the first one will be identified.");
		}
		else {
			mNamedSpriteMap.insert(std::make_pair(sprite_name, spriddy));
		}
	}

	// Apply stylesheet(s)
	BOOST_FOREACH(auto stylesheet, mStylesheets) {
		applyStylesheet(*stylesheet, *spriddy, sprite_name, sprite_classes);
	}

	// Set properties from xml attributes, overwriting those from the stylesheet(s)
	BOOST_FOREACH(auto &attr, node->getAttributes()) {
		setSpriteProperty(*spriddy, attr, mXmlFile);
	}

	return true;
}

} // namespace ui
} // namespace ds
