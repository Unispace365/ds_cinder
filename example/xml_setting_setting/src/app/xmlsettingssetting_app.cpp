#include "xmlsettingssetting_app.h"

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "ui/settings/settings_ui.h"
namespace setter {

XmlSettingsSetting::XmlSettingsSetting()
	: inherited(ds::RootList().ortho())
	, mGlobals(mEngine )
	, mSettings(nullptr)
	, mTestSprite(nullptr)
{

	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/FONT_FILE_HERE.ttf"), "font-name-here");

	enableCommonKeystrokes(true);

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
}

void XmlSettingsSetting::setupServer(){


	mEngine.loadTextCfg("text.xml");

//	mEngine.getRootSprite(0).clearChildren();
//	mEngine.getRootSprite(1).clearChildren();
//	mEngine.getRootSprite(2).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.2f, 0.1f, 0.6f));

	mTestSprite = new ds::ui::Sprite(mEngine, 100.0f, 100.0f);
	mTestSprite->setTransparent(false);
	mTestSprite->setColor(ci::Color(0.6f, 0.2f, 0.2f));
	mTestSprite->tweenPosition(ci::vec3(500.0f, 500.0f, 0.0f), mGlobals.getSettingsLayout().getFloat("animation:duration", 0, 0.2f), mGlobals.getSettingsLayout().getFloat("animation:delay", 0, 0.2f), ci::EaseInOutExpo(), [this](){tweenTestSprite(mTestSprite); });
	rootSprite.addChild(*mTestSprite);

	mSettings = new SettingsUi(mGlobals);
	rootSprite.addChild(*mSettings);
	// add sprites

}

void XmlSettingsSetting::tweenTestSprite(ds::ui::Sprite* bs){
	bs->tweenPosition(ci::vec3(ci::Rand::randFloat(0.0f, mEngine.getWorldWidth()), ci::Rand::randFloat(0.0f, mEngine.getWorldHeight()), 0.0f), mGlobals.getSettingsLayout().getFloat("animation:duration", 0, 0.2f), mGlobals.getSettingsLayout().getFloat("animation:delay", 0, 0.2f), ci::EaseInOutExpo(), [this, bs](){tweenTestSprite(bs); });
}

void XmlSettingsSetting::update() {
	inherited::update();

	if(mTestSprite){
		mTestSprite->setColorA(mGlobals.getSettingsLayout().getColorA("sprite:color", 0, ci::ColorA(0, 0, 0, 0)));
	}
}

void XmlSettingsSetting::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getCode() == KeyEvent::KEY_x){
		if(mSettings){
			mSettings->toggleVisibility();
		}
	}
}

} // namespace setter

// This line tells Cinder to actually create the application
CINDER_APP(setter::XmlSettingsSetting, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

