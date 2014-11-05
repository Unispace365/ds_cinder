#include "globeexample_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "ui/globe/globe_view.h"

namespace globe_example {

GlobeExample::GlobeExample()
	: inherited(ds::RootList()
								.persp() // sample perp view
								.perspFov(30.0f)
								.perspPosition(ci::Vec3f(0.0, 0.0f, 2080.0f))
								.perspTarget(ci::Vec3f(0.0f, 0.0f, 0.0f))
								.perspNear(1000.0f)
								.perspFar(2500.0f)
)
	, mGlobals(mEngine)
{

	enableCommonKeystrokes(true);
}

void GlobeExample::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	GlobeView* globby = new GlobeView(mGlobals);
	rootSprite.addChild(*globby);
	// add sprites
}

void GlobeExample::update() {
	inherited::update();
}

void GlobeExample::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	float moveAmount = 1.0f;
	if(event.isShiftDown()) moveAmount = 10.0f;
	if(event.isAltDown()) moveAmount = 100.0f;

	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getCode() == KeyEvent::KEY_d){
		moveCamera(ci::Vec3f(moveAmount, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_a){
		moveCamera(ci::Vec3f(-moveAmount, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_w){
		moveCamera(ci::Vec3f(0.0f, -moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_s){
		moveCamera(ci::Vec3f(0.0f, moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_RIGHTBRACKET){
		moveCamera(ci::Vec3f(0.0f, 0.0f, moveAmount));
	} else if(event.getCode() == KeyEvent::KEY_LEFTBRACKET){
		moveCamera(ci::Vec3f(0.0f, 0.0f, -moveAmount));
	} else if(event.getCode() == KeyEvent::KEY_EQUALS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
		p.mFarPlane += moveAmount;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(0, p);
	} else if(event.getCode() == KeyEvent::KEY_MINUS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
		p.mFarPlane -= moveAmount;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(0, p);
	} else if(event.getCode() == KeyEvent::KEY_0){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
		p.mNearPlane -= moveAmount;
		std::cout << "Clip near camera: " << p.mNearPlane << std::endl;
		mEngine.setPerspectiveCamera(0, p);
	} else if(event.getCode() == KeyEvent::KEY_9){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
		p.mNearPlane += moveAmount;
		std::cout << "Clip near camera: " << p.mNearPlane << std::endl;
		mEngine.setPerspectiveCamera(0, p);
	}
}

void GlobeExample::moveCamera(const ci::Vec3f& deltaMove){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
	p.mPosition += deltaMove;
	std::cout << "Moving camera: " << p.mPosition.x << " " << p.mPosition.y << " " << p.mPosition.z << std::endl;
	mEngine.setPerspectiveCamera(0, p);
}

} // namespace globe_example

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(globe_example::GlobeExample, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))