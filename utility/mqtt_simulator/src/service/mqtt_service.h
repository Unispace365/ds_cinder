#pragma once
#ifndef DOWNSTREAM_SERVICE_MQTT_SERVICE
#define DOWNSTREAM_SERVICE_MQTT_SERVICE

#include <ds/network/mqtt/mqtt_watcher.h>
#include <ds/app/event_client.h>

namespace downstream {

/**
* \class downstream::MqttService
*	No state is kept here. This simply connects to MQTT and dispatches events as needed.
*/
class MqttService {
public:
	MqttService(ds::ui::SpriteEngine&);

	void									initialize();

private:
	ds::ui::SpriteEngine&					mEngine;
	ds::net::MqttWatcher					mMqttWatcher;
	ds::EventClient							mEventClient;

	std::string								mInboundTopic;
	std::string								mOutboundTopic;

	bool									mConnectMqtt;
};


} // namespace ds

#endif // PWC_SERVICE_BEACON_CONNECTOIN_SERVICE
