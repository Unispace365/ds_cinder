#ifndef _FULLSTARTER_APP_APPEVENTS_H_
#define _FULLSTARTER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace fullstarter {

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

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_APPEVENTS_H_