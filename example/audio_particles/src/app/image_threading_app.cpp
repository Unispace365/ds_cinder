#include "image_threading_app.h"

#include <cinder/Clipboard.h>
#include <cinder/app/RendererGl.h>
#include <cinder/Rand.h>

#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/image.h>

#include <ui/particle_background.h>

namespace mv {

ImageThreading::ImageThreading()
	: inherited() 
	, mMedia(nullptr)
	, mHeader(nullptr)
	, mLabelText(nullptr)
	, mIsVideo(false)
	, mGlobals(mEngine)
{
	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");

	return;
	try{
		auto ctx = ci::audio::Context::master();

		// The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
		mInputDeviceNode = ctx->createInputDeviceNode();

		// By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
		// an increase in resolution of the resulting spectrum data.
		auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
		mMonitorSpectralNode = ctx->makeNode(new ci::audio::MonitorSpectralNode(monitorFormat));

		mInputDeviceNode >> mMonitorSpectralNode;

		// InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
		mInputDeviceNode->enable();
		ctx->enable();
	} catch(std::exception& e){
		std::cout << "Couldn't start audio stuff; " << e.what() << std::endl;
	}
}

void ImageThreading::setupServer(){

	mEngine.loadSettings("app_settings", "app_settings.xml");
	/* Settings */
	mEngine.loadSettings("layout", "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();

	mEngine.getRootSprite().clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.0f, 0.0f, 0.0f));

	rootSprite.addChildPtr(new ParticleBackground(mGlobals));
}

void ImageThreading::update() {
	inherited::update();
	if(mMonitorSpectralNode){
		mGlobals.mVolume = mMonitorSpectralNode->getVolume();
	} else {
		mGlobals.mVolume = ci::randFloat(0.0f, 0.1f);
	}
}

void ImageThreading::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	if(event.getChar() == KeyEvent::KEY_v && event.isControlDown() && ci::Clipboard::hasString()){
		//loadMedia(ci::Clipboard::getString());

	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_l){
		if (mIsVideo && mMedia){
			//static_cast<ds::ui::GstVideo*>(mMedia)->setPan(-1.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_c){
		if (mIsVideo && mMedia){
			//static_cast<ds::ui::GstVideo*>(mMedia)->setPan(0.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_r){
		if (mIsVideo && mMedia){
			//static_cast<ds::ui::GstVideo*>(mMedia)->setPan(1.0f);
		}
	} else if (event.getChar() == KeyEvent::KEY_l){
		//loadMedia("c:/test.mp4");
	} else if (event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	}
}

void ImageThreading::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){

		ds::ui::Image* imgy = new ds::ui::Image(mEngine);
		mEngine.getRootSprite().addChildPtr(imgy);
		imgy->setImageFile((*it).string());
		imgy->setOpacity(0.5f);

		const float headerHeight = mHeader->getHeight();

		imgy->setScale(100.0f / imgy->getWidth());
		imgy->setPosition(ci::randFloat(0.0f, mEngine.getWorldWidth()), ci::randFloat(0.0f, mEngine.getWorldHeight()));

		//fitSpriteInArea(ci::Rectf(0.0f, headerHeight, mEngine.getWorldWidth(), mEngine.getWorldHeight()), imgy);
		//loadMedia((*it).string());
		//return; // only do the first one
	}
}

void ImageThreading::fitSpriteInArea(ci::Rectf area, ds::ui::Sprite* spriddy){
	if(!spriddy) return;

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
CINDER_APP(mv::ImageThreading, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })

