#include "stdafx.h"

#include "bridge_service.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#include <ds/content/content_events.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/string_util.h>

namespace ds::content {

	struct SchemaCompleteEvent : public ds::RegisteredEvent<SchemaCompleteEvent> {};
	struct IndexCompleteEvent : public ds::RegisteredEvent<IndexCompleteEvent> {};

	BridgeService::BridgeService(ds::ui::SpriteEngine& engine)
		: mEngine(engine)
		, mRefreshTimer(engine)
		, mNodeWatcher(engine, "localhost", 7788)
		, mLoop(engine) {
		mThread.setName("BridgeService");
	}

	BridgeService::~BridgeService() {
		stop();

		try {
			mThread.join();
		}
		catch (std::exception&) {}
	}

	void BridgeService::start() {
		if (!mThread.isRunning()) {
			try {
				mThread.start(mLoop);
			}
			catch (std::exception& ex) {
				DS_LOG_WARNING("BridgeService::start() threw an exception: " << ex.what())
			}

			mNodeWatcher.setDelayTime(0.3f);
			mNodeWatcher.setDelayedMessageNodeCallback([this](const ds::NodeWatcher::Message& msg) {
				// Will be called from the main thread.

				std::string authHash;
				for (const auto& m : msg.mData) {
					authHash.append(m);
				}

				if (!authHash.empty()) {
					auto server = mEngine.mContent.getChildByName("server");
					server.setName("server");
					server.setProperty("auth", authHash);
					mEngine.mContent.replaceChild(server);
				}

				refreshDatabase();
				});

			mNodeWatcher.startWatching();
		}

		// Refresh content regularly.
		mRefreshTimer.repeatedCallback([this] { refreshEvents(); }, 2.f);

		refreshDatabase(mThread.isRunning());
	}

	void BridgeService::stop() {
		if (!mThread.isRunning()) return;
		try {
			mRefreshTimer.cancel();
			mLoop.abort();
			mThread.wakeUp();
		}
		catch (std::exception& ex) {
			DS_LOG_WARNING("BridgeService::stop() threw an exception: " << ex.what())
		}
	}

	void BridgeService::refreshDatabase(bool force) {
		mLoop.refreshDatabase(force);
		mThread.wakeUp();
	}

	void BridgeService::refreshEvents(bool force) {
		mLoop.refreshEvents(force);
		mThread.wakeUp();
	}

	BridgeService::Loop::Loop(ds::ui::SpriteEngine& engine)
		: mApp(ci::app::App::get())
		, mEngine(engine)
		, mAbort(false)
		, mForce(false)
		, mRefreshDatabase(true) // Force refresh on start.
		, mRefreshEvents(false)
		, mRefreshRateMs(10000) {
	}

