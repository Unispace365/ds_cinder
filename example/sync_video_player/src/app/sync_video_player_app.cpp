#include "sync_video_player_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

namespace ds {

sync_video_player::sync_video_player()
	: inherited(ds::RootList().ortho().pickColor()) 
	, mGlobals(mEngine)
	, mVideo(nullptr)
	, mVideoHolder(nullptr)
	, mFpsDisplay(nullptr)
	, mVerbose(false)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");

	enableCommonKeystrokes(true);
}

void sync_video_player::setupServer(){

	mVideo = nullptr;

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	ds::ui::Sprite* spriggy = new ds::ui::Sprite(mEngine, mEngine.getWorldWidth(), mEngine.getWorldHeight());
	spriggy->setTransparent(false);
	spriggy->setColor(ci::Color(0.05f, 0.05f, 0.2f));
	rootSprite.addChildPtr(spriggy);

	mVideoHolder = new ds::ui::Sprite(mEngine);
	rootSprite.addChildPtr(mVideoHolder);

	mFpsDisplay = new ds::ui::Text(mEngine);
	mFpsDisplay->setFont("noto-bold", 24.0f);
	mFpsDisplay->setColor(ci::Color::white());
	mFpsDisplay->setText("GstFps");
	mFpsDisplay->setPosition(mEngine.getWorldWidth() / 4.0f, 30.0f);
	rootSprite.addChildPtr(mFpsDisplay);
}

void sync_video_player::fileDrop(ci::app::FileDropEvent event){
	if(mEngine.getMode() == ds::ui::SpriteEngine::CLIENT_MODE){
		return;
	}
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		paths.push_back((*it).string());
	}

	if(paths.empty()){
		return;
	}

// 	if(mVideo){
// 		mVideo->stop();
// 		mVideo->release();
// 		mVideo = nullptr;
// 	}

	mVideo = new ds::ui::GstVideo(mEngine);
	mVideo->setLooping(true);
	//mVideo->setVerboseLogging(mVerbose);
	mVideo->loadVideo(paths.front());
	mVideo->enable(true);
	mVideo->enableMultiTouch(ds::ui::MULTITOUCH_CAN_SCALE | ds::ui::MULTITOUCH_CAN_POSITION);
	mVideo->setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
		ds::ui::GstVideo* video = dynamic_cast<ds::ui::GstVideo*>(bs);
		if(video){
			if(video->getIsPlaying()){
				video->pause();
			} else {
				video->play();
			}
		}
	});
	mVideoHolder->addChildPtr(mVideo);
}


void sync_video_player::update() {
	inherited::update();

	if(mVideo){
		std::stringstream ss;
		ss << "fps: " << mVideo->getVideoPlayingFramerate();
//		mFpsDisplay->setText(ss.str());
	}

}

void sync_video_player::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getChar() == KeyEvent::KEY_v){
		mVerbose = !mVerbose;
		if(mVideo){
			mVideo->setVerboseLogging(mVerbose);
		}
	}
}

} // namespace ds

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(ds::sync_video_player, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))