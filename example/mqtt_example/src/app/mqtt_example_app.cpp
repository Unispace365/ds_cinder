#include "mqtt_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

namespace example {

mqtt_example::mqtt_example()
	: inherited() 
	, mMqttWatcher(mEngine, "10.143.100.179", "#", "#", 0.01f, 1883)
//	, mMqttWatcher(mEngine, "PUT_AN_MQTT_SERVER_HERE_DUMMY", "#", "#", 0.01f, 1883)
{

	mMqttWatcher.addInboundListener([this](const ds::net::MqttWatcher::MessageQueue& mq) {
		// new
		for (auto msg : mq) {
			const std::string& topic = msg.topic;
			const std::string& message = msg.message;
			std::cout << "Got MQTT message:  " << message.front() << std::endl;
		}

	});
	
	/*fonts in use */
	mEngine.editFonts().installFont(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "Noto Sans Bold", "noto-bold");
}

void mqtt_example::setupServer(){
	mMqttWatcher.startListening();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
}

void mqtt_example::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	}

	std::cout << "sending message to mqtt: Hello Whirlled!" << std::endl;
	mMqttWatcher.sendOutboundMessage("Hello Whirlled!");
}


} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::mqtt_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

