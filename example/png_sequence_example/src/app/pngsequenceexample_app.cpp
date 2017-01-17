#include "pngsequenceexample_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/text.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"


namespace example {

PngSequenceExample::PngSequenceExample()
	: inherited(ds::RootList()
								.ortho() // sample ortho view
								.pickColor()

								 ) // ortho view on top
	, mGlobals(mEngine)
	, mTouchDebug(mEngine)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");

	enableCommonKeystrokes(true);
}

void PngSequenceExample::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	rootSprite.enable(true);
	rootSprite.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	rootSprite.setTapCallback([this](ds::ui::Sprite*, const ci::vec3&){
		if(mPngSequence){
			if(mPngSequence->isPlaying()){
				mPngSequence->pause();
			} else {
				mPngSequence->setCurrentFrameIndex(0);
				mPngSequence->play();
			}
		}
	});
	
	// add sprites

	mPngSequence = new ds::ui::PngSequenceSprite(mEngine);
	mPngSequence->setFrameTime(0.032f);
	mPngSequence->enable(true);
	mPngSequence->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	mPngSequence->setSize(400.0f, 400.0f);
	rootSprite.addChildPtr(mPngSequence);
}

void PngSequenceExample::update() {
	inherited::update();
}

void PngSequenceExample::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();

	// Shows all enabled sprites with a label for class type
	} else if(event.getCode() == KeyEvent::KEY_f){

		const int numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(int i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
					sprite.setTransparent(false);
					sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
					sprite.setOpacity(0.95f);

					ds::ui::Text* labelly = mGlobals.getText("media_viewer:title").create(mEngine, &sprite);
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	} else if(event.getCode() == KeyEvent::KEY_l){
		if(mPngSequence->getLoopStyle() == ds::ui::PngSequenceSprite::Loop){
			mPngSequence->setLoopStyle(ds::ui::PngSequenceSprite::Once);
		} else {
			mPngSequence->setLoopStyle(ds::ui::PngSequenceSprite::Loop);
		}
	}
}

void PngSequenceExample::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void PngSequenceExample::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void PngSequenceExample::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void PngSequenceExample::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		std::cout << (*it).string() << std::endl;
		paths.push_back((*it).string());

	}

	std::sort(paths.begin(), paths.end());

	if(mPngSequence){
		mPngSequence->setImages(paths);
	}
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::PngSequenceExample, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

