#include <cinder/app/AppBasic.h>
#include <cinder/Rand.h>

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
	virtual void				keyDown(ci::app::KeyEvent event);

private:
	typedef ds::App	 inherited;
	ds::ui::Sprite&		mSprite1;
	ds::ui::Image&		mSprite2;
	ds::ui::Circle&		mSprite3;
};

AnimationApp::AnimationApp()
	: mSprite1(*(new ds::ui::Sprite(mEngine)))
	, mSprite2(*(new ds::ui::Image(mEngine, "%APP%/data/this_is_real.png")))
	, mSprite3(*(new ds::ui::Circle(mEngine, true, 25.0f)))
{
}

void AnimationApp::keyDown(ci::app::KeyEvent event){
	ds::App::keyDown(event);
	if(event.getCode() == ci::app::KeyEvent::KEY_p){
		mEngine.setTouchSmoothing(!mEngine.getTouchSmoothing());
	}
}

void AnimationApp::setupServer()
{
	const ci::Vec2f		 cen(getWindowCenter());
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
		const ci::Vec3f			scaleEnd(ci::Vec3f(nextScale, nextScale, 1.0f));
		mSprite1.tweenSize(scaleEnd, 0.5f, 1.0f, ci::EaseInOutCubic());
		mSprite1.tweenPosition(ti.mCurrentGlobalPoint, 0.5f, 0.0f, ci::EaseInOutCubic());

		// An easy way to tween a sprite with a string script (that you can set via an xml setting or something)
		mSprite3.runAnimationScript("fade; slide:100; duration:0.5; delay:0.2; ease:outBack");

		// In case you need a more specific tween or custom usage (not common) you can access to the raw tweenline
		// A custom tween parameter -- only position y (also could have been done by
		// setting the end position.x and .z values to the same as the start, this is
		// here just to illustrate how to do a custom)
		ds::ui::SpriteAnim<ci::Vec3f>	anim_pos([](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimPosition; },
												 [](ds::ui::Sprite& s)->ci::Vec3f { return s.getPosition(); },
												 [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setPosition(s.getPosition().x, v.y, s.getPosition().z); });
		mEngine.getTweenline().apply(mSprite2, anim_pos, ti.mCurrentGlobalPoint, 1.0f, EaseInOutQuart());
	});
}
// This line tells Cinder to actually create the application
CINDER_APP_BASIC( AnimationApp, RendererGl(RendererGl::AA_MSAA_4) )
