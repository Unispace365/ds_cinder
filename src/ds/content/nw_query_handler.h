#pragma once

#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>


namespace ds::model {

/**
 * \class NWQueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class NWQueryHandler {
  public:
	NWQueryHandler(ui::SpriteEngine& eng);

  private:
	virtual void handleQuery();

	virtual void addReference(ContentModelRef curParent, std::map<int, ContentModelRef>& overallMap);
	virtual void parseModelProperties(ContentModelRef& node, std::vector<ContentModelRef>& allNodes);

	Resource processResource(const Resource& input) const;

	ui::SpriteEngine& mEngine;
	EventClient		  mEventClient;


	std::string mPlatformKey;
};

} // namespace ds::model
