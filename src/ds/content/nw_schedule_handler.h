#pragma once

#include <Poco/DateTime.h>
#include <ds/app/auto_update.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds::model {

/**
 * \class schneider::NWScheduleHandler
 * \brief Check the event schedule and let the rest of the app know when there's a change to the current event
 */
class NWScheduleHandler : public AutoUpdate {
  public:
	NWScheduleHandler(ui::SpriteEngine& eng);

	void update(const UpdateParams&) override;

  protected:
	virtual void checkSchedule();
	virtual bool eventIsNow(ContentModelRef& theEvent, Poco::DateTime& ldt);

	ui::SpriteEngine& mEngine;
	EventClient		  mEventClient;
};

} // namespace ds::model
