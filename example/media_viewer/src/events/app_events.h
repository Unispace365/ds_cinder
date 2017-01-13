#ifndef _MEDIAVIEWER_APP_APPEVENTS_H_
#define _MEDIAVIEWER_APP_APPEVENTS_H_

#include <ds/app/event.h>
#include "model/generated/media_model.h"

namespace mv {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

class RequestMediaOpenEvent : public ds::RegisteredEvent < RequestMediaOpenEvent > {
public: 
	RequestMediaOpenEvent(ds::model::MediaRef media, const ci::vec3& location, const float startWidth)
		: mMedia(media), mLocation(location), mStartWidth(startWidth){}

	ds::model::MediaRef mMedia;
	ci::vec3 mLocation;
	const float mStartWidth;
};

class RequestCloseAllEvent : public ds::RegisteredEvent < RequestCloseAllEvent > {
public:
	RequestCloseAllEvent(){}
};

class RequestLayoutEvent : public ds::RegisteredEvent < RequestLayoutEvent > {
public:
	RequestLayoutEvent(){}
};

} // !namespace mv

#endif // !_MEDIAVIEWER_APP_APPEVENTS_H_