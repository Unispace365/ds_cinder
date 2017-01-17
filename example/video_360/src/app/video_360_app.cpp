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
	, mHeader(nullptr)
	, mLabelText(nullptr)
	, mPanoramicVideo(nullptr)
	, mOverlay(nullptr)
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

	mHeader = nullptr;
	mLabelText = nullptr;
	mPanoramicVideo = nullptr;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.3f, 0.3f, 0.3f));
	rootSprite.setPosition(0.0f, 0.0f, 0.0f);
	rootSprite.enable(true);
	rootSprite.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	rootSprite.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		if(mPanoramicVideo){
			mPanoramicVideo->move(ti.mDeltaPoint);
			if(mOverlay){
				mOverlay->setPosition(mPanoramicVideo->getPosition());
			}
		}
	});

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
		if (mPanoramicVideo){
			mPanoramicVideo->getVideo()->setPan(-1.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_c){
		if (mPanoramicVideo){
			mPanoramicVideo->getVideo()->setPan(0.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_r){
		if (mPanoramicVideo){
			mPanoramicVideo->getVideo()->setPan(1.0f);
		}
	} else if (event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getChar() == KeyEvent::KEY_1){
		if (!mPanoramicVideo->getVideo()->removeShader("test1")) {
			mPanoramicVideo->getVideo()->addNewShader(ds::Environment::getAppFolder("data/shaders"), "test1");
		}
	} else if (event.getChar() == KeyEvent::KEY_2){
		if(!mPanoramicVideo->getVideo()->removeShader("test2")) {
			mPanoramicVideo->getVideo()->addNewShader(ds::Environment::getAppFolder("data/shaders"), "test2");
		}
	} else if (event.getChar() == KeyEvent::KEY_3){
		if(!mPanoramicVideo->getVideo()->removeShader("toonify")) {
			mPanoramicVideo->getVideo()->addNewShader(ds::Environment::getAppFolder("data/shaders"), "toonify");
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

	if(!mHeader || !mLabelText) return;
	mLabelText->setText(newMedia);

	if(mPanoramicVideo){
		mPanoramicVideo->release();
		mPanoramicVideo = nullptr;
	}

	if(!mPanoramicVideo){
		mPanoramicVideo = new ds::ui::PanoramicVideo(mEngine);
		mEngine.getRootSprite().addChildPtr(mPanoramicVideo);
	}


	//Configure drone video
	mPanoramicVideo->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mPanoramicVideo->setFOV(90.0f);
	//mPanoramicVideo->setCenter(0.0f, 0.5f);
//	mPanoramicVideo->setPosition(0.25f* mEngine.getWorldWidth(), 0.5f * mEngine.getWorldHeight());
	mPanoramicVideo->loadVideo(newMedia);

	return;

	mOverlay = new ds::ui::Sprite(mEngine, mPanoramicVideo->getWidth(), mPanoramicVideo->getHeight());
	mOverlay->enable(false);
	mOverlay->setTransparent(false);
	mOverlay->setColor(ci::Color(0.5f, 0.2f, 0.2f));
	mOverlay->setOpacity(0.5f);
	mEngine.getRootSprite().addChildPtr(mOverlay);


	auto realVideo = mPanoramicVideo->getVideo();
	//Set shader uniforms - Shaders are enabled/disabled by user keyboard input
	ds::gl::Uniform uniform;
	uniform.setInt("Texture0", 1);  // Use texture unit 1 since Vidoe CSC is hardcoded to TU 0
	realVideo->setShadersUniforms("toonify", uniform);

	uniform.clear();
	uniform.setFloat("opacity", 0.9f);
	uniform.setInt("tex0", 1);  // Use texture unit 1 since Vidoe CSC is hardcoded to TU 0
	realVideo->setShadersUniforms("test2", uniform);

	uniform.clear();
	uniform.setInt("tex1", 1);
	uniform.setFloat("opacity", 1.0f);
	realVideo->setShadersUniforms("test1", uniform);

}


} // namespace test

// This line tells Cinder to actually create the application
CINDER_APP(test::video_360, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

