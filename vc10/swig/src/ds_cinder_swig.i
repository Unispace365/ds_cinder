%module (directors="1") ds_cinder_swig
%feature("autodoc","1");
%include "std_string.i"

%{
#include "cinder/app/App.h"
#include "cinder/app/AppBasic.h"
#include "cinder/app/KeyEvent.h"

#include "ds/app/app.h"
#include "ds/ui/tween/sprite_anim.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/util/blend.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include "ds/ui/sprite/dirty_state.h"
#include "ds/ui/touch/multi_touch_constraints.h"
#include "ds/util/bit_mask.h"
#include "run_app.h"
%}

%import "ds/ui/sprite/shader/sprite_shader.h"
%import "ds/ui/sprite/util/blend.h"
%import "ds/ui/sprite/dirty_state.h"
%import "ds/util/bit_mask.h"

%import "cinder/app/KeyEvent.h"

%ignore prepareSettings;
%ignore "getSettings() const";
%ignore getSettings;
%ignore keyDown;
%ignore keyUp;

%ignore "resize";
%ignore "fileDrop";
%ignore "ResourceLoadExc";

%rename(Cinder_App) cinder::app::App;
%rename(Cinder_AppBasic) cinder::app::AppBasic;
%feature("director") cinder::app::App;
%feature("director") cinder::app::AppBasic;
%feature("director") ds::App;
%feature("director") ds::ui::Sprite;

using namespace cinder;
using namespace cinder::app;

#define CINDER_MSW
%include "cinder/app/MouseEvent.h"
%include "cinder/app/App.h"
%include "cinder/app/AppBasic.h"

//----------------------------------
// Cinder Tweening/Easing functions
//----------------------------------
%include "cinder/Easing.h"
%ignore cinder::TweenBase;
%include "cinder/Tween.h"
namespace std {
  template<class T>
  class function {
    public:
      ~function();
  };
}
%template(CinderEaseFn) std::function< float (float) >;
// Add an EaseFn conversion method to Ease functors that can be passed parameters
%extend cinder::EaseInBounce { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInBounce { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutBounce { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutBounce { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInBounce { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInBack { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutBack { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutBack { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInBack { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInAtan { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutAtan { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutAtan { cinder::EaseFn func() { return *$self; } }

%include "cinder/CinderMath.h"
%ignore NaN;
%include "cinder/Vector.h"
%template(Vec2i) cinder::Vec2<int>;
%template(Vec2f) cinder::Vec2<float>;
%template(Vec3f) cinder::Vec3<float>;

namespace ci = cinder;
%include "ds/ui/tween/tweenline.h"
%include "ds/ui/tween/sprite_anim.h"
%include "ds/ui/sprite/sprite.h"
%include "ds/ui/sprite/image.h"
%include "ds/ui/touch/multi_touch_constraints.h"

%extend ds::ui::Sprite {
    void tweenPositionTo( const Vec3f &pos, float d, cinder::EaseFn easeFunction = cinder::easeNone ) {
        $self->getEngine().getTweenline().apply(
            *$self,
            $self->ANIM_POSITION(),
            pos, d,
            easeFunction
        );
    }

    void tweenScaleTo( const Vec3f &pos, float d, cinder::EaseFn easeFunction = cinder::easeNone ) {
        $self->getEngine().getTweenline().apply(
            *$self,
            $self->ANIM_SCALE(),
            pos, d,
            easeFunction
        );
    }

    void tweenRotationTo( const Vec3f &rot, float d, cinder::EaseFn easeFunction = cinder::easeNone ) {
        $self->getEngine().getTweenline().apply(
            *$self,
            $self->ANIM_ROTATION(),
            rot, d,
            easeFunction
        );
    }
}

%include "ds/app/engine.h"
%include "ds/app/app.h"
%include "run_app.h"


