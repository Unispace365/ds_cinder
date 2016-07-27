#include "multi_touch_constraints.h"

#include <boost/algorithm/string.hpp>

namespace {

	/** \cond Ignore this file for doxygen as it creates a bunch of duplicates */
ds::BitMask newUniqueBitMask()
{
  static unsigned category = 0;

  return ds::BitMask(category++);
}

ds::BitMask MULTITOUCH_INFO_ONLY_ = newUniqueBitMask();
ds::BitMask MULTITOUCH_CAN_SCALE_ = newUniqueBitMask();
ds::BitMask MULTITOUCH_CAN_ROTATE_ = newUniqueBitMask();
ds::BitMask MULTITOUCH_CAN_POSITION_X_ = newUniqueBitMask();
ds::BitMask MULTITOUCH_CAN_POSITION_Y_ = newUniqueBitMask();
ds::BitMask MULTITOUCH_CAN_POSITION_ = MULTITOUCH_CAN_POSITION_X_ | MULTITOUCH_CAN_POSITION_Y_;
ds::BitMask MULTITOUCH_NO_CONSTRAINTS_ = MULTITOUCH_CAN_SCALE_ | MULTITOUCH_CAN_ROTATE_ | MULTITOUCH_CAN_POSITION_;

}

namespace ds {
namespace ui {

const BitMask &MULTITOUCH_INFO_ONLY = MULTITOUCH_INFO_ONLY_;
const BitMask &MULTITOUCH_CAN_SCALE = MULTITOUCH_CAN_SCALE_;
const BitMask &MULTITOUCH_CAN_ROTATE = MULTITOUCH_CAN_ROTATE_;
const BitMask &MULTITOUCH_CAN_POSITION_X = MULTITOUCH_CAN_POSITION_X_;
const BitMask &MULTITOUCH_CAN_POSITION_Y = MULTITOUCH_CAN_POSITION_Y_;
const BitMask &MULTITOUCH_CAN_POSITION = MULTITOUCH_CAN_POSITION_;
const BitMask &MULTITOUCH_NO_CONSTRAINTS = MULTITOUCH_NO_CONSTRAINTS_;


// TODO: add the rest of the permutations, if you want em
const ds::BitMask parseMultitouchMode(const std::string& s){
	if(boost::iequals(s, "info"))				return MULTITOUCH_INFO_ONLY;
	else if(boost::iequals(s, "pos"))			return MULTITOUCH_CAN_POSITION;
	else if(boost::iequals(s, "all"))			return MULTITOUCH_NO_CONSTRAINTS;
	else if(boost::iequals(s, "scale"))			return MULTITOUCH_CAN_SCALE;
	else if(boost::iequals(s, "pos_x"))			return MULTITOUCH_CAN_POSITION_X;
	else if(boost::iequals(s, "pos_y"))			return MULTITOUCH_CAN_POSITION_Y;
	else if(boost::iequals(s, "pos_scale"))		return MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_SCALE;
	else if(boost::iequals(s, "pos_rotate"))	return MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_ROTATE;
	else if(boost::iequals(s, "rotate"))		return MULTITOUCH_CAN_ROTATE;
	return MULTITOUCH_INFO_ONLY;
}


const std::string getMultitouchStringForBitMask(const ds::BitMask& s){
	if(s & MULTITOUCH_INFO_ONLY){
		return "info";
	} else if(s & MULTITOUCH_CAN_POSITION && s & MULTITOUCH_CAN_ROTATE && s & MULTITOUCH_CAN_SCALE){
		return "all";
	} else if(s & MULTITOUCH_CAN_POSITION){
		if(s & MULTITOUCH_CAN_ROTATE){
			return "pos_rotate";
		}
		if(s & MULTITOUCH_CAN_SCALE){
			return "pos_scale";
		}

		if(s & MULTITOUCH_CAN_POSITION_X && s & MULTITOUCH_CAN_POSITION_Y){
			return "pos";
		} else if(s & MULTITOUCH_CAN_POSITION_X){
			return "pos_x";
		} else if(s & MULTITOUCH_CAN_POSITION_Y){
			return "pos_y";
		}
	} else if(s & MULTITOUCH_CAN_ROTATE){
		return "rotate";
	} else if(s & MULTITOUCH_CAN_SCALE){
		return "scale";
	}

	return "info";
}

} // namespace ui
} // namespace ds

/** \endcond */