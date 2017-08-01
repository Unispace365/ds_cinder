#include "stdafx.h"

#include "settings_updater.h"

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

	for(auto it = theChillins.begin(); it != theChillins.end(); ++it){
		if((*it)->isComment()){
			theComment.append((*it)->getValue());
			std::cout << "Xml Comment: " << (*it)->getTag() << " " << (*it)->getValue() << std::endl;
		} else {
			std::cout << "Xml setting: " << (*it)->getTag() << " " << (*it)->getValue() << std::endl;

			SettingsManager::Setting newSetting;

			if((*it)->hasAttribute("name")) newSetting.mName = (*it)->getAttribute("name");
			if((*it)->hasAttribute("value")) newSetting.mRawValue = (*it)->getAttribute("value");

			if((*it)->getTag() == "text"){
				if(newSetting.mRawValue == "true" || newSetting.mRawValue == "false"){
					newSetting.mType = "boolean";
				} else {
					newSetting.mType = "string";
				}
			} else if((*it)->getTag() == "float"){
				newSetting.mType = "float";
			} else if((*it)->getTag() == "size"){
				std::stringstream ss;
				std::string xx = "0.0";
				std::string yy = "0.0";
				if((*it)->hasAttribute("x")) xx = (*it)->getAttribute("x");
				if((*it)->hasAttribute("y")) yy = (*it)->getAttribute("y");
				ss << xx << ", " << yy;
				newSetting.mRawValue = ss.str();
				newSetting.mType = "vec2";
			} else if((*it)

			

			// size, int, float, rect, color

			newSetting.mComment = theComment;
			theComment = "";
			newSetting.mSource = source;
			outputSettings.addSetting(newSetting);

		}


	}

	outputSettings.printAllSettings();
}

} // namespace cfg
} // namespace ds