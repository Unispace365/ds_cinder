#ifndef _MQTT_EXAMPLE_APP_H_
#define _MQTT_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "ds/network/mqtt/mqtt_watcher.h"

namespace example {
class AllData;

class mqtt_example : public ds::App {
  public:
	mqtt_example();

	virtual void onKeyDown(ci::app::KeyEvent event) override;
	void		 setupServer();

  private:
	typedef ds::App inherited;


	ds::net::MqttWatcher mMqttWatcher;
};

} // namespace example

#endif // !_MQTT_EXAMPLE_APP_H_
