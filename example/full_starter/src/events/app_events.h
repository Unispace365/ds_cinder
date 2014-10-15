
#pragma once
#ifndef EVENTS_APPEVENTS_H_
#define EVENTS_APPEVENTS_H_

#include <ds/app/event.h>

namespace fullstarter{

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

}//namespace fullstarter

#endif
