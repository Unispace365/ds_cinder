#ifndef DS_APP_ENGINE_EVENTS
#define DS_APP_ENGINE_EVENTS

#include <ds/app/event.h>

namespace ds { namespace app {

	class EngineStateEvent : public ds::RegisteredEvent<EngineStateEvent> {
	  public:
		/// A client has connected to this server
		static const int ENGINE_STATE_CLIENT_STARTED = 0;

		/// A client has requested a new world
		static const int ENGINE_STATE_SEND_WORLD = 1;

		/// A client has entered normal running mode
		static const int ENGINE_STATE_CLIENT_RUNNING = 2;

		EngineStateEvent(const int stateType)
		  : mStateType(stateType){};
		const int mStateType;
	};

	/// Someone began interacting with the app
	class IdleEndedEvent : public ds::RegisteredEvent<IdleEndedEvent> {};

	/// It's been `<idle seconds>` since someone last interacted with the app
	class IdleStartedEvent : public ds::RegisteredEvent<IdleStartedEvent> {};

	/// A request for the app to exit completely
	struct RequestAppExitEvent : public ds::RegisteredEvent<RequestAppExitEvent> {};

	/// A new entry field has been registered in the engine
	struct EntryFieldRegisteredEvent : public ds::RegisteredEvent<EntryFieldRegisteredEvent> {};
}} // namespace ds::app

#endif