	void BridgeService::Loop::run() {
		while (true) {
			DS_LOG_VERBOSE(2, "BridgeService::Loop has woken up")

				mMutex.lock();

			bool abort = mAbort;
			bool contentChanged = mForce;
			bool refreshDatabase = mRefreshDatabase;
			bool refreshEvents = mRefreshEvents;

			mForce = false;
			mRefreshDatabase = false;
			mRefreshEvents = false;

			// Don't hold lock while processing content.
			mMutex.unlock();

			// Abort the loop on request.
			if (abort) break;

			if (refreshDatabase) {
				DS_LOG_VERBOSE(2, "BridgeService::Loop is refreshing content")

				{
					Poco::Mutex::ScopedLock l(mContentMutex);

					loadContent();
					updatePlatformEvents();

					// Prevent frequent events by checking if the content has changed.
					contentChanged |= !(mEngine.mContent.getChildByName(mPlatforms.getName()) == mPlatforms);
					contentChanged |= !(mEngine.mContent.getChildByName(mContent.getName()) == mContent);
					contentChanged |= !(mEngine.mContent.getChildByName(mEvents.getName()) == mEvents);
					contentChanged |= !(mEngine.mContent.getChildByName(mRecords.getName()) == mRecords);
					contentChanged |= !(mEngine.mContent.getChildByName(mTags.getName()) == mTags);
				}

				// Replace content on the main thread only!
				if (contentChanged && mApp) {
					DS_LOG_VERBOSE(2, "BridgeService::Loop is replacing content")

						mApp->dispatchSync([&]() {
						mEngine.getResources().clear();
						{
							Poco::Mutex::ScopedLock l(mContentMutex);
							mEngine.mContent.replaceChild(mPlatforms);
							mEngine.mContent.replaceChild(mContent);
							mEngine.mContent.replaceChild(mEvents);
							mEngine.mContent.replaceChild(mRecords);
							mEngine.mContent.replaceChild(mTags);
						}
						mEngine.getNotifier().notify(ds::CmsDataLoadCompleteEvent());
						mEngine.getNotifier().notify(ds::ContentUpdatedEvent());
						mEngine.getNotifier().notify(ds::ScheduleUpdatedEvent());
							});
				}
			}
			else if (refreshEvents) {
				DS_LOG_VERBOSE(2, "BridgeService::Loop is refreshing events")

				{
					Poco::Mutex::ScopedLock l(mContentMutex);

					updatePlatformEvents();

					// Prevent frequent events by checking if the content has changed.
					contentChanged |= !(mEngine.mContent.getChildByName(mEvents.getName()) == mEvents);
				}

				// Replace content on the main thread only!
				if (contentChanged && mApp) {
					DS_LOG_VERBOSE(2, "BridgeService::Loop is replacing events")

						mApp->dispatchSync([&]() {
						mEngine.getResources().clear();
						{
							Poco::Mutex::ScopedLock l(mContentMutex);
							mEngine.mContent.replaceChild(mEvents);
						}
						mEngine.getNotifier().notify(ds::ScheduleUpdatedEvent());
							});
				}
			}

			// Sleep until woken up externally.
			while (Poco::Thread::trySleep(mRefreshRateMs)) {}
		}

		DS_LOG_VERBOSE(2, "BridgeService::Loop has terminated")
	}

	void BridgeService::Loop::refreshDatabase(bool force) {
		Poco::Mutex::ScopedLock l(mMutex);
		mRefreshDatabase = true;
		mForce = force;
	}

	void BridgeService::Loop::refreshEvents(bool force) {
		Poco::Mutex::ScopedLock l(mMutex);
		mRefreshEvents = true;
		mForce = force;
	}

	void BridgeService::Loop::abort() {
		Poco::Mutex::ScopedLock l(mMutex);
		mAbort = true;
	}

	bool BridgeService::Loop::eventIsNow(ds::model::ContentModelRef& event, Poco::DateTime& ldt) const {
		int tzd = 0;

		/// ---------- Check the effective dates
		Poco::DateTime startDate;
		Poco::DateTime endDate;
		if (!Poco::DateTimeParser::tryParse(event.getPropertyString("start_date"), startDate, tzd)) {
			DS_LOG_WARNING("Couldn't parse the start date for an event!")
				return false;
		}
		if (!Poco::DateTimeParser::tryParse(event.getPropertyString("end_date"), endDate, tzd)) {
			DS_LOG_WARNING("Couldn't parse the end date for an event!")
				return false;
		}

		Poco::Timespan daySpan = Poco::Timespan(1, 0, 0, 0, 0);
		endDate += daySpan;

		if (ldt < startDate || ldt > endDate) {
			DS_LOG_VERBOSE(3, "Event happens outside the current date: " << event.getPropertyString("name"))
				return false;
		}

		/// ---------- Check the effective times of day
		if (!event.getPropertyString("start_time").empty()) {
			Poco::DateTime startTime;
			Poco::DateTime endTime;
			if (!Poco::DateTimeParser::tryParse("%H:%M:%S", event.getPropertyString("start_time"), startTime, tzd)) {
				DS_LOG_WARNING("Couldn't parse the start time for an event!")
					return false;
			}
			if (!Poco::DateTimeParser::tryParse("%H:%M:%S", event.getPropertyString("end_time"), endTime, tzd)) {
				DS_LOG_WARNING("Couldn't parse the end time for an event!")
					return false;
			}

			int daySeconds = ldt.hour() * 60 * 60 + ldt.minute() * 60 + ldt.second();
			int startDaySeconds = startTime.hour() * 60 * 60 + startTime.minute() * 60 + startTime.second();
			int endDaySeconds = endTime.hour() * 60 * 60 + endTime.minute() * 60 + endTime.second();


			if (daySeconds < startDaySeconds || daySeconds > endDaySeconds) {
				DS_LOG_VERBOSE(3, "Event happens outside the current time: " << event.getPropertyString("name"))
					return false;
			}
		}

		/// ---------- Check the effective days of the week
		static const int WEEK_SUN = 0b00000001;
		static const int WEEK_MON = 0b00000010;
		static const int WEEK_TUE = 0b00000100;
		static const int WEEK_WED = 0b00001000;
		static const int WEEK_THU = 0b00010000;
		static const int WEEK_FRI = 0b00100000;
		static const int WEEK_SAT = 0b01000000;
		static const int WEEK_ALL = 0b01111111;

		// Returns the weekday (0 to 6, where
		/// 0 = Sunday, 1 = Monday, ..., 6 = Saturday).
		auto dotw = ldt.dayOfWeek();
		int	 dayFlag = 0;

		if (dotw == 0) dayFlag = WEEK_SUN;
		if (dotw == 1) dayFlag = WEEK_MON;
		if (dotw == 2) dayFlag = WEEK_TUE;
		if (dotw == 3) dayFlag = WEEK_WED;
		if (dotw == 4) dayFlag = WEEK_THU;
		if (dotw == 5) dayFlag = WEEK_FRI;
		if (dotw == 6) dayFlag = WEEK_SAT;


		int effectiveDays = event.getPropertyInt("effective_days");
		if (effectiveDays == WEEK_ALL || effectiveDays & dayFlag) {
			return true;
		}

		return false;
	}

