#include "globals.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include <Poco/String.h>

#include "app_defs.h"

namespace globe_example {

/**
 * \class globe_example::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e)
  : mEngine(e) {
	genDataModel();
}

ds::cfg::Settings& Globals::getSettingsLayout() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}


void Globals::genDataModel() {

	{
		ds::model::LocationRef lr;
		lr.setId(1);
		lr.setLayer(1);
		lr.setName(L"Portland, OR");
		lr.setPopulation(L"639,863");
		lr.setLat(45.5261115);
		lr.setLong(-122.7411413);
		mLocations.push_back(lr);
	}

	{
		ds::model::LocationRef lr;
		lr.setId(1);
		lr.setLayer(2);
		lr.setName(L"Reykjavik, IS");
		lr.setPopulation(L"216,940");
		lr.setLat(64.1285547);
		lr.setLong(-21.9048037);
		mLocations.push_back(lr);
	}

	{
		ds::model::LocationRef lr;
		lr.setId(1);
		lr.setLayer(2);
		lr.setName(L"Stockholm, SW");
		lr.setPopulation(L"789,024");
		lr.setLat(59.309023);
		lr.setLong(17.9174867);
		mLocations.push_back(lr);
	}

	{
		ds::model::LocationRef lr;
		lr.setId(1);
		lr.setLayer(3);
		lr.setName(L"Nairobi, Kenya");
		lr.setPopulation(L"3,138,000");
		lr.setLat(-1.3031934);
		lr.setLong(36.5672003);
		mLocations.push_back(lr);
	}
}


} // namespace globe_example
