#include "stdafx.h"

#include <cinder/app/App.h>
#include <cinder/Rand.h>
#include <cinder/app/RendererGl.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/circle.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class AnimationApp : public ds::App {
public:
	AnimationApp();

	void						setupServer();
	virtual void				onKeyDown(ci::app::KeyEvent event) override;
	void						update();

private:
	typedef ds::App	 inherited;
	ds::ui::Sprite&		mSprite1;
	ds::ui::Image&		mSprite2;
	ds::ui::Circle&		mSprite3;
	size_t				mCallbackId;
};

AnimationApp::AnimationApp()
	: mSprite1(*(new ds::ui::Sprite(mEngine)))
	, mSprite2(*(new ds::ui::Image(mEngine, "%APP%/data/this_is_real.png")))
	, mSprite3(*(new ds::ui::Circle(mEngine, true, 25.0f)))
{
	mCallbackId = mEngine.repeatedCallback([this] { std::cout << "hey" << std::endl; }, 0.5);
}

void AnimationApp::onKeyDown(ci::app::KeyEvent event){
	if(event.getCode() == ci::app::KeyEvent::KEY_p){
		mEngine.setTouchSmoothing(!mEngine.getTouchSmoothing());
	} else if(event.getCode() == ci::app::KeyEvent::KEY_c){
		mSprite1.completeTweenPosition(true);
	} else if(event.getCode() == ci::app::KeyEvent::KEY_v){
		mSprite1.animStop();
	} else if(event.getCode() == ci::app::KeyEvent::KEY_l) {
		mEngine.cancelTimedCallback(mCallbackId);
	}
}

void AnimationApp::update(){
	inherited::update();

	return;

	std::cout << "Position tween is ";
	//if(std::cout.fail()) {
	//	std::cout.clear();
	//}
	if(!mSprite1.getPositionTweenIsRunning()){
		std::cout << "not ";
	}
	std::cout << "running." << std::endl;
	console() << "Hello there" << std::endl;
}

void AnimationApp::setupServer()
{
	const ci::vec2		 cen(getWindowCenter());
	ds::ui::Sprite		 &rootSprite = mEngine.getRootSprite();

	mSprite1.enable(false);
	mSprite1.setCenter(0.5f, 0.5f);
	mSprite1.setPosition(cen.x, cen.y);
	mSprite1.setScale(0.5f, 0.5f);
	mSprite1.setTransparent(false);
	mSprite1.setSize(100, 100);
	mSprite1.setColor(ci::Color(1.0f, 0.2f, 0.2f));
	rootSprite.addChild(mSprite1);

	mSprite2.enable(false);
	mSprite2.setCenter(0.5f, 0.5f);
	mSprite2.setPosition(cen.x*0.25f, cen.y*0.25f);
	mSprite2.setScale(0.25f, 0.25f);
	rootSprite.addChild(mSprite2);

	mSprite3.setScale(10.0f, 10.0f);
	mSprite3.setColor(ci::Color(0.8f, 0.1f, 0.3f));
	mSprite3.setPosition(600.0f, 200.0f);
	rootSprite.addChild(mSprite3);

	rootSprite.setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	rootSprite.enable(true);
	rootSprite.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	rootSprite.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		if(ti.mPhase != ds::ui::TouchInfo::Added) return;

		// These are the easiest tweens to use, the built-in sprite tweens. They clear out the previous tween of the same type (tweening position twice in a row only does the second tween)
		// Tween to a randomized scale
		const float				nextScale = ci::Rand::randFloat(10.25f, 1000.5f);
		const ci::vec3			scaleEnd(ci::vec3(nextScale, nextScale, 1.0f));
		mSprite1.tweenSize(scaleEnd, 0.5f, 1.0f, ci::EaseInOutCubic());
		ci::vec3 possy = ti.mCurrentGlobalPoint;
		mSprite1.tweenPosition(ti.mCurrentGlobalPoint, 0.5f, 0.0f, ci::EaseInOutCubic(), [this, possy]{
			mSprite1.tweenPosition(mSprite1.getPosition() + ci::vec3(100.0f, 100.0f, 0.0f), 0.5f, 0.0f, ci::easeInOutCubic);
			mSprite1.tweenOpacity(ci::randFloat()); 
			std::cout << "Sprite 1 position complete!" << std::endl;
		});

		// An easy way to tween a sprite with a string script (that you can set via an xml setting or something)
		mSprite3.runAnimationScript("fade; slide:100; duration:0.5; delay:0.2; ease:outBack");

		// In case you need a more specific tween or custom usage (not common) you can access to the raw tweenline
		// A custom tween parameter -- only position y (also could have been done by
		// setting the end position.x and .z values to the same as the start, this is
		// here just to illustrate how to do a custom)
		ds::ui::SpriteAnim<ci::vec3>	anim_pos([](ds::ui::Sprite& s)->ci::Anim<ci::vec3>& { return s.mAnimPosition; },
												 [](ds::ui::Sprite& s)->ci::vec3 { return s.getPosition(); },
												 [](const ci::vec3& v, ds::ui::Sprite& s) { s.setPosition(s.getPosition().x, v.y, s.getPosition().z); });
		mEngine.getTweenline().apply(mSprite2, anim_pos, ti.mCurrentGlobalPoint, 1.0f, EaseInOutQuart());
	});

	console() << "Hello there" << std::endl;
}

// This line tells Cinder to actually create the application
CINDER_APP(AnimationApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))
