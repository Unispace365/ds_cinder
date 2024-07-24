#include "stdafx.h"

#include "engage_controller.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>
#include <ds/content/platform.h>

#include "app/app_defs.h"
//#include "app/cms_definitions.hpp"
//#include "app/helpers.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/template/template_config.h"
#include "waffles/background/background_view.h"

//using namespace downstream;


namespace waffles {

EngageController::EngageController(ds::ui::SpriteEngine& eng)
	: ds::ui::Sprite(eng)
	, mEventClient(eng) {

	DS_LOG_INFO("Creating an Engage Controller");
	auto helper = WafflesHelperFactory::getDefault();
	mEventClient.listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) {
		if (mEngine.getAppSettings().getBool("app:editor_mode", 0, false)) {
			// In "editor mode", refresh the currentl slide whenever it changes
			auto currPres		   = mEngine.mContent.getChildByName("current_presentation");
			auto currSlide		   = currPres.getPropertyInt("current_slide");
			auto currentSlideModel = currPres.getChild(0).getChild(currSlide - 1);
			if (!currPres.empty() && !currentSlideModel.empty()) {
				setData();
				if (currentSlideModel != mPlaylist.getChild(currSlide - 1)) {
					gotoItem(currSlide - 1);
				}
			} else {
				setData();
			}
		} else {
			// Otherwise get the new data but don't refresh :)
			// New content will be visisble when leaving and returning to the slide
			setData();
		}
	});

	mEventClient.listenToEvents<waffles::RequestPresentationEndEvent>([this](const auto& ev) {
		auto currPres = mEngine.mContent.getChildByName("current_presentation");
		currPres.clearChildren();
		mEngine.mContent.replaceChild(currPres);
	});
	mEventClient.listenToEvents<ds::app::IdleStartedEvent>([this](const auto& ev) { endEngage(); });
	mEventClient.listenToEvents<ds::app::IdleEndedEvent>([this](const auto& ev) {
		setDefaultPresentation();
		startEngage();
	});

	mEventClient.listenToEvents<waffles::RequestEngagePresentation>([this,helper](const waffles::RequestEngagePresentation& ev) {
		DS_LOG_INFO("got a request engage presentation event in engage controller");
		auto content = ev.mContent;
		if (content.getPropertyString("type_key") == "pinboard_event") {
			if (mPlaylist.getPropertyString("type_key") != "pinboard_event") {

				cancelDelayedCall();
				auto playlist = ds::model::ContentModelRef("Pinboard");
				playlist.setProperty("type_key", getTemplateDefFromName("pinboard_event").name);
				playlist.setProperty("record_name", content.getPropertyString("record_name"));
				auto model = ds::model::ContentModelRef("Pinboard");
				model.setProperty("type_uid", getTemplateDefFromName("pinboard_event").id);
				model.setProperty("record_name", content.getPropertyString("record_name"));
				playlist.addChild(model);
				setPresentation(playlist);
				gotoItem(0);
			}
			return;

		} else if (content.getName() == "assets") {
			if (mPlaylist.getPropertyString("type_key") != "assets_mode") {
				cancelDelayedCall();
				auto playlist = ds::model::ContentModelRef("Assets");
				playlist.setProperty("type_key", getTemplateDefFromName("assets_mode").name);
				playlist.setProperty("record_name", std::string("Asset Mode"));
				auto model = ds::model::ContentModelRef("Assets");
				model.setProperty("type_uid", getTemplateDefFromName("assets_mode").id);
				playlist.addChild(model);
				setPresentation(playlist);
				gotoItem(0);
			}
			return;
		}

		cancelDelayedCall();
		bool newPres  = false;
		bool newSlide = false;
		if (content.getPropertyString("type_key") != "interactive_playlist" &&
			content.getPropertyString("type_key") != "ambient_playlist") {
			// This is a slide
			// Might be a new pres, might not be

			auto theSlide = content;
			content		  = helper->getRecordByUid(theSlide.getPropertyString("parent_uid"));
			auto uid	  = content.getPropertyString("uid");
			if (uid != mPlaylistUid) {
				// New pres
				mPlaylistUid = uid;
				newPres		 = true;
			}
			for (auto slide : content.getChildren()) {
				if (slide.getPropertyString("uid") == theSlide.getPropertyString("uid")) {
					auto slideUid = slide.getPropertyString("uid");
					if (slideUid != mSlideUid) {
						mSlideUid = slideUid;
						newSlide  = true;
					}
					break;
				}
			}
		} else {
			// This is a presentation, might be new, might be same
			auto uid = content.getPropertyString("uid");
			if (uid != mPlaylistUid) {
				mPlaylistUid = uid;
				newPres		 = true;
			}
		}

		if (newPres) {
			setPresentation(helper->getRecordByUid(mPlaylistUid));
		}

		if (newSlide) {
			setSlide(helper->getRecordByUid(mSlideUid));
		} else {
			setData();
			if (!mPlaylist.getChildren().empty()) setSlide(mPlaylist.getChildren().front());
		}

		if (!mEngine.isIdling() && mAmEngaged) {
			gotoItem(mPlaylistIdx);
		}
	});
	mEventClient.listenToEvents<waffles::RequestEngageNext>([this](const auto& ev) {
		if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
			mPlaylist.getPropertyString("type_key") == "assets_mode") {
			return;
		}

		nextItem();
	});
	mEventClient.listenToEvents<waffles::RequestEngageBack>([this](const auto& ev) {
		if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
			mPlaylist.getPropertyString("type_key") == "assets_mode") {
			return;
		}
		prevItem();
	});

	mEventClient.listenToEvents<waffles::RequestPresentationAdvanceEvent>(
		[this](const waffles::RequestPresentationAdvanceEvent& ev) {
			bool isFwd = (!ev.mUserStringData.empty() && ev.mUserStringData == "false") ? false : true;

			if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
				mPlaylist.getPropertyString("type_key") == "assets_mode") {
				return;
			}

			// setData();
			if (isFwd) {
				nextItem();
			} else {
				prevItem();
			}
		});


	setDefaultPresentation();
}

