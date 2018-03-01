#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
	, mPrimaryLayout(nullptr)
	, mIsEngineCheckbox(nullptr)
	, mIncludeComments(nullptr)
	, mImage(nullptr)
{
	hide();
	setOpacity(0.0f);


	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/story_view.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	mMessage = dynamic_cast<ds::ui::Text*>(spriteMap["message"]);
	mImage = dynamic_cast<ds::ui::Image*>(spriteMap["primary_image"]);
	mIsEngineCheckbox = dynamic_cast<ds::ui::ControlCheckBox*>(spriteMap["engine_check_box"]);
	mIncludeComments = dynamic_cast<ds::ui::ControlCheckBox*>(spriteMap["comments_check_box"]);

	if(mIsEngineCheckbox){
		mIsEngineCheckbox->setCheckBoxValue(true);
	}

	if(mIncludeComments) {
		mIncludeComments->setCheckBoxValue(true);
	}

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
	if(in_e.mWhat == StoryDataUpdatedEvent::WHAT()){
		setData();
	}

	if(in_e.mWhat == ds::cfg::Settings::SettingsEditedEvent::WHAT()){
		const ds::cfg::Settings::SettingsEditedEvent& e((const ds::cfg::Settings::SettingsEditedEvent&)in_e);
		if(e.mSettingsType == "app_settings" && e.mSettingName == "something"){
			for(int i = 0; i < 8; i++){
				std::cout << "New setting index=" << i << " value=" << mEngine.getAppSettings().getInt("something", i) << std::endl;
			}
		}
	}
}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data
	if(!mGlobals.mAllData.mStories.empty()){

		auto storyRef = mGlobals.mAllData.mStories.front();

		

		if(mImage && storyRef.getPrimaryResource().getType() == ds::Resource::IMAGE_TYPE){
			mImage->setImageResource(storyRef.getPrimaryResource());
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
	tweenOpacity(1.0f, mEngine.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void StoryView::onUpdateServer(const ds::UpdateParams& p){
	// any changes for this frame happen here
}

bool StoryView::getIsEngineMode(){
	if(mIsEngineCheckbox){
		return mIsEngineCheckbox->getCheckBoxValue();
	}

	return false;
}

bool StoryView::getIncludeComments() {
	if(mIncludeComments) {
		return mIncludeComments->getCheckBoxValue();
	}

	return false;
}

} // namespace downstream

