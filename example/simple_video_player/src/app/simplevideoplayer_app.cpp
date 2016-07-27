#include "simplevideoplayer_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include <ds/ui/sprite/image.h>

#include <cinder/Rand.h>

namespace example {

SimpleVideoPlayer::SimpleVideoPlayer()
	: inherited(ds::RootList()
								.ortho() // sample ortho view
								.pickColor()

								) 
	, mGlobals(mEngine, mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mStressTestButton(nullptr)
	, mStressTesting(false)
	, mFpsDisplay(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSansCJKsc-Thin.otf"), "noto-thin");

	enableCommonKeystrokes(true);
}

void SimpleVideoPlayer::setupServer(){

	mVideos.clear();

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites

	ds::ui::Image* background = new ds::ui::Image(mEngine);
	background->setImageFile("%APP%/data/images/background.png");
	rootSprite.addChildPtr(background);
	
	ds::ui::Sprite* aColor = new ds::ui::Sprite(mEngine);
	aColor->setColor(ci::Color(0.1f, 0.1f, 0.1f));
	aColor->setTransparent(false);
	aColor->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	aColor->enable(false);
	aColor->setOpacity(0.5f);
	rootSprite.addChildPtr(aColor);

	mStressTestButton = new ds::ui::SpriteButton(mEngine);
	mStressTestButton->setColor(ci::Color(0.2f, 0.2f, 0.2f));
	mStressTestButton->setTransparent(false);
	mStressTestButton->setSize(600.0f, 100.0f);
	mStressTestButton->getHighSprite().setTransparent(false);
	mStressTestButton->getHighSprite().setColor(ci::Color(0.3f, 0.3f, 0.3f));
	mStressTestButton->getHighSprite().setSize(600.0f, 100.0f);
	mStressTestButton->setPosition(200.0f, 200.0f);

	mStressText = new ds::ui::Text(mEngine);
	mStressText->setFont("noto-thin", 24.0f);
	mStressText->setColor(ci::Color::white());
	mStressText->setText("Normal mode. Click for stress mode");
	mStressText->setPosition(40.0f, 30.0f);
	mStressTestButton->addChildPtr(mStressText);

	mStressTestButton->setClickFn([this](){
		mStressTesting = !mStressTesting;
		if(mStressText){
			if(mStressTesting){
				mStressText->setText("Stress test mode.");
			} else {
				mStressText->setText("Normal mode.");
			}
		}
	});
	rootSprite.addChildPtr(mStressTestButton);



	mFpsDisplay = new ds::ui::Text(mEngine);
	mFpsDisplay->setFont("noto-thin", 24.0f);
	mFpsDisplay->setColor(ci::Color::white());
	mFpsDisplay->setText("GstFps");
	mFpsDisplay->setPosition(mEngine.getWorldWidth()/2.0f, 30.0f);
	rootSprite.addChildPtr(mFpsDisplay);
}

void SimpleVideoPlayer::update() {
	inherited::update();

	if(mStressTesting && !mVideoPaths.empty()){
		static int removeCount = 0;
		removeCount++;
		if(removeCount % 3000 == 0){
			startVideos(mVideoPaths);
		}
	}

	if(!mVideos.empty() && mFpsDisplay){
		std::stringstream ss;
		ss << "fps: " << mVideos.front()->getVideoPlayingFramerate();
		mFpsDisplay->setText(ss.str());
	}
}

void SimpleVideoPlayer::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	}
}

void SimpleVideoPlayer::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		paths.push_back((*it).string());
	}
	startVideos(paths);
}

void SimpleVideoPlayer::startVideos(const std::vector<std::string> vidPaths){
	
// 	for(auto it = mVideos.begin(); it < mVideos.end(); ++it){
// 		(*it)->stop();
// 		(*it)->release();
// 	}
// 	mVideos.clear();

	mVideoPaths = vidPaths;


	float xp = 0.0f;
	float yp = 0.0f;
	float vidWidth = mEngine.getWorldWidth();
	float vidHeight = mEngine.getWorldHeight();

	int numVideos = 1;
	if(mStressTesting){
		numVideos = 4;
		vidWidth = mEngine.getWorldWidth() / (float)sqrt(numVideos);
		vidHeight = mEngine.getWorldHeight() / (float)sqrt(numVideos);
	}

	int curVideo = 0;

	for(int i = 0; i < numVideos; i++){
		ds::ui::Video* video = nullptr;
		if(mVideos.empty()){
			video = new ds::ui::Video(mEngine);
		} else {
			video = mVideos.front();
		}
		video->setLooping(true);

		video->setVerboseLogging(true);
		video->loadVideo(mVideoPaths[curVideo]);
		curVideo++;
		if(curVideo > mVideoPaths.size() - 1){
			curVideo = 0;
		}
		video->enable(true);
		video->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		video->setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
			ds::ui::Video* video = dynamic_cast<ds::ui::Video*>(bs);
			if(video){
				if(video->getIsPlaying()){
					video->pause();
				} else {
					video->play();
				}
			}
		});

		//video->setOpacity(ci::randFloat());

		fitVideoInArea(ci::Rectf(xp, yp, xp + vidWidth, yp + vidHeight), video);
		xp += vidWidth;
		if(xp > mEngine.getWorldWidth() - vidWidth){
			xp = 0.0f;
			yp += vidHeight;
		}

		mVideos.push_back(video);

		mEngine.getRootSprite().addChildPtr(video);
	}

	if(mFpsDisplay){
		mFpsDisplay->sendToFront();
	}
}

void SimpleVideoPlayer::fitVideoInArea(ci::Rectf area, ds::ui::Video* video){
	const float engAsp = area.getWidth() / area.getHeight();
	const float vidAsp = video->getWidth() / video->getHeight();
	float vidScale = area.getHeight() / video->getHeight();
	if(vidAsp > engAsp){
		vidScale = area.getWidth() / video->getWidth();
	}
	video->setScale(vidScale);
	video->setPosition(area.getX1() + area.getWidth() / 2.0f - video->getScaleWidth() / 2.0f, area.getY1() + area.getHeight() / 2.0f - video->getScaleHeight() / 2.0f);
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(example::SimpleVideoPlayer, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))