#include "mqtt_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"

namespace example {

mqtt_example::mqtt_example()
	: inherited(ds::RootList()
								.ortho() 
								.pickColor() ) 
	, mTouchDebug(mEngine)
	, mMqttWatcher(mEngine, "bmc.downstreamsandbox.com", "presentation/colorado/#", "presentation/colorado/#", 0.01f, 1883)
{

	mMqttWatcher.addInboundListener([this](const ds::net::MqttWatcher::MessageQueue& mq) {

		/* old
		std::queue<std::string> inQ = mq;
		while(!inQ.empty()){
			std::cout << "Got MQTT message:  " << inQ.front() << std::endl;
			inQ.pop();
		}
		*/

		// new
		for (auto msg : mq) {
			const std::string& topic = msg.topic;
			const std::string& message = msg.message;
			std::cout << "Got MQTT message:  " << message.front() << std::endl;
		}

	});
	
	/*fonts in use */
	mEngine.editFonts().installFont(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "Noto Sans Bold", "noto-bold");

	enableCommonKeystrokes(true);
}

void mqtt_example::setupServer(){
	mMqttWatcher.startListening();

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
}

void mqtt_example::update() {
	inherited::update();


}

void mqtt_example::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	}

	std::cout << "sending message to mqtt: Hello Whirlled!" << std::endl;
	mMqttWatcher.sendOutboundMessage("Hello Whirlled!");
}

void mqtt_example::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void mqtt_example::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void mqtt_example::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}


} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::mqtt_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

