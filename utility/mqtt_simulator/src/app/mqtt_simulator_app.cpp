#include "stdafx.h"

#include "mqtt_simulator_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/form/form_view.h"

namespace downstream {

mqtt_simulator_app::mqtt_simulator_app()
	: ds::App()
	, mEventClient(mEngine)
	, mMqttService(mEngine)
{
}

void mqtt_simulator_app::setupServer(){
	mMqttService.initialize();

	// add sprites
	mEngine.getRootSprite().addChildPtr(new FormView(mEngine));
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::mqtt_simulator_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
