#pragma once

#include <ds/app/event_client.h>
#include <ds/data/resource_list.h>
#include <ds/thread/serial_runnable.h>
#include <ds/thread/parallel_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>


namespace ds::model {

/**
 * \class NWQueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class NWQueryHandler {
public:
	NWQueryHandler(ds::ui::SpriteEngine& eng);

private:
	virtual void									handleQuery();

	virtual void									addReference(ds::model::ContentModelRef curParent, std::map<int, ds::model::ContentModelRef>& overallMap);
	virtual void									parseModelProperties(ds::model::ContentModelRef& node,std::vector<ds::model::ContentModelRef>& allNodes);

	ds::Resource							processResource(ds::Resource input);
	ds::EventClient							mEventClient;
	ds::ui::SpriteEngine&					mEngine;

	

	std::string								mPlatformKey;
};

} // !namespace schneider
