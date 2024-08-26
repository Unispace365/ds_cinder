#include "stdafx.h"

#include "template_config.h"

#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/sprite/gst_video.h>
/*
#include "bubbles_template.h"
#include "card_template.h"
#include "carousel_cards.h"
#include "feature_story.h"
#include "media_gallery.h"
#include "pinboard_template.h"*/
#include "template_base.h"
#include "composite_slide.h"

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {

	const TemplateDef& TemplateConfig::getTemplateDefFromName(const std::string& name) {
	auto findy = std::find_if(mTemplateDefinitions.begin(), mTemplateDefinitions.end(),
							  [name](const auto& a) { return name == a.name; });

	if (findy != mTemplateDefinitions.end()) {
		return *findy;
	} else {
		DS_LOG_ERROR("Failed to load template named " << name << "! Showing Invalid template");
		return mTemplateDefinitions[0]; // Fallback to 'invalid' template
	}
}

const TemplateDef& TemplateConfig::getTemplateDefFromId(const std::string& id) {
	auto findy = std::find_if(mTemplateDefinitions.begin(), mTemplateDefinitions.end(),
							  [id](const auto& a) { return id == a.id; });

	if (findy != mTemplateDefinitions.end()) {
		return *findy;
	} else {
		DS_LOG_ERROR("Failed to load template with ID " << id << "! Showing Invalid template");
		return mTemplateDefinitions[0]; // Fallback to 'invalid' template
	}
}

TemplateBase* TemplateConfig::createTemplate(ds::ui::SpriteEngine& engine, ds::model::ContentModelRef model) {
	auto def = getTemplateDefFromId(model.getPropertyString("type_uid"));

	DS_LOG_INFO("\n\n\nDEF " << def.name << "\n\n\n");

	// Add special case templates here
	
	if (def.name == "composite_slide") {
		return new CompositeSlide(engine, def, model);
		
	}/* else if (def.name == "feature_story") {
		return new FeatureStory(engine, def, model);

	} else if (def.name == "cards") {
		return new CardTemplate(engine, def, model);

	} else if (def.name == "carousel_cards") {
		return new CarouselCards(engine, def, model);

	} else if (def.name == "media_gallery") {
		return new MediaGallery(engine, def, model);

	} else if (def.name == "bubbles") {
		return new BubblesTemplate(engine, def, model);

	} else if (def.name == "pinboard_event" || def.name == "pinboard") {
		return new PinboardTemplate(engine, def, model);

	} */ else if (def.name == "media") {
		//get the resource name from the extra params if there are any
		auto resourceName = def.extra.size() > 0 ? def.extra[0] : "media";
		auto res = model.getPropertyResource(resourceName);
		if(resourceName!="media") model.setPropertyResource("media", res);
		auto mediaTemplate = new TemplateBase(engine, def, model);
		/*
		mediaTemplate->setAnimStartCb([mediaTemplate, resourceName, model, &engine] {
			mediaTemplate->callAfterDelay(
				[mediaTemplate, model,resourceName, &engine] {
					auto arrg		 = model.duplicate();
					
					auto displayType = std::string("fit");
					//auto displayType = arrg.getPropertyString("display_mode") == "pioTM16AbaGQ" ? std::string("fit") // TODO: not straight UUID
					//																			: std::string("fill");
					auto res = arrg.getPropertyResource(resourceName);
					auto targetOffset =
						engine.getWafflesSettings().getVec2("template:media:waffles_offset", 0, ci::vec2(0.f, 2160.f));
					auto targetSize =
						engine.getWafflesSettings().getVec2("template:media:waffles_size", 0, ci::vec2(2560.f, 1080.f));
					float startWidth   = targetSize.x;
					float targetAspect = targetSize.x / targetSize.y;
					auto  aspect =
						res.getWidth() / res.getHeight();
					if (displayType == "fit") {
						if (aspect > targetAspect) {
							startWidth = targetSize.x;
						} else {
							auto scale = targetSize.y / res.getHeight();
							startWidth = res.getWidth() * scale;
						}
						// Ensure height fits, otherwise startWidth = full width
					} else {
						if (aspect > targetAspect) {
							auto scale = targetSize.y / res.getHeight();
							startWidth = res.getWidth() * scale;
						} else {
							startWidth = targetSize.x;
						}
					}
					auto args = waffles::ViewerCreationArgs(model, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER,
						ci::vec3((targetSize.x / 2.f) + targetOffset.x, (targetSize.y / 2.f) + targetOffset.y, 0.f),
						waffles::ViewerCreationArgs::kViewLayerBackground, startWidth, true, false);


					args.mFromCenter			   = true;
					args.mShowFullscreenController = false;
					args.mAutoStart				   = arrg.getPropertyBool("autoplay") || engine.isIdling();
					args.mCheckBounds			   = true;
					args.mStartLocked			   = true;
					args.mTouchEvents			   = false;
					if (engine.isIdling()) {
						args.mLooped = false;
					} else {
						args.mLooped = arrg.getPropertyBool("loop");
					}
					args.mVolume		 = arrg.getPropertyFloat("volume");
					args.mAmSlideContent = true;
					//args.mSendToBack	 = true;
					engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(args));
				},
				0.01f);
		});
		*/
		return mediaTemplate;

	} else {
		return new TemplateBase(engine, def, model);
	}
}




TemplateConfig::TemplateConfig(ds::ui::SpriteEngine& eng) : mEngine(eng) {
	//load all the templated from the waffles_integration.xml file
	//invalid
	TemplateDef invalid;
	invalid.id = "FiddleSticks";
	invalid.name = "invalid";
	invalid.layoutXml = "waffles/template/invalid.xml";
	invalid.requiresClear = false;
	mTemplateDefinitions.push_back(invalid);
	initializeTemplateDefs();
	
}

void TemplateConfig::initializeTemplateDefs() {
	// Load the template definitions from the settings
	auto cnt = mEngine.getWafflesSettings().countSetting("template:config");
	for (int i = 0; i < cnt; i++) {
		auto setting = mEngine.getWafflesSettings().getString("template:config", i, "");
		if (setting.empty()) continue;

		TemplateDef def;
		std::vector<std::string> configStr = ds::split(setting, ":");
		if (configStr.size() < 4) {
			DS_LOG_WARNING("Invalid template config setting: " << setting);
			continue;
		}
		if (configStr.size() > 4) {
			def.extra = std::vector<std::string>(configStr.begin() + 4, configStr.end());
		}
	
		auto [name, uid, path,clear] = std::tie(configStr[0], configStr[1], configStr[2], configStr[3]);
		def.name = name;
		def.id = uid;
		def.layoutXml = path;
		//def.requiresClear = ds::parseBoolean(clear);
		mTemplateDefinitions.push_back(def);
	}
}



} // namespace waffles
