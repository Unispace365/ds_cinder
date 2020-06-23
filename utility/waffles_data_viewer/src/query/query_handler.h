#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace downstream {

/**
 * \class downstream::QueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class QueryHandler {
public:
	QueryHandler(ds::ui::SpriteEngine& eng);

private:
	void									addReference(ds::model::ContentModelRef curParent, std::map<int, ds::model::ContentModelRef>& overallMap);
	bool									createTempStreamModel(const std::string& settingsName,
		const int fakeId, std::vector<ds::model::ContentModelRef>& streamList);

	void									parseModelProperties(ds::model::ContentModelRef& node);

	ds::Resource							processResource(ds::Resource input);
	ds::EventClient							mEventClient;
	ds::ui::SpriteEngine&					mEngine;


	std::string								mComputerName;
};

} // !namespace schneider
