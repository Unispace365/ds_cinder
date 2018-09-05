#include "stdafx.h"

#include "mqtt_service.h"

#include <cinder/Json.h>
#include <cinder/Xml.h>
#include <ds/debug/logger.h>

#include "events/app_events.h"

namespace downstream {

MqttService::MqttService(ds::ui::SpriteEngine& eng)
	: mEngine(eng)
	, mMqttWatcher(eng, "", "", "")
	, mConnectMqtt(true)
	, mEventClient(mEngine)
{
	mInboundTopic = mEngine.getAppSettings().getString("rfid:mqtt:inbound_route", 0, "");
	mOutboundTopic = mEngine.getAppSettings().getString("rfid:mqtt:outbound_route", 0, "");

	mEventClient.listenToEvents<MqttInboundRouteEvent>([this](auto& e) {
		mInboundTopic = e.mInboundRoute;
	});

	mEventClient.listenToEvents<SendMqttMessageEvent>([this](const SendMqttMessageEvent& e) {
		mMqttWatcher.sendOutboundMessage(e.mMessage);
	});
}

void MqttService::initialize() {


	mMqttWatcher.stopListening();
	mMqttWatcher.setHostString(mEngine.getAppSettings().getString("rfid:mqtt:address", 0, ""));
	mMqttWatcher.setTopicInbound(mInboundTopic);
	mMqttWatcher.setTopicOutbound(mOutboundTopic);
	mMqttWatcher.setPort(mEngine.getAppSettings().getInt("rfid:mqtt:port", 0, 1883));

	mMqttWatcher.clearInboundListeners();
	mMqttWatcher.addInboundListener([this](const ds::net::MqttWatcher::MessageQueue& queue) {

		for(auto msg : queue) {
			mEngine.getNotifier().notify(MqttMessageEvent(msg.topic, msg.message));
			DS_LOG_INFO("MQTT message: " << msg.topic << " " << msg.message);
		}
	});

	mMqttWatcher.startListening();
}


}  // namespace ds
