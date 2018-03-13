#include "stdafx.h"

#include "generic_data_model_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/content/content_events.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "ui/table/table_view.h"

namespace downstream {

generic_data_model_app::generic_data_model_app()
	: ds::App()
	, mEventClient(mEngine)
{
	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) {
		auto dm = mEngine.mContent.getChildByName("sample_data");
		for(auto it : dm.getChildren()) {
			auto sl = new ds::ui::SmartLayout(mEngine, "sample_data.xml");
			sl->setContentModel(it);
			mEngine.getRootSprite().addChildPtr(sl);

		}
	});
}

void generic_data_model_app::setupServer(){
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	
	// add sprites
	rootSprite.addChildPtr(new TableView(mEngine));

}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::generic_data_model_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
