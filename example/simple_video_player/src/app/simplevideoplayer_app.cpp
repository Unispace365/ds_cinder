#include "simplevideoplayer_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include <ds/ui/sprite/image.h>

namespace example {

SimpleVideoPlayer::SimpleVideoPlayer()
	: inherited(ds::RootList()
								.ortho() // sample ortho view
								.pickColor()

								) 
	, mGlobals(mEngine, mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mVideo(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/FONT_FILE_HERE.ttf"), "font-name-here");

	enableCommonKeystrokes(true);
}

void SimpleVideoPlayer::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites

	ds::ui::Image* background = new ds::ui::Image(mEngine);
	background->setImageFile("%APP%/data/images/background.png");
	rootSprite.addChildPtr(background);
}

void SimpleVideoPlayer::update() {
	inherited::update();

}

void SimpleVideoPlayer::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} 
}

void SimpleVideoPlayer::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		startVideo((*it).string());
	}
}

void SimpleVideoPlayer::startVideo(const std::string& vidPath){
	if(mVideo){
		mVideo->stop();
		mVideo->release();
	}

	mVideo = nullptr;

	mVideo = new ds::ui::Video(mEngine);
	mVideo->setLooping(true);
	mVideo->setTransparent(false);
	mVideo->loadVideo(vidPath);
	mVideo->enable(true);
	mVideo->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mVideo->setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
		if(mVideo){
			if(mVideo->getIsPlaying()){
				mVideo->pause();
			} else {
				mVideo->play();
			}
		}
	});

	const float engAsp = mEngine.getWorldWidth() / mEngine.getWorldHeight();
	const float vidAsp = mVideo->getWidth() / mVideo->getHeight();
	float vidScale = mEngine.getWorldHeight() / mVideo->getHeight();
	if(vidAsp > engAsp){
		vidScale = mEngine.getWorldWidth() / mVideo->getHeight();
	}
	mVideo->setScale(vidScale);
	mVideo->setPosition(mEngine.getWorldWidth() / 2.0f - mVideo->getScaleWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f - mVideo->getScaleHeight() / 2.0f);
	mEngine.getRootSprite().addChildPtr(mVideo);
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(example::SimpleVideoPlayer, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))