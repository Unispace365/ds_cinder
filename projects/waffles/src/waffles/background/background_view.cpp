#include "stdafx.h"

#include "background_view.h"

#include <cinder/Rand.h>

#include <ds/app/engine/engine_events.h>
#include <ds/app/environment.h>
#include <ds/content/content_events.h>
#include <ds/debug/logger.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/pdf_player.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "app/waffles_app_defs.h"


#include "waffles/waffles_events.h"
#include "waffles/template/template_config.h"
#include "waffles/background/background_view.h"
#include "waffles/background/media_background.h"
#include "waffles/background/particle_background/particle_background.h"

namespace {

// Register our custom UI elements with the engine.

auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
		// Register our custom sprite(s).
		e.registerSpriteImporter("background_view", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
			return new waffles::BackgroundView(enginey);
		});
	});
	return true;
}();

} // namespace

namespace waffles {

BackgroundView::BackgroundView(ds::ui::SpriteEngine& g)
	: ds::ui::Sprite(g)
	, mEventClient(g) {

	setTransparent(false);
	//set background color from waffles settings
	auto& color = mEngine.getColors().getColorFromName("waffles:background");
	setColor(color);

	mEventClient.listenToEvents<RequestBackgroundChange>([this](auto& e) { //
		startBackground(e.mBackgroundType, e.mMedia, e.mPdfPage);
	});

	mEventClient.listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) {
		auto helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
		auto model = ds::model::ContentModelRef("Empty");
		model.setProperty("type_uid", waffles::getTemplateDefFromName("empty").id);
		ds::Resource r = helper->getBackgroundForPlatform();
		DS_LOG_INFO("BackgroundView got from getBackground: " << r.getAbsoluteFilePath());
		if (helper->getApplyParticles()) {
			mEngine.getNotifier().notify(waffles::RequestBackgroundChange(
				waffles::BACKGROUND_TYPE_PARTICLES, ds::model::ContentModelRef())); // 1 = BACKGROUND_TYPE_PARTICLES
		} else if (!r.empty()) {
			model.setPropertyResource("media_res", r);
			mEngine.getNotifier().notify(waffles::RequestBackgroundChange(waffles::BACKGROUND_TYPE_USER_MEDIA, model));
		} else {
			mEngine.getNotifier().notify(
				waffles::RequestBackgroundChange(waffles::BACKGROUND_TYPE_DEFAULT, ds::model::ContentModelRef()));
		}
	});



}

void BackgroundView::startBackground(const int type, ds::model::ContentModelRef media, const int pdfPage) {

	if (type == BACKGROUND_TYPE_NONE && !mCurrentBackground) {
		mEngine.getNotifier().notify(BackgroundChangeComplete());
		return;
	} else if (type == BACKGROUND_TYPE_PARTICLES) {
		auto partBackground = dynamic_cast<ParticleBackground*>(mCurrentBackground);
		std::string mp = mEngine.mContent.getChildByName("background.particle")
							 .getPropertyResource("media_res")
							 .getAbsoluteFilePath();

		if (pdfPage < 2 && partBackground &&
			partBackground->getMediaPath() == mEngine.mContent.getChildByName("background.particle")
												  .getPropertyResource("media_res")
												  .getAbsoluteFilePath()) {
			mEngine.getNotifier().notify(BackgroundChangeComplete());
			return; // samesies
		}
	} else if (type == BACKGROUND_TYPE_DEFAULT || type == BACKGROUND_TYPE_USER_MEDIA) {
		auto medBackground = dynamic_cast<MediaBackground*>(mCurrentBackground);
		if (medBackground) {
			auto prevMedia = medBackground->getContentModel();

			bool pagesMatch = true;
			auto mp			= medBackground->getSprite<ds::ui::MediaPlayer>("the_player");
			if (mp) {
				auto playah = dynamic_cast<ds::ui::PDFPlayer*>(mp->getPlayer());
				if (playah) {
					if (pdfPage != playah->getPageNum()) pagesMatch = false;
				}
			}

			if (pagesMatch && prevMedia.getPropertyResource("media_res").getAbsoluteFilePath() ==
								  media.getPropertyResource("media_res").getAbsoluteFilePath()) {
				// new request is the same, so ignore it, yay!
				mEngine.getNotifier().notify(BackgroundChangeComplete());
				return;
			}
		}
	}

	if (mCurrentBackground) {
		auto sp = mCurrentBackground;
		sp->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [sp] { sp->release(); });
		mCurrentBackground = nullptr;
	}

	if (type == BACKGROUND_TYPE_NONE) {
		// no background
	} else if (type == BACKGROUND_TYPE_PARTICLES) {
		// particles
		mCurrentBackground = addChildPtr(new ParticleBackground(mEngine));

	} else if (type == BACKGROUND_TYPE_USER_MEDIA) {
		if (media.empty()) {
			DS_LOG_WARNING("Media not found for current user media - skipping setting the background");
			return;
		}
		auto theUser = mEngine.mContent.getChildByName("background.user");
		theUser.setProperty("pdf_page", pdfPage);
		theUser.clearChildren();
		theUser.addChild(media);
		auto mb = new MediaBackground(mEngine);

		mb->setContentModel(media);

		auto mp = mb->getSprite<ds::ui::MediaPlayer>("the_player");
		if (mp) {
			auto playah = dynamic_cast<ds::ui::PDFPlayer*>(mp->getPlayer());
			if (playah) {
				playah->setPageNum(pdfPage);
			}
		}

		mCurrentBackground = addChildPtr(mb);


	} else if (type == BACKGROUND_TYPE_DEFAULT) {
		auto mb = new MediaBackground(mEngine);
		auto mediaModel = mEngine.mContent.getChildByName("background.default");
		mb->setContentModel(mediaModel);
		mCurrentBackground = addChildPtr(mb);
	}

	auto backgroundModel = mEngine.mContent.getChildByName("background");
	auto  currentModel	  = backgroundModel.getChildByName("current");
	currentModel.clearChildren();
	currentModel.addChild(media);
	currentModel.setProperty("type", type);
	backgroundModel.replaceChild(currentModel);

	mEngine.getNotifier().notify(BackgroundChangeComplete());
}


} // namespace mv
