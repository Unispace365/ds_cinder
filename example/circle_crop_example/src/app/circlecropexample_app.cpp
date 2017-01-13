#include "circlecropexample_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

// These three includes are required for the circle crop and image
#include <ds/ui/ip/ip_defs.h>
#include <ds/ui/image_source/image_file.h>
#include <ds/ui/sprite/image.h>

namespace example {

CircleCropExample::CircleCropExample()
	: inherited(ds::RootList()
								.ortho() // sample ortho view
								.pickColor()

								.persp() // sample perp view
								.perspFov(60.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 10.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) // ortho view on top
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/FONT_FILE_HERE.ttf"), "font-name-here");

	enableCommonKeystrokes(true);
}

void CircleCropExample::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();
	mEngine.getRootSprite(1).clearChildren();
	mEngine.getRootSprite(2).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.2f, 0.1f, 0.6f));
	// add sprites



	//------------------------------------------------------------------//
	//------------------ HERE'S WHAT YOU CAME FOR!!!! ------------------//
	//------------------------------------------------------------------//
	// Add an "ip" (image processing) to an image file, set that as the "image" of an ImageSprite
	// The circle mask is pre-defined. 
	// You can extend ip::function, install that on the engine, and have that run here instead
	// Pass the string key to the image file to look up the ip when the image file is created.
	// kind of confusing, but there you go.
	ds::ui::Image*	imagey = new ds::ui::Image(mEngine);
	std::string fileName = "%APP%/data/images/cupola.png";
	imagey->setImage(ds::ui::ImageFile(fileName, ds::ui::ip::CIRCLE_MASK, ""));
	rootSprite.addChildPtr(imagey);
}

void CircleCropExample::update() {
	inherited::update();

	if( mEngine.isIdling() && !mIdling ){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify( IdleStartedEvent() );
	} else if ( !mEngine.isIdling() && mIdling ){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify( IdleEndedEvent() );
	}

}

void CircleCropExample::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getCode() == KeyEvent::KEY_d){
		moveCamera(ci::vec3(1.0f, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_a){
		moveCamera(ci::vec3(-1.0f, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_w){
		moveCamera(ci::vec3(0.0f, -1.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_s){
		moveCamera(ci::vec3(0.0f, 1.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_RIGHTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, 1.0f));
	} else if(event.getCode() == KeyEvent::KEY_LEFTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, -1.0f));
	} else if(event.getCode() == KeyEvent::KEY_EQUALS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
		p.mFarPlane += 1.0f;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(1, p);
	} else if(event.getCode() == KeyEvent::KEY_MINUS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
		p.mFarPlane -= 1.0f;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(1, p);
	}
}

void CircleCropExample::moveCamera(const ci::vec3& deltaMove){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
	p.mPosition += deltaMove;
	std::cout << "Moving camera: " << p.mPosition.x << " " << p.mPosition.y << " " << p.mPosition.z << std::endl;
	mEngine.setPerspectiveCamera(1, p);
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::CircleCropExample, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))