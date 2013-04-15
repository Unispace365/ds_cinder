%module (directors="1") ds_cinder_swig
%feature("autodoc","1");

%include "nice_exception.i"
%include "std_string.i"
%include "std_vector.i"

%{
// Disable float -> integer conversion (possible loss of data) warning
#pragma warning(push)
#pragma warning(disable: 4244)
#include "ds/app/app.h"
#pragma warning(pop)

#define CINDER_MSW

#include "cinder/app/KeyEvent.h"
#include "ds/ui/tween/sprite_anim.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/text_layout.h"
#include "ds/ui/sprite/text.h"
#include "ds/ui/sprite/util/blend.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include "ds/ui/sprite/dirty_state.h"
#include "ds/ui/touch/multi_touch_constraints.h"
#include "ds/ui/touch/tap_info.h"
#include "ds/ui/touch/touch_info.h"
#include "ds/ui/touch/drag_destination_info.h"
#include "ds/util/bit_mask.h"

#include "run_app.h"
#include "callbacks.h"
%}
%ignore prepareSettings;

//%feature("director") cinder::app::App;
%feature("director") cinder::app::AppBasic;
%feature("director") ds::App;
%feature("director") ds::ui::Sprite;

%import "ds/ui/sprite/shader/sprite_shader.h"
%import "ds/ui/sprite/util/blend.h"
%import "ds/ui/sprite/dirty_state.h"
%import "ds/util/bit_mask.h"

%include "cinder/app/Event.h"
%include "cinder/app/KeyEvent.h"
%include "cinder/app/MouseEvent.h"

%ignore getSettings() const;
%ignore getSettings;
%ignore keyDown;
%ignore keyUp;
%ignore "resize";
%ignore "fileDrop";
namespace cinder {
	class Exception :public std::exception {
	};
}
%ignore ResourceLoadExc;
%ignore AssetLoadExc;

%rename(Cinder_App) cinder::app::App;
%rename(Cinder_AppBasic) cinder::app::AppBasic;

using namespace cinder;
using namespace cinder::app;
namespace ci = cinder;

// Suppress SWIG warning about cinder::app::App*::Settings inner classes
#pragma SWIG nowarn=325
//%include "cinder/app/App.h"
//%include "cinder/app/AppBasic.h"
namespace cinder { namespace app { 
class App {
	public:
	class Settings { };
	virtual int			getWindowWidth() const = 0;
	virtual void		setWindowWidth( int windowWidth ) = 0;
	virtual int			getWindowHeight() const = 0;
	virtual void		setWindowHeight( int windowHeight ) = 0;
	virtual void		setWindowSize( int windowWidth, int windowHeight ) = 0;
	Vec2f				getWindowCenter() const { return Vec2f( (float)getWindowWidth(), (float)getWindowHeight() ) * 0.5f; }
	Vec2i				getWindowSize() const { return Vec2i( getWindowWidth(), getWindowHeight() ); }
	float				getWindowAspectRatio() const { return getWindowWidth() / (float)getWindowHeight(); }
	Area				getWindowBounds() const { return Area( 0, 0, getWindowWidth(), getWindowHeight() ); }

	virtual Vec2i		getWindowPos() const { return Vec2i::zero(); }
	int         		getWindowPosX() const { return getWindowPos().x; }
	int         		getWindowPosY() const { return getWindowPos().y; }
	void        		setWindowPos( int x, int y ) { setWindowPos( Vec2i( x, y ) ); }
	virtual void        setWindowPos( const Vec2i &windowPos ) {}

	virtual float		getFrameRate() const = 0;
	virtual void		setFrameRate( float aFrameRate ) = 0;
	float				getAverageFps() const { return mAverageFps; }
	double				getFpsSampleInterval() const { return mFpsSampleInterval; }
	void				setFpsSampleInterval( double sampleInterval ) { mFpsSampleInterval = sampleInterval; }

	virtual bool		isFullScreen() const = 0;
	virtual void		setFullScreen( bool aFullScreen ) = 0;

	virtual bool		isBorderless() const { return false; }
	virtual void		setBorderless( bool borderless = true ) { }
	virtual bool		isAlwaysOnTop() const { return false; }
	virtual void		setAlwaysOnTop( bool alwaysOnTop = true ) { }

	double				getElapsedSeconds() const { return mTimer.getSeconds(); }
	uint32_t			getElapsedFrames() const { return mFrameCount; }

	virtual ~App();
};
class AppBasic : public cinder::app::App {
	public:
	class Settings : public cinder::app::App::Settings { };
	virtual int			getWindowWidth() const;
	virtual void		setWindowWidth( int windowWidth );
	virtual int			getWindowHeight() const;
	virtual void		setWindowHeight( int windowHeight );
	virtual void		setWindowSize( int windowWidth, int windowHeight );
	virtual bool		isFullScreen() const;
	virtual void		setFullScreen( bool fullScreen );
	virtual ~AppBasic();
};

} } //namespace app, cinder


//----------------------------------
// Cinder Tweening/Easing functions
//----------------------------------
%include "cinder/Easing.h"
typedef std::function<float (float)> EaseFn;

namespace std {
	template<class T>
	class function {
	public:
		~function();
	};
}
%template(CinderEaseFn) std::function< float (float) >;
%template(DsTapCallbackFn)                 std::function< void (ds::ui::Sprite *, const cinder::Vec3f &)               >;
%template(DsTapInfoCallbackFn)             std::function< bool (ds::ui::Sprite *, const ds::ui::TapInfo &)             >;
%template(DsDragDestinationInfoCallbackFn) std::function< void (ds::ui::Sprite *, const ds::ui::DragDestinationInfo &) >;

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

// Cinder vector classes
%ignore NaN;
%ignore cinder::Vec2::operator[];
%ignore cinder::Vec3::operator[];
%ignore cinder::Vec4::operator[];
%ignore cinder::Vec2::operator=;
%ignore cinder::Vec3::operator=;
%ignore cinder::Vec4::operator=;
%ignore operator<<;
%ignore operator<<;
%ignore operator<<;
%include "cinder/Vector.h"
%template(Vec2i) cinder::Vec2<int>;
%template(Vec2f) cinder::Vec2<float>;
%template(Vec3f) cinder::Vec3<float>;

%include "ds/ui/tween/tweenline.h"
%include "ds/ui/tween/sprite_anim.h"
//%import "ds/ui/sprite/sprite_engine.h"
namespace ds { namespace ui {
	class SpriteEngine {
	public:
	virtual float                  getWorldWidth() const = 0;
	virtual float                  getWorldHeight() const = 0;

	protected:
		virtual ~SpriteEngine();
	};
} }

%include "ds/ui/sprite/sprite.h"
%include "ds/ui/sprite/image.h"
%include "ds/ui/sprite/text_layout.h"
%include "ds/ui/sprite/text.h"
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
namespace std {
	%template(SpriteVector) vector< ds::ui::Sprite *>;
}

%feature("director") ds::TapCallback;
%feature("director") ds::DoubleTapCallback;
%feature("director") ds::TapInfoCallback;
%feature("director") ds::SwipeCallback;
%feature("director") ds::DragDestinationInfoCallback;

%include "callbacks.h"

/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */

