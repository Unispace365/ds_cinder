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
	: ds::App() 
	, mGlobals(mEngine)
	, mSlideshow(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().installFont(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "Noto Sans Bold", "noto-bold");

}

void MediaSlideshow::setupServer(){

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();

	mSlideshow = nullptr;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
}

void MediaSlideshow::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
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
CINDER_APP(example::MediaSlideshow, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })

