#include "stdafx.h"

#include <cinder/app/App.h>
#include <Poco/Random.h>

#include <cinder/app/RendererGl.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/automator/automator.h>
#include <ds/debug/automator/actions/base_action.h>
#include <ds/debug/automator/actions/callback_action.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class AutomatorApp : public ds::App {
public:
	AutomatorApp();

	void						setupServer();
	void						handleTouchUno(const ci::vec3& pos);
	void						handleTouchDuo(const ci::vec3& pos);
	void						keyDown(ci::app::KeyEvent event);
	void						recenterSprites();

private:
	typedef ds::App				inherited;
	ds::ui::Image&				mSprite1;
	ds::ui::Image&				mSprite2;
	ds::debug::Automator		mAutomator;
};

AutomatorApp::AutomatorApp()
	: mSprite1(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
	, mSprite2(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
	, mAutomator(mEngine)
{
}

void AutomatorApp::setupServer()
{
	enableCommonKeystrokes();
	const ci::vec2     cen(getWindowCenter());
	ds::ui::Sprite     &rootSprite = mEngine.getRootSprite();

	mSprite1.setCenter(0.5f, 0.5f);
	mSprite1.setPosition(cen.x, cen.y);
	mSprite1.setScale(0.5f, 0.5f);
	mSprite1.enable(true);
	mSprite1.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mSprite1.setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){
		handleTouchUno(ti.mCurrentGlobalPoint);
	});
	rootSprite.addChild(mSprite1);

	mSprite2.setCenter(0.5f, 0.5f);
	mSprite2.setPosition(cen.x*0.25f, cen.y*0.25f);
	mSprite2.setScale(0.25f, 0.25f);
	mSprite2.enable(true);
	mSprite2.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mSprite2.setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){
		handleTouchDuo(ti.mCurrentGlobalPoint);
	});
	rootSprite.addChild(mSprite2);

	auto callback = [this](){recenterSprites(); };
	mAutomator.addFactory(std::shared_ptr<ds::debug::BaseActionFactory>(new ds::debug::CallbackActionFactory(callback, 1.0f, 10.0f)));
	
}

void AutomatorApp::handleTouchUno(const ci::vec3& pos){
	static Poco::Random   RND;
	// Tween to a randomized scale
	const float           nextScale = 0.25f + RND.nextFloat() * 2;
	const ci::vec3       scaleEnd(ci::vec3(nextScale, nextScale, 1));
	mSprite1.tweenPosition(pos, 0.25f, 0.0f, ci::EaseInOutExpo());
	mSprite1.tweenScale(scaleEnd, 0.25f, 0.0f, ci::EaseInOutExpo());
}

void AutomatorApp::handleTouchDuo(const ci::vec3& pos){

	mSprite2.tweenPosition(pos, 0.25f, 0.0f, ci::EaseInOutExpo());
}

void AutomatorApp::keyDown(ci::app::KeyEvent event){
	inherited::keyDown(event);
	if(event.getCode() == ci::app::KeyEvent::KEY_a){
		mAutomator.toggleActive();
	}
}

void AutomatorApp::recenterSprites(){
	mSprite2.setPosition(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f);
	mSprite1.setPosition(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f);
	mSprite1.setScale(1.0f, 1.0f);
}

// This line tells Cinder to actually create the application
CINDER_APP(AutomatorApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))
