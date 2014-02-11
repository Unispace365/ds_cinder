#pragma once
#ifndef DS_UI_SPRITE_WEBLISTENER_H_
#define DS_UI_SPRITE_WEBLISTENER_H_

#include <cinder/Vector.h>

namespace ds {
namespace web {

/**
 * \class ds::web::TouchEvent
 * \brief A touch event custom for the web view.
 */
class TouchEvent {
public:
	enum			Phase { kAdded, kMoved, kRemoved };
	TouchEvent();

	Phase			mPhase;
	ci::Vec2f		mPosition;
	ci::Vec2f		mUnitPosition;
};

} // namespace web
} // namespace ds

#endif // DS_UI_SPRITE_WEBLISTENER_H_
