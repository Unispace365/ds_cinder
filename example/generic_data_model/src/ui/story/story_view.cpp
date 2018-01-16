#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

#include "query/data_wrangler.h"
#include "model/data_model.h"

#include "model/data_model.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
	, mPrimaryLayout(nullptr)
	, mImage(nullptr)
{
	hide();
	setOpacity(0.0f);


	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/story_view.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	mMessage = dynamic_cast<ds::ui::Text*>(spriteMap["message"]);
	mImage = dynamic_cast<ds::ui::Image*>(spriteMap["primary_image"]);

	// calls layout
	setData();
	animateOn();


}

void StoryView::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == IdleEndedEvent::WHAT()){
		const IdleEndedEvent& e((const IdleEndedEvent&)in_e);
		animateOn();
	} else if(in_e.mWhat == IdleStartedEvent::WHAT()){
		animateOff();
	}

	// If you have an event that is dispatched when new content is queryied, you could map that here.
	if(in_e.mWhat == DataUpdatedEvent::WHAT()){
		setData();
	}
}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data

	if(mGlobals.mDataWrangler.mData.hasChild("leadership")) {
		auto leadershipTable = mGlobals.mDataWrangler.mData.getChild("leadership");
		auto leadershipItems = leadershipTable.getChildren("rows");

		if(!leadershipItems.empty()) {
			auto leadershipItem = leadershipItems[rand() % leadershipItems.size()];

			if(mMessage) {
				mMessage->setText(leadershipItem.getPropertyValue("firstname"));
			}
			if(mImage) {
				mImage->setImageResource(leadershipItem.getProperty("resourceid").getResource());
			}
		}
	}

	layout();
}

void StoryView::layout(){
	if(mPrimaryLayout){
		mPrimaryLayout->runLayout();
	}
}

void StoryView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void StoryView::onUpdateServer(const ds::UpdateParams& p){
	// any changes for this frame happen here
}



} // namespace downstream

