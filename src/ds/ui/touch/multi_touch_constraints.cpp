#include "multi_touch_constraints.h"

namespace {

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

} // namespace ui
} // namespace ds
