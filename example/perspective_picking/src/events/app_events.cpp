#include "app_events.h"

namespace perspective_picking {


static ds::EventRegistry	ISE("IdleStartedEvent");
int IdleStartedEvent::WHAT(){ return ISE.mWhat; }

IdleStartedEvent::IdleStartedEvent(  )
		: ds::Event(ISE.mWhat)
{
}


static ds::EventRegistry	IEE("IdleEndedEvent");
int IdleEndedEvent::WHAT(){ return IEE.mWhat; }

IdleEndedEvent::IdleEndedEvent(  )
	: ds::Event(IEE.mWhat)
{
}

} // !namespace perspective_picking


