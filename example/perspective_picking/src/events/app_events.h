#ifndef _PERSPECTIVEPICKING_APP_APPEVENTS_H_
#define _PERSPECTIVEPICKING_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace perspective_picking {

class IdleStartedEvent : public ds::Event {
public:

	static int		WHAT();

	IdleStartedEvent( );

};

class IdleEndedEvent : public ds::Event {
public:

	static int		WHAT();

	IdleEndedEvent( );

};

} // !namespace perspective_picking

#endif // !_PERSPECTIVEPICKING_APP_APPEVENTS_H_