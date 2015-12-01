#include "interface_xml_importer.h"

#include "stylesheet_parser.h"

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/gradient_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/circle.h>
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

// Color format: #AARRGGBB OR #RRGGBB OR AARRGGBB OR RRGGBB. Example: ff0033 or #9933ffbb
static ci::ColorA parseColor( const std::string &color ) {
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

// Example: size="400, 400" the space after the comma is required to read the second and third token
ci::Vec3f parseVector( const std::string &s ) {
	auto tokens = ds::split( s, ", ", true );
	ci::Vec3f v;
	v.x = tokens.size() > 0 ? std::stof(tokens[0]) : 0.0f;
	v.y = tokens.size() > 1 ? std::stof(tokens[1]) : 0.0f;
	v.z = tokens.size() > 2 ? std::stof(tokens[2]) : 0.0f;

	return v;
}

static bool parseBoolean( const std::string &s ) {
	return (s == "true" || s == "TRUE" || s == "yes" || s == "YES" || s == "on" || s == "ON" ) ? true : false;
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
static const ds::BitMask parseMultitouchMode(const std::string& s){
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

static std::string filePathRelativeTo( const std::string &base, const std::string &relative ) {
	using namespace boost::filesystem;
	boost::system::error_code e;
	std::string ret = canonical( path(relative), path(base).parent_path(), e ).string();
	if (e.value() != boost::system::errc::success) {
		DS_LOG_WARNING( "Trying to use bad relative file path: " << relative << ": " << e.message() );
	}
	return ret;
}

static void setSpriteProperty(ds::ui::Sprite &sprite, ci::XmlTree::Attr &attr, const std::string &referer = "") {
	std::string property = attr.getName();

	// This is a pretty long "case switch" (well, effectively a case switch).
	// It seems like it'd be slow, but in practice, it's relatively fast.
	// The slower parts of this are the actual functions that are called (particularly multilinetext setResizeLimit())
	// So be sure that this is actually performing slowly before considering a refactor.

	if(property == "name" || property == "class") {
		// Do nothing, these are handled elsewhere
	} else if(property == "width") {
		sprite.setSize(attr.getValue<float>(), sprite.getHeight());
	} else if(property == "height") {
		sprite.setSize(sprite.getWidth(), attr.getValue<float>());
	} else if(property == "depth") {
		sprite.setSizeAll(sprite.getWidth(), sprite.getHeight(), attr.getValue<float>());
	} else if(property == "size") {
		ci::Vec3f v = parseVector(attr.getValue());
		sprite.setSize(v.x, v.y);
	} else if(property == "color") {
		sprite.setTransparent(false);
		sprite.setColorA(parseColor(attr.getValue()));
	} else if(property == "opacity") {
		sprite.setOpacity(attr.getValue<float>());
	} else if(property == "position") {
		sprite.setPosition(parseVector(attr.getValue()));
	} else if(property == "rotation") {
		sprite.setRotation(parseVector(attr.getValue()));
	} else if(property == "scale") {
		sprite.setScale(parseVector(attr.getValue()));
	} else if(property == "center") {
		sprite.setCenter(parseVector(attr.getValue()));
	} else if(property == "clipping") {
		sprite.setClipping(parseBoolean(attr.getValue()));
	} else if(property == "blend_mode") {
		sprite.setBlendMode(parseBlendMode(attr.getValue()));
	} else if(property == "enable"){
		sprite.enable(parseBoolean(attr.getValue()));
	} else if(property == "multitouch"){
		sprite.enableMultiTouch(parseMultitouchMode(attr.getValue()));
	} else if(property == "transparent"){
		sprite.setTransparent(parseBoolean(attr.getValue()));
	} else if(property == "animate_on"){
		sprite.setAnimateOnScript(attr.getValue());
	} else if(property == "t_pad") {
		sprite.mLayoutTPad = attr.getValue<float>();
	} else if(property == "b_pad") {
		sprite.mLayoutBPad = attr.getValue<float>();
	} else if(property == "l_pad") {
		sprite.mLayoutLPad = attr.getValue<float>();
	} else if(property == "r_pad") {
		sprite.mLayoutRPad = attr.getValue<float>();
	} else if(property == "layout_size_mode"){
		auto sizeMode = attr.getValue();
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
		auto alignMode = attr.getValue();
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
		auto alignMode = attr.getValue();
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
		sprite.mLayoutFudge = parseVector(attr.getValue()).xy();
	} else if(property == "layout_size"){
		sprite.mLayoutSize = parseVector(attr.getValue()).xy();
	}

	// LayoutSprite specific (the other layout stuff could apply to any sprite)
	else if(property == "layout_type"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		if(layoutSprite){
			auto layoutType = attr.getValue();
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
			layoutSprite->setSpacing(attr.getValue<float>());
		} else {
			DS_LOG_WARNING("Couldn't set layout_type, as this sprite is not a LayoutSprite.");
		}
	}
	else if(property == "overall_alignment"){
		auto layoutSprite = dynamic_cast<LayoutSprite*>(&sprite);
		auto alignMode = attr.getValue();
		if(alignMode == "left" || alignMode == "top"){
			layoutSprite->setOverallAlignment(LayoutSprite::kLeft);
		} else if(alignMode == "center" || alignMode == "middle"){
			layoutSprite->setOverallAlignment(LayoutSprite::kCenter);
		} else if(alignMode == "right" || alignMode == "bottom"){
			layoutSprite->setOverallAlignment(LayoutSprite::kRight);
		} else {
			DS_LOG_WARNING("overall_alignment set to an invalid value of " << alignMode);
		}
	}

	// Text, MultilineText specific attributes
	else if(property == "font") {
		// Try to set the font
		auto text = dynamic_cast<Text *>(&sprite);
		if(text) {
			auto cfg = text->getEngine().getEngineCfg().getText(attr.getValue());
			cfg.configure(*text);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "resize_limit") {
		// Try to set the resize limit
		auto text = dynamic_cast<Text *>(&sprite);
		if(text) {
			auto v = parseVector(attr.getValue());
			text->setResizeLimit(v.x, v.y);
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "text_align"){
		auto text = dynamic_cast<MultilineText*>(&sprite);
		if(text){
			std::string alignString = attr.getValue();
			if(alignString == "right"){
				text->setAlignment(ds::ui::Alignment::kRight);
			} else if(alignString == "center"){
				text->setAlignment(ds::ui::Alignment::kCenter);
			} else {
				text->setAlignment(ds::ui::Alignment::kCenter);
			}
		}
	}

	// Image properties
	else if(property == "filename" || property == "src") {
		auto image = dynamic_cast<Image *>(&sprite);
		if(image) {
			image->setImageFile(filePathRelativeTo(referer, attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}


	// Image Button properties
	else if(property == "down_image") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setHighImage(filePathRelativeTo(referer, attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "up_image") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setNormalImage(filePathRelativeTo(referer, attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "btn_touch_padding") {
		auto image = dynamic_cast<ImageButton *>(&sprite);
		if(image) {
			image->setTouchPad(attr.getValue<float>());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}

	// Gradient sprite properties
	else if(property == "colorTop"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsV(parseColor(attr.getValue()), gradient->getColorBL());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorBot"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsV(gradient->getColorTL(), parseColor(attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorLeft"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsH(parseColor(attr.getValue()), gradient->getColorTR());
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "colorRight"){
		auto gradient = dynamic_cast<GradientSprite*>(&sprite);
		if(gradient){
			gradient->setColorsH(gradient->getColorTL(), parseColor(attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set incompatible attribute _" << property << "_ on sprite of type: " << typeid(sprite).name());
		}
	}

	// Circle sprite properties
	else if(property == "filled"){
		auto circle = dynamic_cast<Circle*>(&sprite);
		if(circle){
			circle->setFilled(parseBoolean(attr.getValue()));
		} else {
			DS_LOG_WARNING("Trying to set filled on a non-cicle sprite of type: " << typeid(sprite).name());
		}
	} else if(property == "radius"){
		auto circle = dynamic_cast<Circle*>(&sprite);
		if(circle){
			circle->setRadius(attr.getValue<float>());
		} else {
			DS_LOG_WARNING("Trying to set radius on a non-cicle sprite of type: " << typeid(sprite).name());
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

bool XmlImporter::loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& preloadData, NamedSpriteMap &map, SpriteImporter customImporter){
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

static void applyStylesheet( const Stylesheet &stylesheet, ds::ui::Sprite &sprite, const std::string &name, const std::string &classes ) {
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
				setSpriteProperty( sprite, attr, stylesheet.mReferer );
			}
		}
	}
}
bool XmlImporter::readSprite(ds::ui::Sprite* parent, std::unique_ptr<ci::XmlTree>& node) {
	std::string type = node->getTag();

	ds::ui::Sprite* spriddy = nullptr;

	auto &engine = parent->getEngine();
	if (type == "sprite") {
		spriddy = new ds::ui::Sprite(engine);
	}
	else if (type == "image") {
		auto image = new ds::ui::Image(engine);
		std::string relative_file = node->getValue();
		boost::trim(relative_file);
		if (relative_file != "") {
			setSpriteProperty(*image, ci::XmlTree::Attr(nullptr, "filename", relative_file));
		}
		spriddy = image;
	}
	else if (type == "text") {
		auto text = new ds::ui::Text(engine);
		auto content = node->getValue();
		boost::trim(content);
		text->setText(content);
		spriddy = text;
	}
	else if (type == "multiline_text") {
		auto text = new ds::ui::MultilineText(engine);
		auto content = node->getValue();
		boost::trim(content);
		text->setText(content);
		spriddy = text;
	}
	else if(type == "image_button") {
		auto content = node->getValue();
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
	else if(type == "circle"){
		spriddy = new ds::ui::Circle(engine);
	}
	else if (mCustomImporter) {
		spriddy = mCustomImporter(type, *node);
	}

	if (!spriddy) {
		DS_LOG_WARNING("Error creating sprite! Type=" << type);
		return false;
	}

	BOOST_FOREACH(auto &sprite, node->getChildren()) {
		readSprite(spriddy, sprite);
	}

	parent->addChild(*spriddy);

	// Get sprite name and classes
	std::string sprite_name = node->getAttributeValue<std::string>("name", "");
	std::string sprite_classes = node->getAttributeValue<std::string>("class", "");

	// Put sprite in named sprites map
	if (sprite_name != "") {
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
