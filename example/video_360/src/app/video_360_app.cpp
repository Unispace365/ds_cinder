#include "video_360_app.h"

#include <cinder/Clipboard.h>

#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <ds/ui/sprite/video.h>

namespace test {

video_360::video_360()
: inherited()
, mMedia(nullptr)
	, mHeader(nullptr)
	, mLabelText(nullptr)
{
	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");
	enableCommonKeystrokes(true);
}

void video_360::setupServer(){
	/* Settings */
	mEngine.loadSettings("layout", "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite().clearChildren();
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.3f, 0.3f, 0.3f));
	rootSprite.setPosition(0.0f, 0.0f, 0.0f);
	rootSprite.enable(false);
	const float headerHeight = mEngine.getSettings("layout").getFloat("header:height", 0, 100.0f);
	mHeader = new ds::ui::Sprite(mEngine);
	mHeader->setSize(mEngine.getWorldWidth(), headerHeight);
	mHeader->setColor(ci::Color(0.0f, 0.0f, 0.0f));
	mHeader->setTransparent(false);
	mHeader->enable(false);
	rootSprite.addChildPtr(mHeader);

	mLabelText = mEngine.getEngineCfg().getText("header:label").create(mEngine, mHeader);
	mLabelText->setPosition(20.0f,20.0f);
	mLabelText->setText("Drop a file or paste a file path");

	mDroneVideo = new ds::ui::DroneVideoSprite(mEngine);
	rootSprite.addChildPtr(mDroneVideo);
}

void video_360::update() {
	inherited::update();

}

void video_360::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);

	if(event.getChar() == KeyEvent::KEY_v && event.isControlDown() && ci::Clipboard::hasString()){
		loadMedia(ci::Clipboard::getString());
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_l){
		if (mDroneVideo){
			mDroneVideo->getVideo()->setPan(ds::ui::GstVideo::kPanLeft);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_c){
		if (mDroneVideo){
			mDroneVideo->getVideo()->setPan(ds::ui::GstVideo::kPanCenter);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_r){
		if (mDroneVideo){
			mDroneVideo->getVideo()->setPan(ds::ui::GstVideo::kPanRight);
		}
	} else if (event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getChar() == KeyEvent::KEY_1){
		if (!mBaseVideo->removeShader("test1")) {
			mBaseVideo->addNewShader(ds::Environment::getAppFolder("data/shaders"), "test1");
		}
	} else if (event.getChar() == KeyEvent::KEY_2){
		if (!mBaseVideo->removeShader("test2")) {
			mBaseVideo->addNewShader(ds::Environment::getAppFolder("data/shaders"), "test2");
		}
	} else if (event.getChar() == KeyEvent::KEY_3){
		if (!mBaseVideo->removeShader("toonify")) {
			mBaseVideo->addNewShader(ds::Environment::getAppFolder("data/shaders"), "toonify");
		}
	}
}

void video_360::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		loadMedia((*it).string());
		return; // only do the first one
	}
}

void video_360::loadMedia(const std::string& newMedia){

	if (!mHeader || !mLabelText) return;

	if (mMedia){
		mMedia->release();
	}
	mMedia = nullptr;

	mLabelText->setText(newMedia);

	const float headerHeight = mHeader->getHeight();

	Poco::File filey = Poco::File(newMedia);
	std::string extensionay = Poco::Path(filey.path()).getExtension();
	std::cout << "File extension: " << extensionay << std::endl;
	std::transform(extensionay.begin(), extensionay.end(), extensionay.begin(), ::tolower);

	mBaseVideo = new ds::ui::Video(mEngine);

	//Set shader uniforms - Shaders are enabled/disabled by user keyboard input
	ds::gl::Uniform uniform;
	uniform.setInt("Texture0", 1);  // Use texture unit 1 since Vidoe CSC is hardcoded to TU 0
	mBaseVideo->setShadersUniforms("toonify", uniform);

	uniform.clear();
	uniform.setFloat("opacity", 0.9f);
	uniform.setInt("tex0", 1);  // Use texture unit 1 since Vidoe CSC is hardcoded to TU 0
	mBaseVideo->setShadersUniforms("test2", uniform);

	uniform.clear();
	uniform.setInt("tex1", 1);
	uniform.setFloat("opacity", 1.0f);
	mBaseVideo->setShadersUniforms("test1", uniform);

	//Configure drone video
	mDroneVideo->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	//mDroneVideo->setSize(1000.0f, 500.0f);
	mDroneVideo->installVideo(mBaseVideo, newMedia);
	mDroneVideo->setCenter(0.5f, 0.5f);
	mDroneVideo->setPosition(0.5f * mEngine.getWorldWidth(), 0.5f * mEngine.getWorldHeight());
	mDroneVideo->setUseDepthBuffer(false);
	mMedia = mDroneVideo;
}



void video_360::fitSpriteInArea(ci::Rectf area, ds::ui::Sprite* spriddy){
	if (!spriddy) return;

	const float engAsp = area.getWidth() / area.getHeight();
	const float vidAsp = spriddy->getWidth() / spriddy->getHeight();
	float vidScale = area.getHeight() / spriddy->getHeight();
	if(vidAsp > engAsp){
		vidScale = area.getWidth() / spriddy->getWidth();
	}
	spriddy->setScale(vidScale);
	spriddy->setPosition(area.getX1() + area.getWidth() / 2.0f - spriddy->getScaleWidth() / 2.0f, area.getY1() + area.getHeight() / 2.0f - spriddy->getScaleHeight() / 2.0f);
}

} // namespace test

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(test::video_360, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))