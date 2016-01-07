#include "layout_loader.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

#include <ds/ui/interface_xml/interface_xml_importer.h>

namespace layout_builder {

LayoutLoader::LayoutLoader(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
{
	
	setPosition(200.0f, 200.0f);
}

void LayoutLoader::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RefreshLayoutRequest::WHAT()){
		loadLayout(mLayoutLocation);
	} else if(in_e.mWhat == LayoutLayoutRequest::WHAT()){
		layout();
	} else if(in_e.mWhat == LoadLayoutRequest::WHAT()){
		const LoadLayoutRequest& e((const LoadLayoutRequest&)in_e);
		loadLayout(e.mLocation);
	} else if(in_e.mWhat == AnimateLayoutRequest::WHAT()){
		animateOn();
	}
	
}

void LayoutLoader::loadLayout(const std::string& layoutLocation) {
	if(mLayout){
		mLayout->release();
		mLayout = nullptr;
	}

	mLayoutLocation = layoutLocation;
	if(mLayoutLocation.empty()) return;

	mLayout = new ds::ui::LayoutSprite(mEngine);
	mLayout->setSpriteName(ds::wstr_from_utf8(layoutLocation));
	addChildPtr(mLayout);

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(mLayout, ds::Environment::expand(mLayoutLocation), spriteMap);


	mEngine.getNotifier().notify(InspectTreeRequest(mLayout));

	layout();
	animateOn();
}

void LayoutLoader::layout(){
	if(mLayout){
		mLayout->runLayout();
	}
}


void LayoutLoader::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void LayoutLoader::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder
