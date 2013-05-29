%module (directors="1") ds_cinder
%feature("autodoc","1");

%include "nice_exception.i"
%include "std_string.i"
%include "std_wstring.i"
%include "std_vector.i"

%{
// Disable float -> integer conversion (possible loss of data) warning
#pragma warning(push)
#pragma warning(disable: 4244)
#include "ds/app/app.h"
#pragma warning(pop)

#define CINDER_MSW

#include "cinder/app/KeyEvent.h"
#include "cinder/Timeline.h"
#include "cinder/Color.h"
#include "cinder/gl/GlslProg.h"
#include "ds/ui/tween/sprite_anim.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/text_layout.h"
#include "ds/ui/sprite/text.h"
#include "ds/ui/sprite/multiline_text.h"
#include "ds/ui/sprite/video.h"
#include "ds/ui/sprite/util/blend.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include "ds/ui/sprite/dirty_state.h"
#include "ds/ui/touch/multi_touch_constraints.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/touch/tap_info.h"
#include "ds/ui/touch/touch_info.h"
#include "ds/ui/touch/drag_destination_info.h"
#include "ds/util/bit_mask.h"

#include "run_app.h"
#include "callbacks.h"
#include "frustum_sprite.h"
%}
%ignore prepareSettings;

//%feature("director") cinder::app::App;
%feature("director") cinder::app::AppBasic;
%feature("nodirector") cinder::app::App::getWindowHeight;
%feature("nodirector") cinder::app::App::getWindowWidth;
%feature("nodirector") cinder::app::AppBasic::getWindowHeight;
%feature("nodirector") cinder::app::AppBasic::getWindowWidth;
%feature("director") ds::App;
%feature("nodirector") ds::App::touchesBegan;
%feature("nodirector") ds::App::touchesMoved;
%feature("nodirector") ds::App::touchesEnded;
%feature("nodirector") ds::Engine::touchesBegin;
%feature("nodirector") ds::Engine::touchesMoved;
%feature("nodirector") ds::Engine::touchesEnded;

%feature("director") ds::ui::Sprite;
%feature("director") ds::ui::ShaderSprite;
%feature("nodirector") ds::ui::Sprite::drawClient;
%feature("nodirector") ds::ui::Sprite::drawServer;
%feature("nodirector") ds::ui::Sprite::updateClient;
%feature("nodirector") ds::ui::Sprite::drawLocalClient;
//%feature("nodirector") ds::ui::Sprite::updateServer;

%import "ds/ui/sprite/util/blend.h"
%import "ds/ui/sprite/dirty_state.h"
%include "ds/util/bit_mask.h"

%include "cinder/app/Event.h"
%include "cinder/app/KeyEvent.h"
%include "cinder/app/MouseEvent.h"
%include "ds/ui/sprite/shader/sprite_shader.h"

%ignore cinder::ColorT::operator=;
%ignore cinder::ColorT::operator[];
%ignore cinder::ColorAT::operator=;
%ignore cinder::ColorAT::operator[];
%rename(to_float_ptr) "cinder::ColorT::operator T*";
%rename(to_const_float_ptr) "cinder::ColorT::operator const T*";
%include "stdint.i"
%include "cinder/Color.h"
%template(CinderColor) cinder::ColorT<float>;

%ignore getSettings() const;
%ignore getSettings;
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
	virtual void		setFrameRate( float frameRate );
};

} } //namespace app, cinder

%rename(to_unspecified_bool_type) "cinder::gl::GlslProg::operator unspecified_bool_type";
%include "cinder/gl/GlslProg.h"

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

%template(DsTouchInfoCallbackFn)           std::function< void (ds::ui::Sprite *, const ds::ui::TouchInfo &)           >;
%template(DsTapCallbackFn)                 std::function< void (ds::ui::Sprite *, const cinder::Vec3f &)               >;
%template(DsTapInfoCallbackFn)             std::function< bool (ds::ui::Sprite *, const ds::ui::TapInfo &)             >;
%template(DsDragDestinationInfoCallbackFn) std::function< void (ds::ui::Sprite *, const ds::ui::DragDestinationInfo &) >;




