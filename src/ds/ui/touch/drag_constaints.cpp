#include "drag_constaints.h"


namespace {

ds::BitMask newUniqueBitMask()
{
  static unsigned category = 0;

  return ds::BitMask(category++);
}

ds::BitMask DRAGGING_ENTERED_ = newUniqueBitMask();
ds::BitMask DRAGGING_UPDATED_ = newUniqueBitMask();
ds::BitMask DRAGGING_EXITED_ = newUniqueBitMask();
ds::BitMask DRAGGING_RELEASED_ = newUniqueBitMask();
ds::BitMask DRAGGING_NULL_ = newUniqueBitMask();

}

namespace ds {
namespace ui {

const BitMask &DRAGGING_ENTERED = DRAGGING_ENTERED_;
const BitMask &DRAGGING_UPDATED = DRAGGING_UPDATED_;
const BitMask &DRAGGING_EXITED = DRAGGING_EXITED_;
const BitMask &DRAGGING_RELEASED = DRAGGING_RELEASED_;
const BitMask &DRAGGING_NULL = DRAGGING_NULL_;

} // namespace ui
} // namespace ds
