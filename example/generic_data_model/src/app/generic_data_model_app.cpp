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
	, mSampleValue(123.45f)
{
	// The Engine's mContent retains it's userdata and properties through data requeries
	// See Table view for getting this data out
	mEngine.mContent.setUserData(this);
	std::string wellHelloThere = "Well hello there";
	mEngine.mContent.setProperty("example", wellHelloThere);

	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) {


		/// This is a baseline equality test, of course this should return true
		bool equalityTest = mEngine.mContent == mEngine.mContent;
		DS_LOG_INFO("Test for equality A: " << equalityTest);

		/// Duplicating the data makes a copy of the root and all it's children
		auto theDupe = mEngine.mContent.duplicate();

		/// We still expect this to be true, since the test for equality is on each item, and not if they're the same data pointer
		equalityTest = mEngine.mContent == theDupe;
		DS_LOG_INFO("Test for equality after dupe: " << equalityTest);

		/// Set the label for a child of the dupe
		if(!theDupe.getChildren().empty()) {
			auto chillin = theDupe.getChild(0);
			chillin.setLabel("Well shing a thinga majig");
		}

		/// Now we'd expect these to not be the same, since there's a label difference on a child
		equalityTest = mEngine.mContent == theDupe;
		DS_LOG_INFO("Test for equality after dupe change: " << equalityTest);

		//mEngine.mContent.printTree(true, "");
		//theDupe.printTree(true, "");


		auto dm = mEngine.mContent.getChildByName("sample_data");
		for(auto it : dm.getChildren()) {
			auto sl = new ds::ui::SmartLayout(mEngine, "sample_data.xml");
			sl->setContentModel(it);
			mEngine.getRootSprite().addChildPtr(sl);

		}
	});

	registerKeyPress("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); }, ci::app::KeyEvent::KEY_u);
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
