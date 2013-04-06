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
%include "cinder/app/App.h"
%include "cinder/app/AppBasic.h"

%include "cinder/CinderMath.h"
%ignore NaN;
%include "cinder/Vector.h"
%template(Vec2f) cinder::Vec2<float>;

%include "ds/ui/tween/sprite_anim.h"
%include "ds/ui/sprite/sprite.h"
%include "ds/ui/sprite/image.h"
%include "ds/ui/touch/multi_touch_constraints.h"

%include "ds/app/app.h"
%include "run_app.h"