void EngageController::setData() {
	ds::model::ContentModelRef thePlaylist;
	auto helper = WafflesHelperFactory::getDefault();
	if (!mPlaylistUid.empty()) {
		thePlaylist = helper->getRecordByUid(mPlaylistUid);
	} else {
		thePlaylist = ds::model::ContentModelRef("Empty Playlist");
		thePlaylist.setProperty("type_key", getTemplateDefFromName("empty").name);
		auto model = ds::model::ContentModelRef("Empty");
		model.setProperty("type_uid", getTemplateDefFromName("empty").id);
		thePlaylist.addChild(model);
		mPlaylistUid.clear();
	}

	if (thePlaylist != mPlaylist) {
		mPlaylist = thePlaylist;
	}
}

void EngageController::setDefaultPresentation() {
	auto helper = WafflesHelperFactory::getDefault();
	ds::model::Platform platformObj(mEngine);
	if (platformObj.getPlatformType() == ds::model::Platform::UNDEFINED) return;
	auto thePlaylist = helper->getPresentation();
	if (!thePlaylist.empty() && !thePlaylist.getChildren().empty()) {
		mPlaylist	  = thePlaylist;
		mPlaylistUid  = thePlaylist.getPropertyString("uid");
		auto theSlide = thePlaylist.getChildren().front();
		mSlideUid	  = theSlide.getPropertyString("uid");
	} else {
		mPlaylistUid.clear();
	}
}

void EngageController::setPresentation(ds::model::ContentModelRef thePlaylist) {
	mPlaylist = thePlaylist;
}

bool EngageController::setSlide(ds::model::ContentModelRef theSlide) {
	int idx = 0;
	for (auto slide : mPlaylist.getChildren()) {
		if (slide.getPropertyString("uid") == mSlideUid) {
			if (slide == mSlide) {
				// Already here, no need to change
				mPlaylistIdx = idx;
				return false;
			} else {
				mPlaylistIdx = idx;
				return true;
			}
			// break;
		}
		idx++;
	}

	return false;
}

void EngageController::startEngage() {
	mAmEngaged	 = true;
	mPlaylistIdx = 0;

	// Notify listeners.
	mEngine.getNotifier().notify(waffles::EngageStarted());

	if (!mPlaylistUid.empty()) {
		// Always start on the first playlist item when starting the engage
		cancelDelayedCall();


		// Wait for background to finish transitioning to empty, then trigger the template
		mEventClient.listenToEvents<waffles::BackgroundChangeComplete>([this](const auto& ev) {
			if (mAmEngaged) {
				// Only trigger the template if we're still in engage mode
				gotoItem(mPlaylistIdx);
			}

			// Stop listening
			mEventClient.stopListeningToEvents<waffles::BackgroundChangeComplete>();
		});
	} else {
		mEngine.getNotifier().notify(waffles::ChangeTemplateRequest());

		callAfterDelay(
			[this] {
				setDefaultPresentation();
				setData();
				startEngage();
			},
			0.25f);
	}
}

void EngageController::nextItem() {
	gotoItem(mPlaylistIdx + 1);
}

void EngageController::prevItem() {
	gotoItem(mPlaylistIdx - 1);
}


void EngageController::gotoItem(int index) {

	int childCount = mPlaylist.getChildren().size();
	if (childCount == 0) return;

	if (index >= childCount) {
		index = 0;
	} else if (index < 0) {
		index = childCount - 1;
	}

	mPlaylistIdx  = index;
	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.setName("current_presentation");
	currPres.setChildren({mPlaylist});
	currPres.setProperty("current_slide", mPlaylistIdx + 1);
	mEngine.mContent.replaceChild(currPres);

	mSlide	  = mPlaylist.getChildren()[mPlaylistIdx];
	mSlideUid = mSlide.getPropertyString("uid");

	// mEngine.getNotifier().notify(waffles::RequestCloseAllEvent(true));

	if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
		mPlaylist.getPropertyString("type_key") == "assets_mode") {
		mPlaylistUid.clear();
	}

	mEngine.getNotifier().notify(waffles::PresentationStateChanged(mPlaylist));
	mEngine.getNotifier().notify(waffles::ChangeTemplateRequest(mSlide));

	if (mPlaylist.getPropertyString("type_key") == "empty") {
		callAfterDelay(
			[this] {
				setDefaultPresentation();
				setData();
			},
			0.25f);
	}
}

void EngageController::endEngage() {
	cancelDelayedCall();

	mAmEngaged	 = false;
	mPlaylistIdx = -1;
	// Revert to no template
	mEngine.getNotifier().notify(waffles::RequestCloseAllEvent(true));
	mEngine.getNotifier().notify(waffles::ChangeTemplateRequest());

	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.clearChildren();
	mEngine.mContent.replaceChild(currPres);

	// Notify listeners.
	mEngine.getNotifier().notify(waffles::EngageEnded());
}

} // namespace waffles
