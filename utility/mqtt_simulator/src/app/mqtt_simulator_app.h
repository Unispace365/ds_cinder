#ifndef _MQTT_SIMULATOR_APP_H_
#define _MQTT_SIMULATOR_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include "service/mqtt_service.h"

namespace downstream {
class AllData;

class mqtt_simulator_app : public ds::App {
public:
	mqtt_simulator_app();

	void				setupServer();

private:

	// App events can be handled here
	ds::EventClient		mEventClient;

	MqttService			mMqttService;
};

} // !namespace downstream

#endif // !_MQTT_SIMULATOR_APP_H_
