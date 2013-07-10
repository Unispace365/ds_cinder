#include "callbacks.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/touch/touch_info.h"
#include "ds/ui/touch/tap_info.h"
#include "ds/ui/touch/drag_destination_info.h"

namespace ds {

TouchInfoCallback::~TouchInfoCallback() {}
void TouchInfoCallback::onTouchInfo( ds::ui::Sprite *s, const ds::ui::TouchInfo &ti ) {}
TouchInfoCallbackFn TouchInfoCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ds::ui::TouchInfo &ti) {
			this->onTouchInfo( s, ti );
	});
}

TapCallback::~TapCallback() {}
void TapCallback::onTap( ds::ui::Sprite *s, const cinder::Vec3f &v ) {}
TapCallbackFn TapCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ci::Vec3f &v) {
			this->onTap( s, v );
	});
}


DoubleTapCallback::~DoubleTapCallback() {}
void DoubleTapCallback::onDoubleTap( ds::ui::Sprite *s, const cinder::Vec3f &v ) {}
DoubleTapCallbackFn DoubleTapCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ci::Vec3f &v) {
			this->onDoubleTap( s, v );
	});
}


TapInfoCallback::~TapInfoCallback() {} 
bool TapInfoCallback::onTapInfo( ds::ui::Sprite *s, const ds::ui::TapInfo &ti ) { return false; } 
TapInfoCallbackFn TapInfoCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ds::ui::TapInfo &ti ) {
			return this->onTapInfo( s, ti );
	});
}


SwipeCallback::~SwipeCallback() {} 
void SwipeCallback::onSwipe( ds::ui::Sprite *s, const cinder::Vec3f &v ) {} 
SwipeCallbackFn SwipeCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ci::Vec3f &v) {
			this->onSwipe( s, v );
	});
}


DragDestinationInfoCallback::~DragDestinationInfoCallback() {} 
void DragDestinationInfoCallback::onDragDestinationInfo( ds::ui::Sprite *s, const ds::ui::DragDestinationInfo &ddi ) {}
DragDestinationInfoCallbackFn DragDestinationInfoCallback::func() {
	return ( [this](ds::ui::Sprite *s, const ds::ui::DragDestinationInfo &ddi ) {
			this->onDragDestinationInfo( s, ddi );
	});
}

BindShaderCallback::~BindShaderCallback() {}
void BindShaderCallback::onBindShader( ds::ui::Sprite *s, ds::ui::SpriteShader &ss ) {}
BindShaderCallbackFn BindShaderCallback::func() {
	return ( [this](ds::ui::Sprite *s, ds::ui::SpriteShader &ss ) {
			this->onBindShader( s, ss );
	});
}

} //namespace ds

/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */

