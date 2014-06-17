#include "na/events/app_events.h"

namespace na {

/**
 * \class na::WallModeChanged
 */
static EventRegistry    WALL_MODE_CHANGED('WMCh', "wall  mode changed");

int WallModeChanged::WHAT()
{
  return WALL_MODE_CHANGED.mWhat;
}

WallModeChanged::WallModeChanged(const int mode)
  : Event(WALL_MODE_CHANGED.mWhat)
  , mMode(mode)
{
}

/**
 * \class na::BackgroundEvent
 */
static EventRegistry    BACKGROUND_EVENT('Bg_E', "background event");

int BackgroundEvent::WHAT()
{
  return BACKGROUND_EVENT.mWhat;
}

BackgroundEvent::BackgroundEvent(const int selection)
  : Event(BACKGROUND_EVENT.mWhat)
  , mSelection(selection)
{
}

/**
 * \class na::ColorEvent
 */
static EventRegistry    COLOR_EVENT('ClrE', "color event");

int ColorEvent::WHAT()
{
  return COLOR_EVENT.mWhat;
}

ColorEvent::ColorEvent(const int target, const ColorRgba& c, const float duration)
  : Event(COLOR_EVENT.mWhat)
  , mTarget(target)
  , mColor(c)
  , mDuration(duration)
{
}

/**
 * \class na::SilenceEvent
 */
static EventRegistry    SILENCE_EVENT('shhh', "silence event");

int SilenceEvent::WHAT()
{
  return SILENCE_EVENT.mWhat;
}

SilenceEvent::SilenceEvent(const int route)
  : Event(SILENCE_EVENT.mWhat)
  , mRoute(route)
{
}

/**
 * \class na::CloseRouteEvent
 */
static EventRegistry    CLOSE_ROUTE_EVENT('ClsR', "close route event");

int CloseRouteEvent::WHAT()
{
  return CLOSE_ROUTE_EVENT.mWhat;
}

CloseRouteEvent::CloseRouteEvent(const int route)
  : Event(CLOSE_ROUTE_EVENT.mWhat)
  , mRoute(route)
{
}

/**
 * \class na::AppExitEvent
 */
static EventRegistry    APP_EXIT_EVENT('aext', "app exit event");

int AppExitEvent::WHAT()
{
  return APP_EXIT_EVENT.mWhat;
}

AppExitEvent::AppExitEvent()
  : Event(APP_EXIT_EVENT.mWhat)
{
}

} // namespace na
