#include "stdafx.h"

#include "video_converter_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/conversion/conversion_view.h"
#include "ui/playback/playback_view.h"

namespace downstream {

video_converter_app::video_converter_app()
	: ds::App()
	, mEventClient(mEngine)
{

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	//ds::event::Registry::get().addEventCreator(SomethingHappenedEvent::NAME(), [this]()->ds::Event* {return new SomethingHappenedEvent(); });

	//mEventClient.listenToEvents<SomethingHappenedEvent>([this](const SomethingHappenedEvent& e) { std::cout << "Something happened" << std::endl; });

	//registerKeyPress("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); }, ci::app::KeyEvent::KEY_n);
}

void video_converter_app::setupServer(){

	// add sprites
	mEngine.getRootSprite().addChildPtr(new ConversionView(mEngine));
	mEngine.getRootSprite().addChildPtr(new PlaybackView(mEngine));

}

void video_converter_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it : event.getFiles()){
		paths.emplace_back(it.string());
	}
	mEngine.getNotifier().notify(RequestConvertFiles(paths));

}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::video_converter_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); settings->setResizable(false); })