	void BridgeService::Loop::loadContent() {
		// DS_LOG_INFO("BridgeService::loadContent")

		const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);

		std::unordered_map<std::string, ds::model::ContentModelRef> recordMap;
		std::unordered_map<std::string, std::string>				selectMap;

		std::unordered_map<std::string, std::pair<std::string, bool>> slotReverseOrderingMap;
		{
			ds::query::Result result;
			std::string		  slotQuery = "SELECT "		  //
				" l.uid,"			  // 0
				" l.app_key,"		  // 1
				" l.reverse_ordering" // 2
				" FROM lookup AS l"
				" WHERE l.type = 'slot' ";
			if (ds::query::Client::query(cms.getDatabasePath(), slotQuery, result)) {
				ds::query::Result::RowIterator it(result);
				while (it.hasValue()) {
					slotReverseOrderingMap[it.getString(0)] = { it.getString(1), it.getInt(2) };
					++it;
				}
			}
		}

		std::vector<ds::model::ContentModelRef> rankOrderedRecords;
		{
			ds::query::Result result;
			std::string		  recordQuery = "SELECT "			//
				" r.uid,"					// 0
				" r.type_uid,"			// 1
				" l.name as type_name,"	// 2
				" l.app_key as type_key," // 3
				" r.parent_uid,"			// 4
				" r.parent_slot,"			// 5
				" r.variant,"				// 6
				" r.name,"				// 7
				" r.span_type,"			// 8
				" r.span_start_date,"		// 9
				" r.span_end_date,"		// 10
				" r.start_time,"			// 11
				" r.end_time,"			// 12
				" r.effective_days"		// 13
				" FROM record AS r"
				" INNER JOIN lookup AS l ON l.uid = r.type_uid"
				" WHERE r.complete = 1 AND r.visible = 1 AND (r.span_end_date IS NULL OR "
				" date(r.span_end_date, '+5 day') > date('now'))"
				" ORDER BY r.parent_slot ASC, r.rank ASC;";


			if (ds::query::Client::query(cms.getDatabasePath(), recordQuery, result)) {
				ds::query::Result::RowIterator it(result);
				int							   recordId = 1;
				while (it.hasValue()) {
					auto record = ds::model::ContentModelRef(it.getString(7) + "(" + it.getString(0) + ")");
					record.setId(recordId);

					record.setProperty("record_name", it.getString(7));
					record.setProperty("uid", it.getString(0));
					record.setProperty("type_uid", it.getString(1));
					record.setProperty("type_name", it.getString(2));
					record.setProperty("type_key", it.getString(3));
					if (!it.getString(4).empty()) record.setProperty("parent_uid", it.getString(4));
					if (!it.getString(5).empty()) {
						record.setProperty("parent_slot", it.getString(5));
						record.setLabel(slotReverseOrderingMap[it.getString(5)].first);
						record.setProperty("reverse_ordered", slotReverseOrderingMap[it.getString(5)].second);
					}

					const auto& variant = it.getString(6);
					record.setProperty("variant", variant);
					if (variant == "SCHEDULE") {
						record.setProperty("span_type", it.getString(8));
						record.setProperty("start_date", it.getString(9));
						record.setProperty("end_date", it.getString(10));
						record.setProperty("start_time", it.getString(11));
						record.setProperty("end_time", it.getString(12));
						record.setProperty("effective_days", it.getInt(13));
					}

					// Put it in the map so we can wire everything up into the tree
					recordMap[it.getString(0)] = record;

					rankOrderedRecords.push_back(record);
					++it;
				}
			}


			// "Sort" individual slot groups by reversing the reverse_ordered ones
			// This tracks the begininng and end of each slot section and reverses as needed each time it reaches a new
			// section.
			/* int		sectionStart = 0;
			int		sectionEnd	 = 0;
			std::string lastSlot;
			for (int i = 0; i < rankOrderedRecords.size(); ++i) {
				auto& r = rankOrderedRecords.at(i);
				if (lastSlot.empty()) {
					lastSlot = r.getPropertyString("parent_slot");
				} else if (lastSlot != r.getPropertyString("parent_slot")) {
					if (sectionStart != sectionEnd &&
			rankOrderedRecords.at(sectionStart).getPropertyBool("reverse_ordered")) {
						std::reverse((rankOrderedRecords.begin()+sectionStart), (rankOrderedRecords.begin()+sectionEnd));
					}
					lastSlot	 = r.getPropertyString("parent_slot");
					sectionStart = i;
				}

				sectionEnd = i;
			}
			// Ensure we also reverse the final group if required
			if (sectionStart != sectionEnd && rankOrderedRecords.at(sectionStart).getPropertyBool("reverse_ordered")) {
				std::reverse((rankOrderedRecords.begin() + sectionStart), (rankOrderedRecords.begin() + sectionEnd));
			} */

			mContent = ds::model::ContentModelRef("content");
			mPlatforms = ds::model::ContentModelRef("platform");
			mEvents = ds::model::ContentModelRef("all_events");
			mRecords = ds::model::ContentModelRef("all_records");
			mTags = ds::model::ContentModelRef("all_tags");

			for (const auto& record : rankOrderedRecords) {
				mRecords.addChild(record);
				auto type = record.getPropertyString("variant");
				if (type == "ROOT_CONTENT") {
					mContent.addChild(record);
				}
				else if (type == "ROOT_PLATFORM") {
					mPlatforms.addChild(record);
				}
				else if (type == "SCHEDULE") {
					for (const auto& parentUid : ds::split(record.getPropertyString("parent_uid"), ",")) {
						// Create/Add the event to its specific platform
						auto platformEvents = recordMap[parentUid].getChildByName("scheduled_events");
						platformEvents.setName("scheduled_events");
						platformEvents.addChild(record);
						recordMap[parentUid].replaceChild(platformEvents);
					}

					// Also add to the 'all_events' table in case an application needs events not specifically
					// assigned to it
					mEvents.addChild(record);
				}
				else if (type == "RECORD") {
					// it's has a record for a parent!
					recordMap[record.getPropertyString("parent_uid")].addChild(record);
				}
				else {
					DS_LOG_INFO("TYPE: " << type);
				}
			}
		}

