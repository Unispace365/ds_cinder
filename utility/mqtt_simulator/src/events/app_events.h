#ifndef _MQTT_SIMULATOR_APP_APPEVENTS_H_
#define _MQTT_SIMULATOR_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

struct MqttMessageEvent : public ds::RegisteredEvent<MqttMessageEvent> {
	MqttMessageEvent(std::string topic, std::string message) : mTopic(topic), mMessage(message) {}
	std::string mTopic;
	std::string mMessage;
};

struct MqttInboundRouteEvent : public ds::RegisteredEvent<MqttInboundRouteEvent> {
	MqttInboundRouteEvent(std::string newInboundRoute) : mInboundRoute(newInboundRoute) {}
	std::string mInboundRoute;
};


struct SendMqttMessageEvent : public ds::RegisteredEvent<SendMqttMessageEvent> {
	SendMqttMessageEvent(std::string theMsg) : mMessage(theMsg) {}
	std::string mMessage;
};

} // !namespace downstream

#endif // !_MQTT_SIMULATOR_APP_APPEVENTS_H_
