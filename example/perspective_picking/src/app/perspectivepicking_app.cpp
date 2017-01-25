#include "perspectivepicking_app.h"

#include <cinder/app/RendererGl.h>

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

namespace perspective_picking {

PerspectivePicking::PerspectivePicking()
	: inherited(ds::RootList()

								.persp() // sample perp view
								.perspFov(60.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 960.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(2000.0f)
								)
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mDebugCamera(0)
{


	/*fonts in use */
	//mEngine.editFonts().installFont(ds::Environment::getAppFile("data/fonts/FONT_FILE_HERE.ttf"), "Font Name", "font-name-here");

	enableCommonKeystrokes(true);
}

void PerspectivePicking::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite().clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	// add sprites

	ds::ui::Sprite* testTouchUno = new ds::ui::Sprite(mEngine);
	testTouchUno->setTransparent(false);
	testTouchUno->setColor(ci::Color(0.6f, 0.2f, 0.2f));
	testTouchUno->setSize(200.0f, 200.0f);
	testTouchUno->enable(true);
	testTouchUno->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	testTouchUno->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		std::cout << "Touching uno" << std::endl;
		bs->move(ti.mDeltaPoint.x, -ti.mDeltaPoint.y);
	});
	rootSprite.addChild(*testTouchUno);



	ds::ui::Sprite* testTouchDuo = new ds::ui::Sprite(mEngine);
	testTouchDuo->setTransparent(false);
	testTouchDuo->setColor(ci::Color(0.2f, 0.2f, 0.6f));
	testTouchDuo->setSize(200.0f, 200.0f);
	testTouchDuo->setPosition(300.0f, 0.0f);
	testTouchDuo->setCenter(0.5f, 0.5f);
	testTouchDuo->enable(true);
	testTouchDuo->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	testTouchDuo->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		std::cout << "Touching duo" << std::endl;
		bs->move(ti.mDeltaPoint.x, -ti.mDeltaPoint.y);
	});
	rootSprite.addChild(*testTouchDuo);



	ds::ui::Sprite* testTouchTre = new ds::ui::Sprite(mEngine);
	testTouchTre->setTransparent(false);
	testTouchTre->setColor(ci::Color(0.2f, 0.2f, 0.6f));
	testTouchTre->setSize(200.0f, 200.0f);
	testTouchTre->setPosition(600.0f, 0.0f);
	testTouchTre->setCenter(1.0f, 1.0f);
	testTouchTre->enable(true);
	testTouchTre->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	testTouchTre->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		std::cout << "Touching tre" << std::endl;
		bs->move(ti.mDeltaPoint.x, -ti.mDeltaPoint.y);
	});
	rootSprite.addChild(*testTouchTre);
}

void PerspectivePicking::update() {
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

void PerspectivePicking::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	//inherited::keyDown(event);


	float moveAmount = 1.0f;
	if(event.isShiftDown()) moveAmount = 10.0f;
	else if(event.isControlDown()) moveAmount = 100.0f;

	bool moveTarget = false;

	if(event.isAltDown()) moveTarget = true;

	if(event.getChar() == KeyEvent::KEY_r){
		setupServer();
	} else if(event.getCode() == KeyEvent::KEY_RIGHT){
		moveCamera(ci::vec3(moveAmount, 0.0f, 0.0f), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_LEFT){
		moveCamera(ci::vec3(-moveAmount, 0.0f, 0.0f), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_UP){
		moveCamera(ci::vec3(0.0f, -moveAmount, 0.0f), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_DOWN){
		moveCamera(ci::vec3(0.0f, moveAmount, 0.0f), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_RIGHTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, moveAmount), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_LEFTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, -moveAmount), moveTarget);
	} else if(event.getCode() == KeyEvent::KEY_l){
		shiftLensH(moveAmount / 100.0f);
	} else if(event.getCode() == KeyEvent::KEY_k){
		shiftLensH(-moveAmount / 100.0f);

	} else if(event.getCode() == KeyEvent::KEY_SEMICOLON){
		moveRoot(ci::vec3(0.0f, moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_COMMA){
		moveRoot(ci::vec3(-moveAmount, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_PERIOD){
		moveRoot(ci::vec3(0.0f, -moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_SLASH){
		moveRoot(ci::vec3(moveAmount, 0.0f, 0.0f));

	} else if(event.getCode() == KeyEvent::KEY_EQUALS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(mDebugCamera);
		p.mFarPlane += moveAmount;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(mDebugCamera, p);

	} else if(event.getCode() == KeyEvent::KEY_MINUS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(mDebugCamera);
		p.mFarPlane -= moveAmount;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(mDebugCamera, p);

	} else if(event.getCode() == KeyEvent::KEY_z){
		ds::ui::Sprite& rooty = mEngine.getRootSprite(mDebugCamera);
		rooty.setPosition(ci::vec3(0.0f, 0.0f, 0.0f));
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(mDebugCamera);
		p.mTarget = ci::vec3(0.0f, 0.0f, p.mTarget.z);
		p.mPosition = ci::vec3(0.0f, 0.0f, p.mPosition.z);
		mEngine.setPerspectiveCamera(mDebugCamera, p);

		std::cout << "Reset camera " << mDebugCamera << std::endl;

	} else if(event.getCode() == KeyEvent::KEY_a){
		std::cout << getAverageFps() << std::endl;
	} else if(event.getCode() == KeyEvent::KEY_ESCAPE){
		std::exit(0);
	}
}

void PerspectivePicking::moveRoot(const ci::vec3& deltaMove){
	ds::ui::Sprite& rooty = mEngine.getRootSprite(mDebugCamera);
	rooty.move(deltaMove);
	std::cout << "Moving root " << mDebugCamera << " pos: " << rooty.getPosition() << std::endl;
}

void PerspectivePicking::moveCamera(const ci::vec3& deltaMove, const bool moveTarget){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(mDebugCamera);
	if(moveTarget){
		p.mTarget += deltaMove;
		std::cout << "Moving target " << mDebugCamera << " pos: " << p.mTarget << std::endl;
	} else {
		p.mPosition += deltaMove;
		std::cout << "Moving camera " << mDebugCamera << " pos: " << p.mPosition << std::endl;
	}
	mEngine.setPerspectiveCamera(mDebugCamera, p);
}



void PerspectivePicking::shiftLensH(const float amount){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(mDebugCamera);
	p.mLensShiftH += amount;

	std::cout << "Shifting lens " << mDebugCamera << " h-shift: " << p.mLensShiftH << std::endl;
	mEngine.setPerspectiveCamera(mDebugCamera, p);
}

} // namespace perspective_picking

// This line tells Cinder to actually create the application
CINDER_APP(perspective_picking::PerspectivePicking, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

