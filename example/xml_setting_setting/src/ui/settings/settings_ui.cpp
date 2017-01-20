#include "settings_ui.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

namespace setter {

SettingsUi::SettingsUi(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mParams(nullptr)
	, mTest("Heyo")
	, mDrawParams(true)
	, mGlobals(g)
{

	mParams = ci::params::InterfaceGl::create(mEngine.getWindow(), "App parameters", ci::ivec2(600, 250));

	setTransparent(false);
// 
// 	
// 	mGlobals.getSettingsLayout().forEachFloatKey([this](const std::string& key){
// 
// 		const std::function<void(float)> &setterFn = [this, key](float value){
// 			ds::cfg::Settings& settings = const_cast<ds::cfg::Settings&>(mGlobals.getSettingsLayout());
// 			ds::cfg::Settings::Editor editor(settings);
// 			editor.setFloat(key, value); };
// 
// 		const std::function<float()> &getterFn = [this, key](){
// 			return mGlobals.getSettingsLayout().getFloat(key);
// 		};
// 
// 		mParams->addParam(key, setterFn, getterFn);
// 	});
// 	

	registerSettings(&mGlobals.getSettingsLayout());

}

void SettingsUi::registerSettings(const ds::cfg::Settings* settings){


	settings->forEachFloatKey([this, settings](const std::string& key){
		const std::function<void(float)> &setterFn = [this, settings, key](float value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setFloat(key, value); };

		const std::function<float()> &getterFn = [this, settings, key](){
			return settings->getFloat(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});

	settings->forEachColorAKey([this, settings](const std::string& key){

		const std::function<void(ci::ColorA)> &setterFn = [this, settings, key](ci::ColorA value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setColorA(key, value); };

		const std::function<ci::ColorA()> &getterFn = [this, settings, key](){
			return settings->getColorA(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});

	settings->forEachIntKey([this, settings](const std::string& key){

		const std::function<void(int)> &setterFn = [this, settings, key](int value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setInt(key, value); };

		const std::function<int()> &getterFn = [this, settings, key](){
			return settings->getInt(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});

	settings->forEachSizeKey([this, settings](const std::string& key){

		const std::function<void(ci::vec3)> &setterFn = [this, settings, key](ci::vec3 value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setSize(key, ci::vec2(value)); };

		const std::function<ci::vec3()> &getterFn = [this, settings, key](){
			ci::vec2 sizey = settings->getSize(key);
			return ci::vec3(sizey.x, sizey.y, 0.0f);
		};

		mParams->addParam(key, setterFn, getterFn);
	});
}



void SettingsUi::drawLocalClient(){
	if(mDrawParams){
		mParams->show(true);
		mParams->draw();
	} else {
		mParams->show(false);
	}
}

SettingsUi::~SettingsUi(){
	if(mParams){
		mParams->clear(); 
	}
}

void SettingsUi::toggleVisibility(){
	mDrawParams = !mDrawParams;
}


} // namespace setter


