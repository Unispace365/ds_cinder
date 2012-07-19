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

}

class AnimationApp : public ds::App {
  public:
    AnimationApp();

    void				      setupServer();
    void				      mouseDown( MouseEvent event );

    Anim<Vec2f>			  mBlackPos, mWhitePos;
    Anim<PosAnim>     mSprite2Pos;

  private:
    typedef ds::App   inherited;
    ds::ui::Image&    mSprite1;
    ds::ui::Image&    mSprite2;
    ds::ui::Image&    mSprite3;
};

AnimationApp::AnimationApp()
    : mSprite1(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
    , mSprite2(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
    , mSprite3(*(new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "this_is_real.jpg"))))
{
}

void AnimationApp::setupServer()
{
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

  mSprite3.enable(false);
  mSprite3.setPosition(200, 400);
  mSprite3.setScale(0.25f, 0.25f);
  rootSprite.addChild(mSprite3);
}

void AnimationApp::mouseDown( MouseEvent event )
{
  inherited::mouseDown(event);
#if 0
  // the call to apply() replaces any existing tweens on mBlackPos with this new one
	auto ans = timeline().apply( &mBlackPos, (Vec2f)event.getPos(), 2.0f, EaseOutQuint() );
  ds::ui::Image*      up3 = &mSprite3;
  const Vec2f*        pos3 = mBlackPos.ptr();
  ans.updateFn([up3, pos3](){up3->setPosition(Vec3f(pos3->x, pos3->y, 0));});
  ans.finishFn([this](){this->mBlackPos.stop();});
#endif

#if 0
  ci::Vec3f   start(0, 0, 0);
  ci::Vec3f   end(400, 200, 0);
  ds::ui::Image*      img = &mSprite1;
  timeline().applyFn<ci::Vec3f>([img](ci::Vec3f pos){img->setPosition(pos);}, start, end, 2.0);
#endif

  ci::Vec3f       end3(ci::Vec3f(float(event.getPos().x), float(event.getPos().y), 0));
//  	auto ans = mTimeline.apply(anim, start, end, duration, cinder::EaseOutQuint() );

  mEngine.getTweenline().addVec3f(mSprite2, mSprite2.getPosition(), end3, ds::ui::Tweenline::POSITION(), 3.0f);
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( AnimationApp, RendererGl )
