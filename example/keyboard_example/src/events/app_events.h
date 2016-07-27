#ifndef _KEYBOARDEXAMPLE_APP_APPEVENTS_H_
#define _KEYBOARDEXAMPLE_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace example {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

class KeyPressedEvent : public ds::RegisteredEvent<KeyPressedEvent> {
public:
	KeyPressedEvent(const std::wstring& fullString) : mFullString(fullString){}
	const std::wstring mFullString;
};

} // !namespace example

#endif // !_KEYBOARDEXAMPLE_APP_APPEVENTS_H_