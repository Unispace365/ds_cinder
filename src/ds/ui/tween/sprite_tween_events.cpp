#include "sprite_tween_events.h"

#ifndef DefineTweenEvent
#define DefineTweenEvent(name) \
static ds::EventRegistry __##name("TweenEvent##name"); \
int TweenEvent##name::WHAT() { return __##name.mWhat; } \
TweenEvent##name::TweenEvent##name(const std::string& aKey) \
	: ds::Event(__##name.mWhat) \
	, key(aKey)\
{}
#endif

namespace ds {
namespace ui {

DefineTweenEvent(Added);
DefineTweenEvent(Start);
DefineTweenEvent(Change);
DefineTweenEvent(End);
DefineTweenEvent(Removed);

} //!namespace ui
} //!namespace ds

#ifdef DefineTweenEvent
#undef DefineTweenEvent
#endif
