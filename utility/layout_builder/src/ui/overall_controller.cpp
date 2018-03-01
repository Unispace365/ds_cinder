#include "overall_controller.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace layout_builder {

OverallController::OverallController(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mController(nullptr)
{

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	setColor(ci::Color(0.5f, 0.5f, 0.5f));
	setTransparent(false);

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/layout_builder/controller.xml"), spriteMap);
	mController = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["layout"]);
	if(mController){
		mController->runLayout();
		mController->tweenAnimateOn(true, 0.3f, 0.1f);
	}

	auto refreshButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["refresh_button"]);
	if(refreshButton){
		refreshButton->setClickFn([this]{
			mEngine.getNotifier().notify(RefreshLayoutRequest());
		});
	}

	auto restartButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["restart_button"]);
	if(restartButton){
		restartButton->setClickFn([this, restartButton]{
			mEngine.getNotifier().notify(AppRestartRequest());
		});
	}

	auto animateOn = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["animate_layout"]);
	if(animateOn){
		animateOn->setClickFn([this]{
			mEngine.getNotifier().notify(AnimateLayoutRequest());
		});
	}


	auto saveLayout = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["save_layout"]);
	if(saveLayout){
		saveLayout->setClickFn([this]{
			mEngine.getNotifier().notify(SaveLayoutRequest());
		});
	}

	layout();
}

void OverallController::onAppEvent(const ds::Event& in_e){
// 	if(in_e.mWhat == RefreshLayoutRequest::WHAT()){
// 		loadLayout(mLayoutLocation);
// 	} else if(in_e.mWhat == LayoutLayoutRequest::WHAT()){
// 		layout();
// 	} else if(in_e.mWhat == LoadLayoutRequest::WHAT()){
// 		const LoadLayoutRequest& e((const LoadLayoutRequest&)in_e);
// 		loadLayout(e.mLocation);
// 	} else if(in_e.mWhat == AnimateLayoutRequest::WHAT()){
// 		animateOn();
// 	}

}

void OverallController::layout(){
	if(mController){
		mController->runLayout();
		setSize(mController->getWidth(), mController->getHeight() + 20.0f);
	}
}


void OverallController::animateOn(){
	if(mController){
		mController->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void OverallController::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder
