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


void SettingsUpdater::updateSettings(const std::string& source, const std::string& destination){
	if(source.empty() || destination.empty()) return;


	SettingsManager outputSettings(mEngine);


	ci::XmlTree xml;
	ci::XmlTree::ParseOptions parseOptions;
	parseOptions.setParseComments(true);
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
	for(auto it = theChillins.begin(); it != theChillins.end(); ++it){
		if((*it)->isComment()){
			std::string theValue = (*it)->getValue();

			if(theValue.find("----") == 0){
				nextIsHeader = true;
			} else if(nextIsHeader){
				nextIsHeader = false;
				SettingsManager::Setting newSetting;
				newSetting.mName = theValue;
				if(!theComment.empty()){
					newSetting.mComment = theComment;
					theComment = "";
				}
				newSetting.mType = SETTING_TYPE_SECTION_HEADER;
				newSetting.mSource = source;
				outputSettings.addSetting(newSetting);

			} else {
				theComment.append(theValue);
			}
				
		} else {
			nextIsHeader = false;
			SettingsManager::Setting newSetting;

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
					ww = std::to_string(rr + ds::string_to_float(ll));
					hh = std::to_string(bb + ds::string_to_float(tt));
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


				newSetting.mType = SETTING_TYPE_COLORA;
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