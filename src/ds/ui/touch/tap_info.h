#pragma once
#ifndef DS_UI_TOUCH_TAPINFO_H_
#define DS_UI_TOUCH_TAPINFO_H_

#include <cinder/Vector.h>

namespace ds { namespace ui {

	class Sprite;

	struct TapInfo {
		enum State { Waiting, Tapped, Done, Null };

		State	 mState;
		int		 mCount;
		ci::vec3 mCurrentGlobalPoint;
	};

}} // namespace ds::ui

#endif // DS_UI_TOUCH_TAPINFO_H_
