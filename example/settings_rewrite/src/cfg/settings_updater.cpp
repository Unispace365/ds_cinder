#include "stdafx.h"

#include "settings_updater.h"

#include <boost/algorithm/string.hpp>
#include <cinder/Xml.h>
#include <Poco/File.h>
#include <Poco/String.h>
#include "ds/app/engine/engine.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
#include "ds/util/string_util.h"
#include "ds/util/color_util.h"
#include "ds/util/file_meta_data.h"

#include "settings_manager.h"


namespace ds {
namespace cfg {


SettingsUpdater::SettingsUpdater(ds::Engine& engine)
	: mEngine(engine)
{
}


void SettingsUpdater::updateEngineSettings(const std::string& fileToUpdate){

	if(fileToUpdate.empty()) return;

	Settings outputSettings;
	outputSettings.readFrom(ds::Environment::expand("%APP%/settings/engine.xml"), false);

	ci::XmlTree xml;
	try{
		xml = ci::XmlTree(ci::loadFile(fileToUpdate));
	} catch(std::exception& e){
		DS_LOG_WARNING("Exception loading settings from " << fileToUpdate << " " << e.what());
		return;
	}
	ci::XmlTree rootNode = xml.getChild("settings");
	ci::XmlTree::Container& theChillins = rootNode.getChildren();

	// SERVER
	updateAnEngineSetting("project_path", outputSettings, theChillins);
	updateAnEngineSetting("server:connect", outputSettings, theChillins);
	updateAnEngineSetting("server:ip", outputSettings, theChillins);
	updateAnEngineSetting("server:send_port", outputSettings, theChillins);
	updateAnEngineSetting("server:listen_port", outputSettings, theChillins);
	updateAnEngineSetting("platform:architecture", outputSettings, theChillins);
	updateAnEngineSetting("platform:guid", outputSettings, theChillins);
	updateAnEngineSetting("xml_importer:cache", outputSettings, theChillins);

	// WINDOW SETTINGS
	updateAnEngineSetting("span_all_displays", outputSettings, theChillins);
	updateAnEngineSetting("world_dimensions", outputSettings, theChillins);
	updateAnEngineSetting("src_rect", outputSettings, theChillins);
	updateAnEngineSetting("dst_rect", outputSettings, theChillins);
	updateAnEngineSetting("screen:title", outputSettings, theChillins);
	updateAnEngineSetting("screen:mode", outputSettings, theChillins);
	updateAnEngineSetting("screen:always_on_top", outputSettings, theChillins);
	updateAnEngineSetting("console:show", outputSettings, theChillins);
	updateAnEngineSetting("idle_time", outputSettings, theChillins);
	updateAnEngineSetting("system:never_sleep", outputSettings, theChillins);

	// RENDER SETTINGS
	updateAnEngineSetting("frame_rate", outputSettings, theChillins);
	updateAnEngineSetting("vertical_sync", outputSettings, theChillins);
	updateAnEngineSetting("hide_mouse", outputSettings, theChillins);
	updateAnEngineSetting("camera:arrow_keys", outputSettings, theChillins);
	updateAnEngineSetting("platform:mute", outputSettings, theChillins);
	updateAnEngineSetting("animation:duration", outputSettings, theChillins);

	// TOUCH SETTINGS
	updateAnEngineSetting("touch_mode", outputSettings, theChillins, "touch:mode");
	updateAnEngineSetting("tuio_port", outputSettings, theChillins, "touch:tuio:port");
	updateAnEngineSetting("tuio:receive_objects", outputSettings, theChillins, "touch:tuio:receive_objects");
	updateAnEngineSetting("touch_overlay:override_translation", outputSettings, theChillins, "touch:override_translation");
	updateAnEngineSetting("touch_overlay:dimensions", outputSettings, theChillins, "touch:dimensions");
	updateAnEngineSetting("touch_overlay:offset", outputSettings, theChillins, "touch:offset");
	updateAnEngineSetting("touch_overlay:filter_rect", outputSettings, theChillins, "touch:filter_rect");
	updateAnEngineSetting("touch_overlay:verbose_logging", outputSettings, theChillins, "touch:verbose_logging");
	updateAnEngineSetting("touch_overlay:debug", outputSettings, theChillins, "touch:debug");
	updateAnEngineSetting("touch_overlay:debug_circle_radius", outputSettings, theChillins, "touch:debug_circle_radius");
	updateAnEngineSetting("touch_color", outputSettings, theChillins, "touch:debug_circle_color");
	updateAnEngineSetting("touch_overlay:debug_circle_color", outputSettings, theChillins, "touch:debug_circle_color");
	updateAnEngineSetting("touch_overlay:debug_circle_filled", outputSettings, theChillins, "touch:debug_circle_filled");
	updateAnEngineSetting("touch:rotate_touches_default", outputSettings, theChillins, "touch:rotate_touches_default");
	updateAnEngineSetting("tap_threshold", outputSettings, theChillins, "touch:tap_threshold");
	updateAnEngineSetting("touch:minimum_distance", outputSettings, theChillins);
	updateAnEngineSetting("touch_smoothing", outputSettings, theChillins, "touch:smoothing");
	updateAnEngineSetting("touch_smooth_frames", outputSettings, theChillins, "touch:smooth_frames");
	updateAnEngineSetting("tap_threshold", outputSettings, theChillins, "touch:tap_threshold");
	updateAnEngineSetting("touch:swipe:queue_size", outputSettings, theChillins);
	updateAnEngineSetting("touch:swipe:minimum_velocity", outputSettings, theChillins);
	updateAnEngineSetting("touch:swipe:maximum_time", outputSettings, theChillins);

	// RESOURCE SETTINGS
	updateAnEngineSetting("resource_location", outputSettings, theChillins);
	updateAnEngineSetting("resource_db", outputSettings, theChillins);
	updateAnEngineSetting("configuration_folder:allow_expand_override", outputSettings, theChillins);
	updateAnEngineSetting("node:refresh_rate", outputSettings, theChillins);

	// LOGGER
	updateAnEngineSetting("logger:level", outputSettings, theChillins);
	updateAnEngineSetting("logger:module", outputSettings, theChillins);
	updateAnEngineSetting("logger:async", outputSettings, theChillins);
	updateAnEngineSetting("logger:file", outputSettings, theChillins);
	updateAnEngineSetting("smart_layout:verbose_logging", outputSettings, theChillins);

	// METRICS
	updateAnEngineSetting("metrics:active", outputSettings, theChillins);
	updateAnEngineSetting("metrics:send_base_info", outputSettings, theChillins);
	updateAnEngineSetting("metrics:base_info_send_delay", outputSettings, theChillins);
	updateAnEngineSetting("metrics:send_touch_info", outputSettings, theChillins);
	updateAnEngineSetting("metrics:udp_host", outputSettings, theChillins);
	updateAnEngineSetting("metrics:udp_port", outputSettings, theChillins);

	outputSettings.writeTo(fileToUpdate);

}

void SettingsUpdater::updateAnEngineSetting(const std::string& settingName, Settings& theSettings, ci::XmlTree::Container& theChillins, const std::string& newSettingName){
	std::string settingNameInNewSettings = newSettingName;
	if(settingNameInNewSettings.empty()) settingNameInNewSettings = settingName;

	for(auto it = theChillins.begin(); it != theChillins.end(); ++it){
		auto theType = (*it)->getTag();
		auto theName = (*it)->getAttributeValue<std::string>("name");
		if(theName == settingName){

			auto& theSetting = theSettings.getSetting(settingNameInNewSettings, 0);
			if(theType == "text" || theType == "float" || theType == "int"){
				theSetting.mRawValue = (*it)->getAttributeValue<std::string>("value");
			} else if(theType == "size"){
				std::stringstream ss;
				std::string xx = "0.0";
				std::string yy = "0.0";
				if((*it)->hasAttribute("x")) xx = (*it)->getAttribute("x");
				if((*it)->hasAttribute("y")) yy = (*it)->getAttribute("y");
				ss << xx << ", " << yy;
				theSetting.mRawValue = ss.str();
			} else if(theType == "point"){
				std::stringstream ss;
				std::string xx = "0.0";
				std::string yy = "0.0";
				std::string zz = "0.0";
				if((*it)->hasAttribute("x")) xx = (*it)->getAttribute("x");
				if((*it)->hasAttribute("y")) yy = (*it)->getAttribute("y");
				if((*it)->hasAttribute("z")) zz = (*it)->getAttribute("z");
				ss << xx << ", " << yy << ", " << zz;
				theSetting.mRawValue = ss.str();
			} else if(theType == "rect"){
				std::stringstream ss;
				std::string ll = "0.0";
				std::string tt = "0.0";
				std::string ww = "0.0";
				std::string hh = "0.0";
				if((*it)->hasAttribute("l")) ll = (*it)->getAttribute("l");
				if((*it)->hasAttribute("t")) tt = (*it)->getAttribute("t");
				if((*it)->hasAttribute("r") && (*it)->hasAttribute("b")){
					float rr = ds::string_to_float((*it)->getAttribute("r"));
					float bb = ds::string_to_float((*it)->getAttribute("b"));
					ww = std::to_string(rr - ds::string_to_float(ll));
					hh = std::to_string(bb - ds::string_to_float(tt));
				} else if((*it)->hasAttribute("w") && (*it)->hasAttribute("h")){
					ww = (*it)->getAttribute("w");
					hh = (*it)->getAttribute("h");
				}
				ss << ll << ", " << tt << ", " << ww << ", " << hh;
				theSetting.mRawValue = ss.str();
			} else if(theType == "color"){
				const float             DEFV = 255.0f;
				if((*it)->hasAttribute("code")) {
					theSetting.mRawValue = (*it)->getAttribute("code");

				} else if((*it)->hasAttribute("r")){
					ci::ColorA c = ci::ColorA((*it)->getAttributeValue<float>("r", DEFV) / DEFV,
											  (*it)->getAttributeValue<float>("g", DEFV) / DEFV,
											  (*it)->getAttributeValue<float>("b", DEFV) / DEFV,
											  (*it)->getAttributeValue<float>("a", DEFV) / DEFV);
					theSetting.mRawValue = ds::ARGBToHex(c);

				} else if((*it)->hasAttribute("hex")){

					cinder::ColorA			c;
					std::string s = (*it)->getAttributeValue<std::string>("hex");

					if(boost::starts_with(s, "#")){
						boost::erase_head(s, 1);
					}

					if(boost::starts_with(s, "0x")){
						boost::erase_head(s, 2);
					}

					std::stringstream converter(s);
					unsigned int value;
					converter >> std::hex >> value;

					float a = (s.length() > 6)
						? ((value >> 24) & 0xFF) / 255.0f
						: 1.0f;
					float r = ((value >> 16) & 0xFF) / 255.0f;
					float g = ((value >> 8) & 0xFF) / 255.0f;
					float b = ((value)& 0xFF) / 255.0f;

					c = ci::ColorA(r, g, b, a);
					theSetting.mRawValue = ds::ARGBToHex(c);
				}
			}
		}
	}
}

void SettingsUpdater::updateSettings(const std::string& source, const std::string& destination, const bool allowComments){
	if(source.empty() || destination.empty()) return;


	Settings outputSettings;


	ci::XmlTree xml;
	ci::XmlTree::ParseOptions parseOptions;
	parseOptions.setParseComments(allowComments);
	try{
		xml = ci::XmlTree(ci::loadFile(source), parseOptions);
	} catch(std::exception& e){
		DS_LOG_WARNING("Exception loading settings from " << source << " " << e.what());
		return;
	}
	ci::XmlTree rootNode = xml.getChild("settings");
	ci::XmlTree::Container& theChillins = rootNode.getChildren();

	std::string theComment;

	bool nextIsHeader = false;
	int readIndex = 1;
	for(auto it = theChillins.begin(); it != theChillins.end(); ++it){
		if((*it)->getTag() == "setting"){
			DS_LOG_WARNING("It looks like this file is already in the new format! Abandoning update.");
			return;
		}

		if((*it)->isComment()){
			std::string theValue = (*it)->getValue();

			if(theValue.find("----") == 0){
				nextIsHeader = true;
			} else if(nextIsHeader){
				nextIsHeader = false;
				Settings::Setting newSetting;
				newSetting.mName = theValue;
				if(!theComment.empty()){
					newSetting.mComment = theComment;
					theComment = "";
				}
				newSetting.mType = SETTING_TYPE_SECTION_HEADER;
				newSetting.mSource = source;
				newSetting.mReadIndex = readIndex++;
				outputSettings.addSetting(newSetting);

			} else {
				theComment.append(theValue);
			}
				
		} else {
			nextIsHeader = false;
			Settings::Setting newSetting;
			newSetting.mReadIndex = readIndex++;

			if((*it)->hasAttribute("name")) newSetting.mName = (*it)->getAttribute("name");
			if((*it)->hasAttribute("value")) newSetting.mRawValue = (*it)->getAttribute("value");

			std::string theTag = (*it)->getTag();


			if(theTag == "text"){
				if(newSetting.mRawValue == "true" || newSetting.mRawValue == "false"){
					newSetting.mType = SETTING_TYPE_BOOL;
				} else {
					newSetting.mType = SETTING_TYPE_STRING;
				}
			} else if(theTag == "wtext"){
				newSetting.mType = SETTING_TYPE_WSTRING;
			} else if(theTag == "float"){
				newSetting.mType = SETTING_TYPE_FLOAT;
			} else if(theTag == "int" || theTag == "resource_id"){
				newSetting.mType = SETTING_TYPE_INT;
			} else if(theTag == "size"){
				std::stringstream ss;
				std::string xx = "0.0";
				std::string yy = "0.0";
				if((*it)->hasAttribute("x")) xx = (*it)->getAttribute("x");
				if((*it)->hasAttribute("y")) yy = (*it)->getAttribute("y");
				ss << xx << ", " << yy;
				newSetting.mRawValue = ss.str();
				newSetting.mType = SETTING_TYPE_VEC2;
			} else if(theTag == "point"){
				std::stringstream ss;
				std::string xx = "0.0";
				std::string yy = "0.0";
				std::string zz = "0.0";
				if((*it)->hasAttribute("x")) xx = (*it)->getAttribute("x");
				if((*it)->hasAttribute("y")) yy = (*it)->getAttribute("y");
				if((*it)->hasAttribute("z")) zz = (*it)->getAttribute("z");
				ss << xx << ", " << yy << ", " << zz;
				newSetting.mRawValue = ss.str();
				newSetting.mType = SETTING_TYPE_VEC3;
			} else if(theTag == "rect"){
				std::stringstream ss;
				std::string ll = "0.0";
				std::string tt = "0.0";
				std::string ww = "0.0";
				std::string hh = "0.0";
				if((*it)->hasAttribute("l")) ll = (*it)->getAttribute("l");
				if((*it)->hasAttribute("t")) tt = (*it)->getAttribute("t");
				if((*it)->hasAttribute("r") && (*it)->hasAttribute("b")){
					float rr = ds::string_to_float((*it)->getAttribute("r"));
					float bb = ds::string_to_float((*it)->getAttribute("b"));
					ww = std::to_string(rr - ds::string_to_float(ll));
					hh = std::to_string(bb - ds::string_to_float(tt));
				} else if((*it)->hasAttribute("w") && (*it)->hasAttribute("h")){
					ww = (*it)->getAttribute("w");
					hh = (*it)->getAttribute("h");
				}
				ss << ll << ", " << tt << ", " << ww << ", " << hh;
				newSetting.mRawValue = ss.str();
				newSetting.mType = SETTING_TYPE_RECT;
			} else if(theTag == "color"){

				const float             DEFV = 255.0f;
				if((*it)->hasAttribute("code")) {
					newSetting.mRawValue = (*it)->getAttribute("code");
					
				} else if((*it)->hasAttribute("r")){
					ci::ColorA c = ci::ColorA((*it)->getAttributeValue<float>("r", DEFV) / DEFV,
								   (*it)->getAttributeValue<float>("g", DEFV) / DEFV,
								   (*it)->getAttributeValue<float>("b", DEFV) / DEFV,
								   (*it)->getAttributeValue<float>("a", DEFV) / DEFV);
					newSetting.mRawValue = ds::ARGBToHex(c);

				} else if((*it)->hasAttribute("hex")){

					cinder::ColorA			c;
					std::string s = (*it)->getAttributeValue<std::string>("hex");

					if(boost::starts_with(s, "#")){
						boost::erase_head(s, 1);
					}

					if(boost::starts_with(s, "0x")){
						boost::erase_head(s, 2);
					}

					std::stringstream converter(s);
					unsigned int value;
					converter >> std::hex >> value;

					float a = (s.length() > 6)
						? ((value >> 24) & 0xFF) / 255.0f
						: 1.0f;
					float r = ((value >> 16) & 0xFF) / 255.0f;
					float g = ((value >> 8) & 0xFF) / 255.0f;
					float b = ((value)& 0xFF) / 255.0f;

					c = ci::ColorA(r, g, b, a);
					newSetting.mRawValue = ds::ARGBToHex(c);
				}


				newSetting.mType = SETTING_TYPE_COLOR;
			}


			newSetting.mComment = theComment;
			theComment = "";
			newSetting.mSource = source;
			outputSettings.addSetting(newSetting);

		}


	}

	//outputSettings.printAllSettings();

	outputSettings.writeTo(destination);
}

} // namespace cfg
} // namespace ds