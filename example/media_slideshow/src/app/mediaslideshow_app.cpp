#include "mediaslideshow_app.h"

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

MediaSlideshow::MediaSlideshow()
	: ds::App(ds::RootList().ortho() ) 
	, mGlobals(mEngine)
	, mTouchDebug(mEngine)
	, mSlideshow(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");

	enableCommonKeystrokes(true);
}

void MediaSlideshow::setupServer(){

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();

	const int numRoots = mEngine.getRootCount();
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);

		rooty.clearChildren();
	}

	mSlideshow = nullptr;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
}

void MediaSlideshow::update() {
	ds::App::update();
}

void MediaSlideshow::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	ds::App::keyDown(event);
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
					if(!labelly) return;
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	}
}

void MediaSlideshow::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void MediaSlideshow::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void MediaSlideshow::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void MediaSlideshow::fileDrop(ci::app::FileDropEvent event){

	std::vector<ds::Resource> slideshow;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		std::string pathy = (*it).string();
		slideshow.push_back(ds::Resource(pathy));

// 		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
// 		mv->initializeIfNeeded();
// 		mEngine.getRootSprite().addChildPtr(mv);
	}

	if(slideshow.empty()) return;

	if(mSlideshow){
		mSlideshow->release();
		mSlideshow = nullptr;
	}

	mSlideshow = new ds::ui::MediaSlideshow(mEngine);
	mSlideshow->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mSlideshow->setMediaSlideshow(slideshow);
	mEngine.getRootSprite().addChildPtr(mSlideshow);
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::MediaSlideshow, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))