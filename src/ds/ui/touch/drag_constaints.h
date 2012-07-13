#pragma once
#ifndef DS_UI_DRAG_CONSTAINTS_H
#define DS_UI_DRAG_CONSTAINTS_H
#include "ds/util/bit_mask.h"

namespace ds {
namespace ui {

extern const BitMask &DRAGGING_ENTERED;
extern const BitMask &DRAGGING_UPDATED;
extern const BitMask &DRAGGING_EXITED;
extern const BitMask &DRAGGING_RELEASED;
extern const BitMask &DRAGGING_NULL;

} // namespace ui
} // namespace ds

#endif//DS_UI_DRAG_CONSTAINTS_H