// Add an EaseFn conversion method to Ease functors that can be passed parameters


%extend cinder::EaseNone         { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInQuad       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutQuad      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutQuad    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInQuad    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInCubic      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutCubic     { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutCubic   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInCubic   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInQuart      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutQuart     { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutQuart   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInQuart   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInQuint      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutQuint     { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutQuint   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInQuint   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInSine       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutSine      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutSine    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInSine    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInExpo       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutExpo      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutExpo    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInExpo    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInCirc       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutCirc      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutCirc    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInCirc    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInBounce     { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInBounce     { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutBounce    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutBounce  { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInBounce  { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInBack       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutBack      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutBack    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInBack    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInElastic    { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutElastic   { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutInElastic { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInAtan       { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseOutAtan      { cinder::EaseFn func() { return *$self; } }
%extend cinder::EaseInOutAtan    { cinder::EaseFn func() { return *$self; } }

// Cinder vector classes
%ignore NaN;
%ignore cinder::Vec2::operator[];
%ignore cinder::Vec3::operator[];
%ignore cinder::Vec4::operator[];
%ignore cinder::Vec2::operator=;
%ignore cinder::Vec3::operator=;
%ignore cinder::Vec4::operator=;
%ignore operator<<;
%include "cinder/Vector.h"
%template(Vec2i) cinder::Vec2<int>;
%template(Vec2f) cinder::Vec2<float>;
%template(Vec3f) cinder::Vec3<float>;

%pythoncode %{
def vec3f_str(self):
	return "Vec3f[ %6.2f %6.2f %6.2f ]" % ( self.x, self.y, self.z )
Vec3f.__str__ = vec3f_str
Vec3f.__repr__ = vec3f_str

def vec2f_str(self):
	return "Vec2f[ %6.2f %6.2f ]" % ( self.x, self.y )
Vec2f.__str__ = vec2f_str
Vec2f.__repr__ = vec2f_str

def vec2i_str(self):
	return "Vec2i[ %5d %5d ]" % ( self.x, self.y )
Vec2i.__str__ = vec2i_str
Vec2i.__repr__ = vec2i_str
%}


%include "ds/ui/tween/tweenline.h"
%include "ds/ui/tween/sprite_anim.h"
//%import "ds/ui/sprite/sprite_engine.h"
namespace ds { namespace ui {
	class SpriteEngine {
	public:
	virtual float                  getWorldWidth() const = 0;
	virtual float                  getWorldHeight() const = 0;
    void                           addToDragDestinationList(Sprite *sprite);
    void                           removeFromDragDestinationList(Sprite *sprite);
    virtual ds::ui::LoadImageService &getLoadImageService() = 0;
    double                         getElapsedTimeSeconds() const;

	protected:
		virtual ~SpriteEngine();
	};
} }

%include "ds/ui/sprite/sprite.h"
%include "ds/ui/sprite/image.h"
%include "ds/ui/sprite/text_layout.h"
%include "ds/ui/sprite/text.h"
%include "ds/ui/sprite/multiline_text.h"
%include "ds/ui/sprite/video.h"
%include "ds/ui/sprite/shader/sprite_shader.h"
%include "ds/ui/touch/multi_touch_constraints.h"

//%import "ds/thread/gl_thread.h"
namespace ds {
	class GlThread;

	template <class T>
	class GlThreadClient {
	public:
		GlThreadClient(GlThread&);
		//bool performOnWorkerThread(void(T::*callerMethod)(), const bool batch = false);
	};

	namespace ui {
		class LoadImageService;
	}

	//%ignore performOnWorkerThread;
	//%ignore ds::GlThreadClient<ds::ui::LoadImageService>::performOnWorkerThread;
	%template(DsGlThreadClientLoadImageService) ds::GlThreadClient<ds::ui::LoadImageService>;
	%nodefaultctor DsGlThreadClientLoadImageService;
	%nodefaultdtor DsGlThreadClientLoadImageService;

/*
	namespace ui {
		%nodefaultctor LoadImageService;
		%nodefaultdtor LoadImageService;
		class LoadImageService : public ds::GlThreadClient<LoadImageService> {
		 public:
			// Clients should call release() for every successful acquire
			bool					acquire(const std::string& filename, const int flags);
			bool					peekToken(const std::string& filename, int* flags = nullptr) const;
			void					clear();
		};
	}
*/
}
%include "ds/ui/service/load_image_service.h"

%include "ds/ui/touch/touch_info.h"
%include "ds/ui/touch/tap_info.h"
%include "ds/ui/touch/drag_destination_info.h"

%extend ds::ui::Sprite {
	void tweenPositionTo( 
			const Vec3f &pos,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_POSITION(),
			pos, d, easeFunction, nullptr, delay );
	}
	void tweenPositionTo( 
			float x, float y,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		Vec3f pos( x, y, $self->getPosition().z );
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_POSITION(),
			pos, d, easeFunction, nullptr, delay );
	}

	void tweenScaleTo( 
			const Vec3f &scale, 
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_SCALE(),
			scale, d, easeFunction, nullptr, delay );
	}

	void tweenScaleTo( 
			float sx, float sy,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		Vec3f scale( sx, sy, $self->getScale().z );
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_SCALE(),
			scale, d, easeFunction, nullptr, delay );
	}

	void tweenRotationTo(
			const Vec3f &rot,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_ROTATION(),
			rot, d, easeFunction, nullptr, delay );
	}

	void tweenSizeTo(
			const Vec3f &size,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_SIZE(),
			size, d, easeFunction, nullptr, delay );
	}

	void tweenSizeTo( 
			float w, float h,
			float d, 
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		Vec3f size( w, h, $self->getDepth() );
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_SIZE(),
			size, d, easeFunction, nullptr, delay );
	}

	void tweenOpacityTo( 
			float o,
			float d,
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {
		$self->getEngine().getTweenline().apply(
			*$self,
			$self->ANIM_OPACITY(),
			o, d, easeFunction, nullptr, delay );
	}

	void tweenColorTo( 
			const Vec3f &color,
			float d,
			cinder::EaseFn easeFunction = cinder::easeNone, 
			float delay = 0.0f ) {

		static std::map< ds::ui::Sprite*, cinder::Anim<cinder::Vec3f> > animMap;
		auto i = animMap.find( $self );
		if ( i == animMap.end() )
			animMap.insert( std::make_pair( $self, cinder::Anim<cinder::Vec3f>() ) );
		auto& anim = animMap[$self];

		auto& timeline = $self->getEngine().getTweenline().getTimeline();
		auto start_color = $self->getColor();
		Vec3f start( start_color.r, start_color.g, start_color.b );

		auto ans = timeline.apply( &anim, start, color, d, easeFunction );
		ds::ui::Sprite* s_ptr = $self;
		const Vec3f* value_ptr = anim.ptr();
		ans.updateFn( [s_ptr, value_ptr](){ s_ptr->setColor( value_ptr->x, value_ptr->y, value_ptr->z ); } );
		ans.delay(delay);
	}

}

%include "ds/app/engine.h"
%include "ds/app/app.h"
%include "ds/params/update_params.h"
%include "run_app.h"
namespace std {
	%template(SpriteVector) vector< ds::ui::Sprite *>;
	%template(IntVector)    vector< int >;
}

%feature("director") ds::TouchInfoCallback;
%feature("director") ds::TapCallback;
%feature("director") ds::DoubleTapCallback;
%feature("director") ds::TapInfoCallback;
%feature("director") ds::SwipeCallback;
%feature("director") ds::DragDestinationInfoCallback;

%include "callbacks.h"
%include "frustum_sprite.h"

/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */

