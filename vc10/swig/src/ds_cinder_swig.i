%module (directors="1") ds_cinder_swig
%include "std_string.i"

%{
#include "cinder/app/App.h"
#include "cinder/app/AppBasic.h"
#include "cinder/app/KeyEvent.h"
#include "ds/app/app.h"
#include "ds/ui/tween/sprite_anim.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/util/bit_mask.h"
#include "ds/ui/sprite/util/blend.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include "ds/ui/sprite/dirty_state.h"
#include "run_app.h"
#include "ds/ui/touch/multi_touch_constraints.h"
%}

%import "ds/ui/sprite/shader/sprite_shader.h"
%import "ds/ui/sprite/util/blend.h"
%import "ds/util/bit_mask.h"
%import "cinder/app/KeyEvent.h"
%import "ds/ui/sprite/dirty_state.h"

#%import "cinder_app.i"

%ignore prepareSettings;
%ignore "getSettings() const";
%ignore getSettings;
%ignore keyDown;
%ignore keyUp;

%ignore "resize";
%ignore "fileDrop";
%ignore "ResourceLoadExc";

%rename(CiApp) cinder::app::App;

/*
#%feature("nspace", 1);
#%import "cinder/app/AppBasic.h"
#%rename(ci_app_App) ci::app::App;
#%rename(ci_app_AppBasic) ci::app::AppBasic;
#%rename(AppBasic) ci::app::AppBasic;
#%rename (DsApp) App;
#%rename("ci::app::AppBasic") AppBasic;

*/

/*
#%include "cinder/app/AppBasic.h"
*/



/*
%nspace ci::app::AppBasic;
%typedef ci::app::AppBasic AppBasic;

#%rename (CiApp) ci::app::App;
#%feature("director") CiApp;
#%feature("director") cinder::app::App;

#%rename (CiAppBasic) cinder::app::AppBasic;
#%feature("director") CiAppBasic;
#%feature("director") cinder::app::AppBasic;

%feature("director") AppBasic;
%feature("director") App;

*/


/*

namespace fs {
    class path;
}

namespace ci { namespace app {
class App {
    virtual fs::path getAppPath() = 0;
};

class AppBasic : public App {
    virtual fs::path getAppPath();
    virtual void doSomeStuff( int x );
};
} }
%import (module="cinder_app") "cinder/app/App.h"
%import (module="cinder_app") "cinder/app/AppBasic.h"

#%feature("director") DsApp;
*/


%feature("director") ds::App;
%feature("director") cinder::app::AppBasic;
%feature("director") cinder::app::App;

%feature("director") ds::ui::Sprite;

#%feature("nodirector") cinder::app::App;
#%feature("nodirector") cinder::app::AppBasic;
#%feature("nodirector") cinder::app::App::launch;
#%feature("nodirector") App::launch;
#%feature("nodirector") cinder::app::AppBasic::launch;
#%feature("nodirector") launch;

using namespace cinder;
using namespace cinder::app;

#define CINDER_MSW
%include "cinder/app/App.h"
%include "cinder/app/AppBasic.h"

/*
*/

%include "ds/ui/tween/sprite_anim.h"
%include "ds/ui/sprite/sprite.h"

%include "ds/app/app.h"
%include "ds/ui/touch/multi_touch_constraints.h"

%include "run_app.h"

