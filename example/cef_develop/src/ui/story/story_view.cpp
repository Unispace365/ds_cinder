#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace cef {

StoryView::StoryView(Globals& g)
  : ds::ui::Sprite(g.mEngine)
  , mGlobals(g)
  , mEventClient(g.mEngine.getNotifier(),
				 [this](const ds::Event* m) {
					 if (m) this->onAppEvent(*m);
				 })
  , mMessage(nullptr) {
	hide();
	setOpacity(0.0f);

	mMessage = mGlobals.getText("sample:config").create(mEngine, this);
	if (mMessage) {
		mMessage->setText("Hello, whirled!");
	}

	// calls layout
	setData();
	animateOn();
}

void StoryView::onAppEvent(const ds::Event& in_e) {
	if (in_e.mWhat == IdleEndedEvent::WHAT()) {
		const IdleEndedEvent& e((const IdleEndedEvent&)in_e);
		animateOn();
	} else if (in_e.mWhat == IdleStartedEvent::WHAT()) {
		animateOff();
	}

	// If you have an event that is dispatched when new content is queryied, you could map that here.
	// if(in_e.mWhat == StoryContentUpdated::WHAT()){
	//	setData();
	//}
}

void StoryView::setData() {
	// update view to match new content
	if (mMessage) {
		// Map the content from the app to the view sprites
		// mMessage->setText(mGlobals.mAllStories.mStories.front().getName());
	}

	layout();
}

void StoryView::layout() {
	if (mMessage) {
		mMessage->setPosition(100.0f, 200.0f);
	}
}


void StoryView::animateOn() {
	show();
	tweenOpacity(1.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f));
}

void StoryView::animateOff() {
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(),
				 [this] { hide(); });
}

} // namespace cef
