#ifndef _PANORAMICVIDEO_APP_APPEVENTS_H_
#define _PANORAMICVIDEO_APP_APPEVENTS_H_

#include <ds/app/event.h>
#include <ds/data/resource.h>

namespace panoramic {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

class RequestVideoList : public ds::RegisteredEvent<RequestVideoList>{
public:
	RequestVideoList(const ci::vec3& location) : mLocation(location){}
	const ci::vec3 mLocation;
};

class RequestCloseAllEvent : public ds::RegisteredEvent<RequestCloseAllEvent>{
public:
	RequestCloseAllEvent(){}
};

class RequestPanoramicVideo : public ds::RegisteredEvent<RequestPanoramicVideo>{
public:
	RequestPanoramicVideo(const ds::Resource reccy) : mResource(reccy){}
	ds::Resource mResource;
};

} // !namespace panoramic

#endif // !_PANORAMICVIDEO_APP_APPEVENTS_H_

