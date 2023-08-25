#pragma once
#ifndef DS_UI_DRAG_DESTINATION_INFO_H
#define DS_UI_DRAG_DESTINATION_INFO_H
#include "cinder/Vector.h"

namespace ds { namespace ui {

	class Sprite;

	struct DragDestinationInfo {
		enum Phase { Entered, Updated, Exited, Released, Null };

		ci::vec3 mCurrentPoint;
		Phase	 mPhase;
		Sprite*	 mSprite;
	};

}} // namespace ds::ui

#endif // DS_UI_DRAG_DESTINATION_INFO_H