		std::vector<ds::model::ContentModelRef> Tags;
		{
			ds::query::Result result;
			std::string		  tagQuery = "SELECT "
				" t.uid,"	   // 0
				" t.tag_class_uid," // 1
				" t.label,"		   // 2
				" l.app_key"		   // 3
				" FROM tags AS t"
				" LEFT JOIN lookup as l on l.uid = t.tag_class_uid;";

			if (ds::query::Client::query(cms.getDatabasePath(), tagQuery, result)) {
				ds::query::Result::RowIterator it(result);
				int							   tagId = 1;
				while (it.hasValue()) {
					auto tag = ds::model::ContentModelRef(it.getString(2) + "(" + it.getString(0) + ")");
					tag.setId(tagId);

					tag.setProperty("tag_uid", it.getString(0));
					tag.setProperty("tag_class_uid", it.getString(1));
					tag.setProperty("label", it.getString(2));
					tag.setProperty("tag_class_app_key", it.getString(3));

					// Put it in the map so we can wire everything up into the tree
					recordMap[it.getString(0)] = tag;

					Tags.push_back(tag);
					++it;
				}
			}
			else {
				DS_LOG_INFO("Tags query result is empty");
			}

			for (const auto& tag : Tags) {
				mTags.addChild(tag);
			}
		}

