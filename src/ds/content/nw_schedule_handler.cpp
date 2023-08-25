#include "stdafx.h"

#include "nw_schedule_handler.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/content/content_events.h>
#include <ds/debug/logger.h>

// #include "events/app_events.h"

namespace ds::model {

NWScheduleHandler::NWScheduleHandler(ds::ui::SpriteEngine& eng)
  : mEngine(eng)
  , mEventClient(eng)
  , AutoUpdate(eng, 1) {
	auto currentEvents = mEngine.mContent.getChildByName("current_events");
	if (currentEvents.empty()) {
		mEngine.mContent.addChild(ds::model::ContentModelRef("current_events"));
	}
	mEventClient.listenToEvents<CmsDataLoadCompleteEvent>([this](auto& e) {
		DS_LOG_INFO("Checking schedule due to content change")
		checkSchedule();
	});

	mEngine.repeatedCallback([this] { checkSchedule(); }, 5.0);
}

void NWScheduleHandler::update(const ds::UpdateParams&) {}

void NWScheduleHandler::checkSchedule() {
	Poco::LocalDateTime ldt = Poco::LocalDateTime();
	Poco::DateTime		thisDayTime;
	thisDayTime.makeLocal(ldt.tzd());


	auto currentEvents = mEngine.mContent.getChildByName("current_events");
	auto cmsEvents	   = mEngine.mContent.getChildByName("cms_events").getChildren();

	std::vector<ds::model::ContentModelRef> newCurrentEvents;

	for (auto it : cmsEvents) {
		try {
			if (eventIsNow(it, thisDayTime)) {
				newCurrentEvents.emplace_back(it);

				//  DS_LOG_INFO("Current event: " << it.getPropertyString("name"));
			}
		} catch (std::exception& e) {
			DS_LOG_WARNING("NWScheduleHandler: exception checking the event " << it.getPropertyString("name") << ": "
																			  << e.what());
		}
	}

	// TODO: compare the lists?

	if (!newCurrentEvents.empty()) {
		auto						theFirstEvent	 = newCurrentEvents.front();
		auto						briefingPinboard = theFirstEvent.getPropertyListInt("briefing_pinboard");
		ds::model::ContentModelRef& thePinboard		 = mEngine.mContent.getChildByName("current_pinboard");

		std::vector<ds::model::ContentProperty> theNewPinboardList;
		for (auto it : briefingPinboard) {
			theNewPinboardList.emplace_back(ds::model::ContentProperty("pinboard_items", std::to_string(it), it, it));
		}
		thePinboard.setPropertyList("pinboard_items", theNewPinboardList);
	}

	currentEvents.setChildren(newCurrentEvents);
	mEngine.getNotifier().notify(ScheduleUpdatedEvent());
}

bool NWScheduleHandler::eventIsNow(ds::model::ContentModelRef& it, Poco::DateTime& ldt) {

	int tzd = 0;

	/// ---------- Check the effective dates
	Poco::DateTime startDate;
	Poco::DateTime endDate;
	if (!Poco::DateTimeParser::tryParse(it.getPropertyString("effective_date_start"), startDate, tzd)) {
		DS_LOG_WARNING("Couldn't parse the start date for an event!");
		return false;
	}
	if (!Poco::DateTimeParser::tryParse(it.getPropertyString("effective_date_end"), endDate, tzd)) {
		DS_LOG_WARNING("Couldn't parse the end date for an event!");
		return false;
	}

	Poco::Timespan daySpan = Poco::Timespan(1, 0, 0, 0, 0);
	endDate += daySpan;

	if (ldt < startDate || ldt > endDate) {
		DS_LOG_VERBOSE(3, "Event happens outside the current yeah: " << it.getPropertyString("name"));
		return false;
	}


	/// ---------- Check the effective times of day
	Poco::DateTime startTime;
	Poco::DateTime endTime;
	if (!Poco::DateTimeParser::tryParse("%H:%M:%S", it.getPropertyString("effective_time_start"), startTime, tzd)) {
		DS_LOG_WARNING("Couldn't parse the start time for an event!");
		return false;
	}
	if (!Poco::DateTimeParser::tryParse("%H:%M:%S", it.getPropertyString("effective_time_end"), endTime, tzd)) {
		("Couldn't parse the end time for an event!");
		return false;
	}

	int daySeconds		= ldt.hour() * 60 * 60 + ldt.minute() * 60 + ldt.second();
	int startDaySeconds = startTime.hour() * 60 * 60 + startTime.minute() * 60 + startTime.second();
	int endDaySeconds	= endTime.hour() * 60 * 60 + endTime.minute() * 60 + endTime.second();


	if (daySeconds < startDaySeconds || daySeconds > endDaySeconds) {
		DS_LOG_VERBOSE(3, "Event happens outside the current time: " << it.getPropertyString("name"));
		return false;
	}

	/// ---------- Check the effective days of the week
	static const int WEEK_SUN = 1;
	static const int WEEK_MON = 2;
	static const int WEEK_TUE = 4;
	static const int WEEK_WED = 8;
	static const int WEEK_THU = 16;
	static const int WEEK_FRI = 32;
	static const int WEEK_SAT = 64;
	static const int WEEK_ALL = 127;

	// Returns the weekday (0 to 6, where
	/// 0 = Sunday, 1 = Monday, ..., 6 = Saturday).
	auto dotw	 = ldt.dayOfWeek();
	int	 dayFlag = 0;

	if (dotw == 0) dayFlag = WEEK_SUN;
	if (dotw == 1) dayFlag = WEEK_MON;
	if (dotw == 2) dayFlag = WEEK_TUE;
	if (dotw == 3) dayFlag = WEEK_WED;
	if (dotw == 4) dayFlag = WEEK_THU;
	if (dotw == 5) dayFlag = WEEK_FRI;
	if (dotw == 6) dayFlag = WEEK_SAT;


	int effectiveWeekdays = it.getPropertyInt("effective_weekdays");
	if (effectiveWeekdays == WEEK_ALL || effectiveWeekdays & dayFlag) {
		return true;
	}

	return false;
}

} // namespace ds::model
