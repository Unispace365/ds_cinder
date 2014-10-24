#ifndef _INTERFACEXMLIMPORTEREXAMPLEAPP_APP_GLOBALS_
#define _INTERFACEXMLIMPORTEREXAMPLEAPP_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/cfg/cfg_nine_patch.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

namespace ds {
namespace ui {
class SpriteEngine;
} // namespace ui
} // namespace ds

namespace importer_example {

/**
 * \class importer_example::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine&			mEngine;
	ds::EventNotifier				mNotifier;


	std::map<std::string, ds::ui::XmlImporter::XmlPreloadData>	mXmlImporterMap;
	void							reloadXml();

	//Shortcuts
	const ds::cfg::Text&			getText(const std::string& name) const;
	const ds::cfg::Settings&		getSettingsLayout() const;
	const ds::cfg::Settings&		getSettings(const std::string& name) const;


};

} // !namespace importer_example

#endif // !_INTERFACEXMLIMPORTEREXAMPLEAPP_APP_GLOBALS_