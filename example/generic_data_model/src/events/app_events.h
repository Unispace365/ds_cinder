#ifndef _GENERIC_DATA_MODEL_APP_APPEVENTS_H_
#define _GENERIC_DATA_MODEL_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class DataUpdatedEvent : public ds::RegisteredEvent<DataUpdatedEvent>{
public:
	DataUpdatedEvent(){};
}; 

class RequestAppExitEvent : public ds::RegisteredEvent<RequestAppExitEvent>{
public:
	RequestAppExitEvent(){};
};

} // !namespace downstream

#endif // !_GENERIC_DATA_MODEL_APP_APPEVENTS_H_
