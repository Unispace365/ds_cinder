#pragma once
#ifndef DS_CFG_SETTINGS_UPDATER_H_
#define DS_CFG_SETTINGS_UPDATER_H_

#include <map>
#include <vector>
#include <cinder/Color.h>
#include <cinder/Rect.h>
#include "ds/data/resource.h"

namespace cinder {
class XmlTree;
}

namespace ds {
class Engine;

namespace cfg {

/**
* \class ds::cfg::SettingsUpdater
* \brief Convert the Classic(tm) settings style to the new SettingsManager
*/
class SettingsUpdater {
public:
	SettingsUpdater(ds::Engine& engine);

	// Read from an xml file 
	void								updateEngineSettings(const std::string& fileToUpdate);
	void								updateSettings(const std::string& source, const std::string& destination, const bool allowComments);

protected:
	ds::Engine&							mEngine;

	void								updateAnEngineSetting(const std::string& settingName, Settings& theSettings, ci::XmlTree::Container& theXml, const std::string& newSettingName = "");

};

} // namespace cfg
} // namespace ds

#endif // DS_CFG_SETTINGS_H_
