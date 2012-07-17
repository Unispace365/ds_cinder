#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>
#include <Poco/Path.h>

#include <ds/app/app.h>
#include <ds/app/environment.h>
#include <ds/data/resource_list.h>
#include <ds/thread/runnable_client.h>
#include <ds/query/query_client.h>
#include <ds/config/settings.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/ui/sprite/image.h>

using namespace std;
using namespace ci;
using namespace ci::app;

namespace {
class PosAnim {
  public:
    PosAnim() : mTarget(nullptr) {
    }
    PosAnim(ds::ui::Sprite* t, const ci::Vec3f& pos) : mTarget(t), mPos(pos) {
    }

    ds::ui::Sprite*     mTarget;
    ci::Vec3f           mPos;

	  const PosAnim       operator*( const PosAnim& rhs ) const { return PosAnim(mTarget, ci::Vec3f(mPos.x * rhs.mPos.x, mPos.y * rhs.mPos.y, mPos.z * rhs.mPos.z)); }
	  const PosAnim       operator+( const PosAnim& rhs ) const { return PosAnim(mTarget, ci::Vec3f(mPos.x + rhs.mPos.x, mPos.y + rhs.mPos.y, mPos.z + rhs.mPos.z)); }
	  const PosAnim       operator*( const float v ) const { return PosAnim(mTarget, ci::Vec3f(mPos.x * v, mPos.y * v, mPos.z * v)); }

    void                operator=(const PosAnim& rhs) { if (mTarget) mTarget->setPosition(rhs.mPos); mPos = rhs.mPos; }
};

#if 0
class PosAnim2 : public ci::Anim<ci::Vec3f> {
  public:
    PosAnim2(ds::ui::Sprite& s) : inherited(&mTarget, ), mSprite(s) {
    }

    ds::ui::Sprite      &mSprite;
    ci::Vec3f           mTarget;

  private:
    typedef ci::Anim<ci::Vec3f>   inherited;
};
#endif

}

class AnimationApp : public ds::App {
  public:
    AnimationApp();

    void				      setup();
    void				      mouseDown( MouseEvent event );

    Anim<Vec2f>			  mBlackPos, mWhitePos;
    Anim<PosAnim>     mSprite2Pos;

  private:
    typedef ds::App   inherited;
    ds::ui::Image&    mSprite1;
    ds::ui::Image&    mSprite2;
};

AnimationApp::AnimationApp()
    : mSprite1(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
    , mSprite2(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
{
//  mBlackPos.setUpd
}

void AnimationApp::setup()
{
  inherited::setup();

  mBlackPos = mWhitePos = getWindowCenter();
  mSprite2Pos.ptr()->mTarget = &mSprite2;
  mSprite2Pos.ptr()->mPos = ci::Vec3f(400, 400, 0);

  ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

  mSprite1.enable(false);
  mSprite1.setPosition(0, 0);
  rootSprite.addChild(mSprite1);

  mSprite2.enable(false);
  mSprite2.setPosition(400, 400);
  mSprite2.setScale(0.5f, 0.5f);
  rootSprite.addChild(mSprite2);
}

void AnimationApp::mouseDown( MouseEvent event )
{
  inherited::mouseDown(event);

	// the call to apply() replaces any existing tweens on mBlackPos with this new one
	timeline().apply( &mBlackPos, (Vec2f)event.getPos(), 2.0f, EaseInCubic() );
	// the call to appendTo causes the white circle to start when the black one finishes
	timeline().apply( &mWhitePos, (Vec2f)event.getPos(), 0.35f, EaseOutQuint() ).appendTo( &mBlackPos );

  ci::Vec3f   start(0, 0, 0);
  ci::Vec3f   end(400, 200, 0);
  ds::ui::Image*      img = &mSprite1;
  timeline().applyFn<ci::Vec3f>([img](ci::Vec3f pos){img->setPosition(pos);}, start, end, 2.0);


  PosAnim       anim(&mSprite2, ci::Vec3f(float(event.getPos().x), float(event.getPos().y), 0));
  timeline().apply( &mSprite2Pos, anim, 2.0f, EaseInCubic() );

}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( AnimationApp, RendererGl )
