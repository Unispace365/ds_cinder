#ifndef _MQTT_EXAMPLE_APP_H_
#define _MQTT_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "ds/touch/touch_debug.h"

#include "ds/network/mqtt/mqtt_watcher.h"

namespace example {
class AllData;

class mqtt_example : public ds::App {
public:
	mqtt_example();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

private:
	typedef ds::App		inherited;



	ds::TouchDebug		mTouchDebug;

	ds::net::MqttWatcher	mMqttWatcher;


	void				moveCamera(const ci::vec3& deltaMove);
};

} // !namespace example

#endif // !_MQTT_EXAMPLE_APP_H_

