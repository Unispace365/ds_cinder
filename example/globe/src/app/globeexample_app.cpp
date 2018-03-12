#include "globeexample_app.h"

#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/text.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include <ds/cfg/settings_editor.h>


#include "ui/globe/globe_view.h"

#include "ui/population/population_view.h"

namespace globe_example {

GlobeExample::GlobeExample()
	: ds::App(ds::RootList()
								.persp() // sample perp view
								.perspFov(30.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 2000.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(10.0f)
								.perspFar(4500.0f)
)
	, mGlobals(mEngine)
{

}

void GlobeExample::setupServer(){

	// Fonts links together a font name and a physical font file
	// Then the "text.xml" and TextCfg will use those font names to specify visible settings (size, color, leading)
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachSetting([this](ds::cfg::Settings::Setting& theSetting) {
		mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
	});

	/* Get our data model synchronously, don't ever do this */
	//ci::XmlTree doc(ci::loadFile(""));

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	rootSprite.addChildPtr(new PopulationView(mGlobals) );

}

void GlobeExample::update() {
	ds::App::update();
}

void GlobeExample::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	float moveAmount = 1.0f;
	if(event.isShiftDown()) moveAmount = 10.0f;
	if(event.isAltDown()) moveAmount = 100.0f;

	if(event.getCode() == KeyEvent::KEY_d){
		moveCamera(ci::vec3(moveAmount, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_a){
		moveCamera(ci::vec3(-moveAmount, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_w){
		moveCamera(ci::vec3(0.0f, -moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_s){
		moveCamera(ci::vec3(0.0f, moveAmount, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_RIGHTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, moveAmount));
	} else if(event.getCode() == KeyEvent::KEY_LEFTBRACKET){
		moveCamera(ci::vec3(0.0f, 0.0f, -moveAmount));
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

void GlobeExample::moveCamera(const ci::vec3& deltaMove){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
	p.mPosition += deltaMove;
	std::cout << "Moving camera: " << p.mPosition.x << " " << p.mPosition.y << " " << p.mPosition.z << std::endl;
	mEngine.setPerspectiveCamera(0, p);
}

} // namespace globe_example

// This line tells Cinder to actually create the application
CINDER_APP(globe_example::GlobeExample, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))