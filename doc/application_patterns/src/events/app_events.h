#pragma once
#ifndef NA_EVENTS_APPEVENTS_H_
#define NA_EVENTS_APPEVENTS_H_

#include <engine_color.h>
#include "na/app/event.h"

namespace na {

/**
 * \class na::WallModeChanged
 * The wall mode has changed, see app/wall_mode.h for the modes.
 */
class WallModeChanged : public Event {
  public:
    static int          WHAT();
    WallModeChanged(const int mode);

    int                 mMode;
};

/**
 * \class na::BackgroundEvent
 * Set the current background color, based on a macro.
 */
class BackgroundEvent : public Event {
  public:
    static int          WHAT();
    static const int    BG_SLN = 0;
    static const int    BG_PPL = 1;
    static const int    BG_HOME = 2;
    BackgroundEvent(const int selection);

    int                 mSelection;
};

/**
 * \class na::ColorEvent
 * A generic color event.
 */
class ColorEvent : public Event {
  public:
    static int          WHAT();
    static const int    BACKGROUND_TARGET = 0;
    ColorEvent(const int target, const ColorRgba&, const float duration = 0.0f);

    int                 mTarget;
    ColorRgba           mColor;
    float               mDuration;
};

/**
 * \class na::SilenceEvent
 * A command for anything producing sound to go silent.
 */
class SilenceEvent : public Event {
  public:
    static int          WHAT();
    SilenceEvent(const int route);

    int                 mRoute;
};

/**
 * \class na::CloseRouteEvent
 * A command to lock off any future changes to the route, preventing it from
 * processing any more events.
 * NOTE: This only works because messaging is synchronous. If it were ever to
 * become async for some reason (and it really shouldn't), then this would
 * have to be bundled in with every event we're trying to prevent.
 */
class CloseRouteEvent : public Event {
  public:
    static int          WHAT();
    CloseRouteEvent(const int route);

    int                 mRoute;
};

/**
 * \class na::AppExitEvent
 */
class AppExitEvent : public Event {
  public:
    static int          WHAT();
    AppExitEvent();
};

} // namespace na

#endif // NA_EVENTS_APPEVENTS_H_