		{
			ds::query::Result result;
			std::string		  recordTagsQuery = "SELECT "
				" rt.tag_uid," // 0
				" rt.record_uid"	   // 1
				" FROM record_tags AS rt;";

			if (ds::query::Client::query(cms.getDatabasePath(), recordTagsQuery, result)) {
				ds::query::Result::RowIterator it(result);
				while (it.hasValue()) {
					// find the record ID and then add all tags associated to that record
					// auto tagInfo			   = Tags;
					auto label = recordMap[it.getString(0)].getPropertyString("label");
					auto tagUid = recordMap[it.getString(0)].getPropertyString("tag_uid");
					auto tagClassAppkey = recordMap[it.getString(0)].getPropertyString("tag_class_app_key");

					auto currentTags = recordMap[it.getString(1)].getPropertyString(tagClassAppkey + "_tags");
					recordMap[it.getString(1)].setProperty(tagClassAppkey + "_tags", currentTags + label + ", ");

					auto currentUids = recordMap[it.getString(1)].getPropertyString(tagClassAppkey + "_tag_uids");
					recordMap[it.getString(1)].setProperty(tagClassAppkey + "_tag_uids", currentUids + tagUid + ", ");
					++it;
				}
			}
			else {
				DS_LOG_INFO("record tags query result is empty");
			}
		}

		// get the select table
		{
			ds::query::Result result;
			std::string		  selectQuery = "SELECT "
				" lookup.uid," // 0
				" lookup.app_key"	   // 1
				" FROM lookup"
				" WHERE lookup.type = 'select'";
			if (ds::query::Client::query(cms.getDatabasePath(), selectQuery, result)) {
				ds::query::Result::RowIterator it(result);
				while (it.hasValue()) {
					selectMap[it.getString(0)] = it.getString(1);
					++it;
				}
			}
		}

		// Insert default values
		{
			ds::query::Result result;

			/* select defaults and the type they belong to (from traits) */
			std::string defaultsQuery =
				"SELECT "
				" record.uid,"					// 0
				" defaults.field_uid,"			// 1
				" lookup.app_key,"				// 2
				" defaults.field_type,"			// 3
				" defaults.checked,"			// 4
				" defaults.color,"				// 5
				" defaults.text_value,"			// 6
				" defaults.rich_text,"			// 7
				" defaults.number,"				// 8
				" defaults.number_min,"			// 9
				" defaults.number_max,"			// 10
				" defaults.option_type,"		// 11
				" defaults.option_value,"		// 12
				" lookup.app_key AS option_key" // 13
				" FROM record"
				" LEFT JOIN trait_map ON trait_map.type_uid = record.type_uid"
				" LEFT JOIN lookup ON record.type_uid = lookup.parent_uid OR trait_map.trait_uid = lookup.parent_uid "
				" LEFT JOIN defaults ON lookup.uid = defaults.field_uid"
				" WHERE EXISTS (select * from defaults where defaults.field_uid = lookup.uid);";

			if (ds::query::Client::query(cms.getDatabasePath(), defaultsQuery, result)) {
				ds::query::Result::RowIterator it(result);
				while (it.hasValue()) {
					auto recordUid = it.getString(0);
					if (recordMap.find(recordUid) == recordMap.end()) {
						++it;
						continue;
					}
					auto& record = recordMap[recordUid];

					auto field_uid = it.getString(1);
					auto field_key = it.getString(2);
					if (!field_key.empty()) {
						field_uid = field_key;
					}
					auto type = it.getString(3);

					if (type == "TEXT") {
						record.setProperty(field_uid, it.getString(6));
					}
					else if (type == "RICH_TEXT") {
						record.setProperty(field_uid, it.getString(7));
					}
					else if (type == "NUMBER") {
						record.setProperty(field_uid, it.getFloat(8));
					}
					else if (type == "OPTIONS") {
						auto key = it.getString(12);
						auto value = selectMap[key];
						record.setProperty(field_uid, value);
						record.setProperty(field_uid + "_value_uid", key);
					}
					else if (type == "CHECKBOX") {
						record.setProperty(field_uid, bool(it.getInt(4)));
					}
					else if (type == "COLOR") {
						record.setProperty(field_uid, it.getString(5));
					}
					else {
						DS_LOG_INFO("UNHANDLED(1): " << type)
					}
					record.setProperty(field_uid + "_field_uid", it.getString(1));
					++it;
				}
			}
		}

