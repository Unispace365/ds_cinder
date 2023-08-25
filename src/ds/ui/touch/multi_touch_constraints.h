#pragma once
#ifndef DS_UI_MULTI_TOUCH_CONSTRAINTS_H
#define DS_UI_MULTI_TOUCH_CONSTRAINTS_H
#include "ds/util/bit_mask.h"
#include <string>

namespace ds { namespace ui {

	/** \property MULTITOUCH_INFO_ONLY
					Only return information about touch handling in Sprite's setTouchInfoFunction(). Does not modify the
	   sprite in any way when touch hits the Sprite. */
	extern const BitMask& MULTITOUCH_INFO_ONLY;
	extern const BitMask& MULTITOUCH_CAN_SCALE;
	extern const BitMask& MULTITOUCH_CAN_ROTATE;
	extern const BitMask& MULTITOUCH_CAN_POSITION_X;
	extern const BitMask& MULTITOUCH_CAN_POSITION_Y;
	extern const BitMask& MULTITOUCH_CAN_POSITION;
	extern const BitMask& MULTITOUCH_NO_CONSTRAINTS;


	/// Get the bitmask for short string names
	const ds::BitMask parseMultitouchMode(const std::string& s);

	/// Get a short string name for the bitmask supplied
	const std::string getMultitouchStringForBitMask(const ds::BitMask& s);

}} // namespace ds::ui

#endif // DS_UI_MULTI_TOUCH_CONSTRAINTS_H