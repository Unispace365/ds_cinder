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

#include "app/app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {

const TemplateDef& getTemplateDefFromName(const std::string& name) {
	auto findy = std::find_if(Config::TemplateDefinitions.begin(), Config::TemplateDefinitions.end(),
							  [name](const auto& a) { return name == a.name; });

	if (findy != Config::TemplateDefinitions.end()) {
		return *findy;
	} else {
		DS_LOG_ERROR("Failed to load template named " << name << "! Showing Invalid template");
		return Config::TemplateDefinitions[0]; // Fallback to 'invalid' template
	}
}

const TemplateDef& getTemplateDefFromId(const std::string& id) {
	auto findy = std::find_if(Config::TemplateDefinitions.begin(), Config::TemplateDefinitions.end(),
							  [id](const auto& a) { return id == a.id; });

	if (findy != Config::TemplateDefinitions.end()) {
		return *findy;
	} else {
		DS_LOG_ERROR("Failed to load template with ID " << id << "! Showing Invalid template");
		return Config::TemplateDefinitions[0]; // Fallback to 'invalid' template
	}
}

TemplateBase* createTemplate(ds::ui::SpriteEngine& engine, ds::model::ContentModelRef model) {
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

	} */ else if (def.name == "triangle_message") {
		DS_LOG_INFO("\n\n\n--- START TRIANGLE MESSAGE ---\n\n\n");

		auto templ = new TemplateBase(engine, def, model);

		auto mediaPlayer = templ->getSprite<ds::ui::MediaPlayer>("media");
		if (mediaPlayer) {
			auto vidPlayer = dynamic_cast<ds::ui::VideoPlayer*>(mediaPlayer->getPlayer());
			if (vidPlayer) {
				auto video = vidPlayer->getVideo();
				if (video) {
					video->setVolume(model.getPropertyFloat("volume") / 100.0f);
				}
			}
		}

		auto res			= model.getPropertyResource("media");
		auto triangles		= templ->getSprite("tris");
		auto extraTriangles = templ->getSprite("tris2");
		if (triangles && extraTriangles) {
			if (res.empty()) {
				triangles->show();
				extraTriangles->show();
			} else {
				triangles->hide();
				extraTriangles->hide();
			}
		}

		return templ;

	} else if (def.name == "media") {
		auto mediaTemplate = new TemplateBase(engine, def, model);
		mediaTemplate->setAnimStartCb([mediaTemplate, model, &engine] {
			mediaTemplate->callAfterDelay(
				[mediaTemplate, model, &engine] {
					auto arrg		 = model.duplicate();
					auto displayType = arrg.getPropertyString("display_mode") == "pioTM16AbaGQ" ? std::string("fit") // TODO: not straight UUID
																								: std::string("fill");

					auto targetOffset =
						engine.getAppSettings().getVec2("template:media:waffles_offset", 0, ci::vec2(0.f, 2160.f));
					auto targetSize =
						engine.getAppSettings().getVec2("template:media:waffles_size", 0, ci::vec2(2560.f, 1080.f));
					float startWidth   = targetSize.x;
					float targetAspect = targetSize.x / targetSize.y;
					auto  aspect =
						arrg.getPropertyResource("media").getWidth() / arrg.getPropertyResource("media").getHeight();
					if (displayType == "fit") {
						if (aspect > targetAspect) {
							startWidth = targetSize.x;
						} else {
							auto scale = targetSize.y / arrg.getPropertyResource("media").getHeight();
							startWidth = arrg.getPropertyResource("media").getWidth() * scale;
						}
						// Ensure height fits, otherwise startWidth = full width
					} else {
						if (aspect > targetAspect) {
							auto scale = targetSize.y / arrg.getPropertyResource("media").getHeight();
							startWidth = arrg.getPropertyResource("media").getWidth() * scale;
						} else {
							startWidth = targetSize.x;
						}
					}
					auto args = waffles::ViewerCreationArgs(
						model, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER,
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
		return mediaTemplate;

	} else {
		return new TemplateBase(engine, def, model);
	}
}


} // namespace waffles
