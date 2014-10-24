#include "globals.h"

#include <Poco/String.h>

#include <ds/debug/logger.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

namespace importer_example {

/**
 * \class importer_example::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e )
		: mEngine(e)
{
	reloadXml();
}

void Globals::reloadXml()
{

	std::vector<std::string> xmls;
	xmls.push_back("sample.xml");

	std::string path = "%APP%/settings/layouts/";
	std::stringstream ss;
	for(auto it = xmls.begin(); it < xmls.end(); ++it){
		ss.str("");
		ss << path << (*it);

		ds::ui::XmlImporter::XmlPreloadData pld;
		if(!ds::ui::XmlImporter::preloadXml(ds::Environment::expand(ss.str()), pld)){
			DS_LOG_WARNING("Trubs loading " << (*it));
		}
		mXmlImporterMap[(*it)] = pld;
	}


}

const ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

const ds::cfg::Settings& Globals::getSettingsLayout() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}


const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);

}



} // !namespace importer_example