		{
			ds::query::Result result;

			std::string valueQuery = "SELECT "
				" v.uid,"					 // 0
				" v.field_uid,"			 // 1
				" v.record_uid,"			 // 2
				" v.field_type,"			 // 3
				" v.is_default,"			 // 4
				" v.is_empty,"				 // 5
				" v.checked,"				 // 6
				" v.color,"				 // 7
				" v.text_value,"			 // 8
				" v.rich_text,"			 // 9
				" v.number,"				 // 10
				" v.number_min,"			 // 11
				" v.number_max,"			 // 12
				" v.datetime,"				 // 13
				" v.resource_hash,"		 // 14
				" v.crop_x,"				 // 15
				" v.crop_y,"				 // 16
				" v.crop_w,"				 // 17
				" v.crop_h,"				 // 18
				" v.composite_frame,"		 // 19
				" v.composite_x,"			 // 20
				" v.composite_y,"			 // 21
				" v.composite_w,"			 // 22
				" v.composite_h,"			 // 23
				" v.option_type,"			 // 24
				" v.option_value,"			 // 25
				" v.link_url,"				 // 26
				" v.link_target_uid,"		 // 27
				" res.hash,"				 // 28
				" res.type,"				 // 29
				" res.uri,"				 // 30
				" res.width,"				 // 31
				" res.height,"				 // 32
				" res.duration,"			 // 33
				" res.pages,"				 // 34
				" res.file_size,"			 // 35
				" l.name AS field_name,"	 // 36
				" l.app_key AS field_key,"	 // 37
				" v.preview_resource_hash," // 38
				" preview_res.hash,"		 // 39
				" preview_res.type,"		 // 40
				" preview_res.uri,"		 // 41
				" preview_res.width,"		 // 42
				" preview_res.height,"		 // 43
				" preview_res.duration,"	 // 44
				" preview_res.pages,"		 // 45
				" preview_res.file_size,"	 // 46
				" v.hotspot_x,"			 // 47
				" v.hotspot_y,"			 // 48
				" v.hotspot_w,"			 // 49
				" v.hotspot_h,"			 // 50
				" res.filename"			 // 51
				" FROM value AS v"
				" LEFT JOIN lookup AS l ON l.uid = v.field_uid"
				" LEFT JOIN resource AS res ON res.hash = v.resource_hash"
				" LEFT JOIN resource AS preview_res ON preview_res.hash = v.preview_resource_hash;";


			if (ds::query::Client::query(cms.getDatabasePath(), valueQuery, result)) {
				ds::query::Result::RowIterator it(result);
				while (it.hasValue()) {
					const auto& recordUid = it.getString(2);
					if (recordMap.find(recordUid) == recordMap.end()) {
						++it;
						continue;
					}
					auto& record = recordMap[recordUid];

					auto field_uid = it.getString(1);
					auto field_key = it.getString(37);
					if (!field_key.empty()) {
						field_uid = field_key;
					}
					auto type = it.getString(3);

					if (type == "TEXT") {
						record.setProperty(field_uid, it.getString(8));
					}
					else if (type == "RICH_TEXT") {
						record.setProperty(field_uid, it.getString(9));
					}
					else if (type == "FILE_IMAGE" || type == "FILE_VIDEO" || type == "FILE_PDF") {
						if (!it.getString(28).empty()) {
							auto res = ds::Resource(mResourceId, ds::Resource::Id::CMS_TYPE, double(it.getFloat(33)),
								float(it.getInt(31)), float(it.getInt(32)), it.getString(51),
								it.getString(30), -1, cms.getResourcePath() + it.getString(30));
							if (type == "FILE_IMAGE")
								res.setType(ds::Resource::IMAGE_TYPE);
							else if (type == "FILE_VIDEO")
								res.setType(ds::Resource::VIDEO_TYPE);
							else if (type == "FILE_PDF")
								res.setType(ds::Resource::PDF_TYPE);
							else
								continue;

							auto cropX = it.getFloat(15);
							auto cropY = it.getFloat(16);
							auto cropW = it.getFloat(17);
							auto cropH = it.getFloat(18);
							if (cropX > 0.f || cropY > 0.f || cropW > 0.f || cropH > 0.f) {
								res.setCrop(cropX, cropY, cropW, cropH);
							}

							record.setPropertyResource(field_uid, res);
						}

						if (!it.getString(38).empty()) {
							const auto& previewType = it.getString(40);

							auto res = ds::Resource(mResourceId, ds::Resource::Id::CMS_TYPE, double(it.getFloat(44)),
								float(it.getInt(42)), float(it.getInt(43)), it.getString(41),
								it.getString(41), -1, cms.getResourcePath() + it.getString(41));
							res.setType(previewType == "FILE_IMAGE" ? ds::Resource::IMAGE_TYPE : ds::Resource::VIDEO_TYPE);

							auto preview_uid = field_uid + "_preview";
							record.setPropertyResource(preview_uid, res);
						}


					}
					else if (type == "LINKS") {
						record.setProperty(field_uid, it.getString(27));
					}
					else if (type == "LINK_WEB") {
						const auto& linkUrl = it.getString(30);
						auto		res = ds::Resource(mResourceId, ds::Resource::Id::CMS_TYPE, double(it.getFloat(33)),
							float(it.getInt(31)), float(it.getInt(32)), linkUrl, linkUrl, -1, linkUrl);

						// Assumes app settings are not changed concurrently on another thread.
						auto webSize = mEngine.getAppSettings().getVec2("web:default_size", 0, ci::vec2(1920.f, 1080.f));
						res.setWidth(webSize.x);
						res.setHeight(webSize.y);
						res.setType(ds::Resource::WEB_TYPE);
						record.setPropertyResource(field_uid, res);


						if (!it.getString(38).empty()) {

							const auto& previewType = it.getString(40);
							auto		res = ds::Resource(mResourceId, ds::Resource::Id::CMS_TYPE, double(it.getFloat(44)),
								float(it.getInt(42)), float(it.getInt(43)), it.getString(41),
								it.getString(41), -1, cms.getResourcePath() + it.getString(41));
							res.setType(type == "FILE_IMAGE" ? ds::Resource::IMAGE_TYPE : ds::Resource::VIDEO_TYPE);

							auto preview_uid = field_uid + "_preview";
							record.setPropertyResource(preview_uid, res);
						}
					}
					else if (type == "NUMBER") {
						record.setProperty(field_uid, it.getFloat(10));
					}
					else if (type == "COMPOSITE_AREA") {
						const auto& frameUid = it.getString(19);
						record.setProperty(frameUid + "_x", it.getFloat(20));
						record.setProperty(frameUid + "_y", it.getFloat(21));
						record.setProperty(frameUid + "_w", it.getFloat(22));
						record.setProperty(frameUid + "_h", it.getFloat(23));
					}
					else if (type == "IMAGE_AREA" || type == "IMAGE_SPOT") {
						record.setProperty("hotspot_x", it.getFloat(47));
						record.setProperty("hotspot_y", it.getFloat(48));
						record.setProperty("hotspot_w", it.getFloat(49));
						record.setProperty("hotspot_h", it.getFloat(50));
					}
					else if (type == "OPTIONS") {
						// this should be the following
						auto key = it.getString(25);
						auto value = selectMap[key];
						record.setProperty(field_uid, value);
						record.setProperty(field_uid + "_value_uid", key);
					}
					else if (type == "CHECKBOX") {
						record.setProperty(field_uid, bool(it.getInt(6)));
					}
					else if (type == "COLOR") {
						record.setProperty(field_uid, it.getString(7));
					}
					else {
						DS_LOG_INFO("UNHANDLED(2): " << type)
					}
					record.setProperty(field_uid + "_field_uid", it.getString(1));

					++it;
				}
			}
		}
	}

	void BridgeService::Loop::updatePlatformEvents() const {
		// DS_LOG_VERBOSE(2, "BridgeService::Loop is updating platform events.")

		Poco::LocalDateTime ldt = Poco::LocalDateTime();
		Poco::DateTime		thisDayTime;
		thisDayTime.makeLocal(ldt.tzd());

		// Obtain currently active platform.
		//auto platform = ds::model::Platform::getRecordByUid(mRecords, platform_key);
		//auto platformEvents = platform.getChildByName("scheduled_events").getChildren();
		
		for (auto platform : mPlatforms.getChildren()) {
			auto platformEvents = platform.getChildByName("scheduled_events").getChildren();
			if (!platformEvents.empty()) {
				std::vector<ds::model::ContentModelRef> currentEvents;
				for (auto& event : platformEvents) {
					if (eventIsNow(event, thisDayTime)) currentEvents.push_back(event);
				}

				//	// TODO: handle correct sorting/combining of events
				//	// Sort playlists by importance.
				std::sort(std::begin(currentEvents), std::end(currentEvents), [](auto& a, auto& b) {
					//		// Prioritize scheduled content over recurring content.
					if (a.getPropertyString("span_type") != "RECURRING" &&
						b.getPropertyString("span_type") == "RECURRING")
						return true;

					//		// Prioritize late start times over early start times.
					if (a.getPropertyString("start_time") != b.getPropertyString("start_time"))
						return a.getPropertyString("start_time") > b.getPropertyString("start_time");

					//		// Prioritize early end times over late end times.
					return a.getPropertyString("end_time") < b.getPropertyString("end_time");
				});

				//	// For interoperability, store current events.
				auto platformCurrentEvents = platform.getChildByName("current_events");
				if (platformCurrentEvents.empty() || platformCurrentEvents.getChildren() != currentEvents) {
					platformCurrentEvents.setName("current_events");
					platformCurrentEvents.setChildren(currentEvents);

					platform.replaceChild(platformCurrentEvents);
				}
			}

			mEngine.getNotifier().notify(ds::PlatformEventsUpdatedEvent());

			// Use helper to obtain the appropriate playlist.
			auto updatedPlaylist = ds::model::ContentModelRef();

			// auto currentPlatform = mEngine.getAppSettings().getString("app:platform", 0, "none");
			// updatedPlaylist = ds::getPlatformPlaylist(mEngine);

			//// Parse content to add custom properties.
			//// TODO: perhaps move this code to a more logical place?
			////for (auto content : updatedPlaylist.getChildren())
			////	TransitionController::parseContent(mEngine, content);

			//// Compare with current playlist.
			// if (!updatedPlaylist.empty()) {

			//	auto currentPlaylist = platform.getChildByName("current_playlist");

			//	//// Check if actually different.
			//	// const auto& current = currentPlaylist.getChildren();
			//	// const auto& updated = updatedPlaylist.getChildren();

			//	// bool isDifferent = updated.size() != current.size();
			//	// for (size_t i = 0; i < updated.size() && !isDifferent; ++i) {
			//	//	isDifferent |= current.at(i) != updated.at(i);
			//	// }

			//	//// Update if different.
			//	// if (isDifferent) {
			//	currentPlaylist.setName("current_playlist");
			//	currentPlaylist.setChildren(updatedPlaylist.getChildren());
			//	platform.replaceChild(currentPlaylist);
			//	//}
			//}
			// else {
			//	// If we arrive here, chances are the current playlist has been disabled.
			//	// Just make sure any previously running content is deactivated.
			//	ds::model::ContentModelRef dummy;
			//	dummy.setName("current_playlist");
			//	platform.replaceChild(dummy);
			//}
		}
	}

}