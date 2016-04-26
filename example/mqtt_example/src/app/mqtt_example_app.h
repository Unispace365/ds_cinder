#ifndef _MQTT_EXAMPLE_APP_H_
#define _MQTT_EXAMPLE_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
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

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	//Idle state of the app to detect state change
	bool				mIdling;


	ds::TouchDebug		mTouchDebug;

	ds::net::MqttWatcher	mMqttWatcher;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace example

#endif // !_MQTT_EXAMPLE_APP_H_