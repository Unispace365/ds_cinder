#include <functional>
#include "cinder/Vector.h"
#include "ds/ui/sprite/shader/sprite_shader.h"

namespace ds {
	namespace ui {
		class Sprite;
		struct TouchInfo;
		struct TapInfo;
		struct DragDestinationInfo;
	}

	typedef std::function< void (ds::ui::Sprite *, const ds::ui::TouchInfo &)           > TouchInfoCallbackFn;
	typedef std::function< void (ds::ui::Sprite *, const cinder::Vec3f &)               > TapCallbackFn;
	typedef std::function< void (ds::ui::Sprite *, const cinder::Vec3f &)               > DoubleTapCallbackFn;
	typedef std::function< bool (ds::ui::Sprite *, const ds::ui::TapInfo &)             > TapInfoCallbackFn;
	typedef std::function< void (ds::ui::Sprite *, const cinder::Vec3f &)               > SwipeCallbackFn;
	typedef std::function< void (ds::ui::Sprite *, const ds::ui::DragDestinationInfo &) > DragDestinationInfoCallbackFn;
	typedef std::function< void (ds::ui::Sprite *, ds::ui::SpriteShader &)              > BindShaderCallbackFn;

	class TouchInfoCallback {
	public:
		virtual ~TouchInfoCallback();
		virtual void onTouchInfo( ds::ui::Sprite *s, const ds::ui::TouchInfo &ti );
		TouchInfoCallbackFn func();
	};

	class TapCallback {
	public:
		virtual ~TapCallback();
		virtual void onTap( ds::ui::Sprite *s, const cinder::Vec3f &v );
		TapCallbackFn func();
	};

	class DoubleTapCallback {
	public:
		virtual ~DoubleTapCallback();
		virtual void onDoubleTap( ds::ui::Sprite *s, const cinder::Vec3f &v );
		DoubleTapCallbackFn func();
	};

	class TapInfoCallback {
	public:
		virtual ~TapInfoCallback();
		virtual bool onTapInfo( ds::ui::Sprite *s, const ds::ui::TapInfo & );
		TapInfoCallbackFn func();
	};

	class SwipeCallback {
	public:
		virtual ~SwipeCallback();
		virtual void onSwipe( ds::ui::Sprite *s, const cinder::Vec3f &v );
		SwipeCallbackFn func();
	};

	class DragDestinationInfoCallback {
	public:
		virtual ~DragDestinationInfoCallback();
		virtual void onDragDestinationInfo( ds::ui::Sprite *s, const ds::ui::DragDestinationInfo & );
		DragDestinationInfoCallbackFn func();
	};

	class BindShaderCallback {
	public:
		virtual ~BindShaderCallback();
		virtual void onBindShader( ds::ui::Sprite *s, ds::ui::SpriteShader & );
		BindShaderCallbackFn func();
	};
}
/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */
