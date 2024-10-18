#include "stdafx.h"

#include "template_layer.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>

//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/template/template_base.h"
#include "waffles/template/template_config.h"



namespace {
static waffles::TemplateLayer* theTemplateLayer = nullptr;
}

namespace waffles {

TemplateLayer::TemplateLayer(ds::ui::SpriteEngine& eng, ci::vec2 size, ci::vec2 pos, std::string channel_name)
	: ds::ui::SmartLayout(eng, "waffles/layer/template_layer.xml") {

	if (!channel_name.empty()) {
		mEventClient.setNotifier(mEngine.getChannel(channel_name));
		setChannelName(channel_name);
	}

	mTemplateConfig = TemplateConfig::getDefault();
	theTemplateLayer = this;
	
	setSize(size);
	setPosition(pos);
	runLayout();

	// Handle background change requests
	mEventClient.listenToEvents<waffles::ChangeTemplateRequest>(
		[this](const waffles::ChangeTemplateRequest& event) { 
			changeTemplate(event); 
		});
}

TemplateLayer* TemplateLayer::get() {
	return theTemplateLayer;
}

void TemplateLayer::changeTemplate(const waffles::ChangeTemplateRequest& event) {
	auto doChange = [this, event] {
		auto bg = getSprite("template_holder");
		if (!bg) {
			DS_LOG_ERROR("TemplateLayer is missing template_holder sprite!");
			return;
		}

		// Save Current Background (will be null if there was no current)
		auto oldTemplate = mCurrentTemplate;
		mCurrentTemplate = nullptr;


		if (!event.mContent.empty()) {
			auto model		 = event.mContent.duplicate();
			mCurrentTemplate = mTemplateConfig->createTemplate(mEngine, model, getChannelName());
		} else {
			// Create empty background type
			mCurrentTemplate = nullptr;
		}


		if (oldTemplate) {
			float delay = 0.f;
			if (mCurrentTemplate) {
				bool needsClear =
					mTemplateConfig->getTemplateDefFromId(mCurrentTemplate->getContentModel().getPropertyString("type_uid"))
						.requiresClear ||
					mTemplateConfig->getTemplateDefFromId(oldTemplate->getContentModel().getPropertyString("type_uid")).requiresClear;

				if (needsClear) {
					delay = oldTemplate->animateOff(0.f, [oldTemplate] { oldTemplate->release(); });
					bg->addChildPtr(mCurrentTemplate);
					mCurrentTemplate->sendToBack();
					runLayout();
					mCurrentTemplate->callAfterDelay(
						[this] {
							if (mCurrentTemplate->getContentModel().getPropertyString("type_key") != "pinboard_event" &&
								mCurrentTemplate->getContentModel().getPropertyString("type_key") != "assets_mode") {
								//mEventClient.notify(waffles::RequestCloseAllEvent(true));
							}
							mCurrentTemplate->animateOn(0.f, [this] {
								// Notify that the background change has completed (after animation!)
								mEventClient.notify(waffles::TemplateChangeComplete());
							});
							mEventClient.notify(
								waffles::TemplateChangeStarted(mCurrentTemplate->getContentModel()));
						},
						delay);
				} else {
					bg->addChildPtr(mCurrentTemplate);
					mCurrentTemplate->sendToBack();
					runLayout();
					delay = mCurrentTemplate->animateOn(0.f, [this] {
						// Notify that the background change has completed (after animation!)
						mEventClient.notify(waffles::TemplateChangeComplete());
					});
					mEventClient.notify(waffles::TemplateChangeStarted(mCurrentTemplate->getContentModel()));

					oldTemplate->animateOff(delay, [oldTemplate] { oldTemplate->release(); });
				}
			} else {
				mEventClient.notify(waffles::TemplateChangeComplete());
				oldTemplate->animateOff(0.f, [oldTemplate] { oldTemplate->release(); });
			}

			
		} else {
			if (mCurrentTemplate) {
				bg->addChildPtr(mCurrentTemplate);
				mCurrentTemplate->sendToBack();
				runLayout();
				mCurrentTemplate->animateOn(0.f, [this] {
					// Notify that the background change has completed (after animation!)
					mEventClient.notify(waffles::TemplateChangeComplete());
				});
				mEventClient.notify(waffles::TemplateChangeStarted(mCurrentTemplate->getContentModel()));
			} else {
				mEventClient.notify(waffles::TemplateChangeComplete());
			}
		}
	};
	
	doChange();
}

} // namespace waffles