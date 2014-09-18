#ifndef _DS_UI_SPRITE_TWEEN_EVENT_H_
#define _DS_UI_SPRITE_TWEEN_EVENT_H_

#include <ds/app/event.h>

/*
* This is a macro that forms a declaration for a Tween Event.
* Tween event are in form of TweenEvent[... name ...]
*/

#define DeclareTweenEvent(name) \
class TweenEvent##name : public ds::Event { \
	public: \
	static int				WHAT(); \
	const std::string&		key; \
	TweenEvent##name(const std::string& aKey); \
};

namespace ds {
namespace ui {

DeclareTweenEvent(Added);
DeclareTweenEvent(Start);
DeclareTweenEvent(Change);
DeclareTweenEvent(End);
DeclareTweenEvent(Removed);
	
} //!namespace ui
} //!namespace ds

#undef DeclareTweenEvent

#endif //!_DS_UI_SPRITE_TWEEN_EVENT_H_