#include "stdafx.h"

#include "pdf_example_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>
#include <ds/ui/media/player/pdf_player.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/story/story_controller.h"

namespace downstream {

pdf_example_app::pdf_example_app()
  : ds::App()
  , mEventClient(mEngine) {}

void pdf_example_app::setupServer() {
	// add sprites
	mEngine.getRootSprite().addChildPtr(new StoryController(mEngine));

	/// loads a test pdf with links from the data folder
	addMediaViewer(ds::Environment::expand("%APP%/data/multi_pages.pdf"));
}

void pdf_example_app::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		addMediaViewer((*it).string());
	}
}
void pdf_example_app::addMediaViewer(std::string uri) {
	ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, uri, true);

	auto mvs				   = mv->getSettings();
	mvs.mPdfLinkTappedCallback = [this](ds::pdf::PdfLinkInfo info) {
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, info.mUrl, true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	};
	mvs.mPdfCanShowLinks = true;
	mv->setSettings(mvs);

	mv->initialize();

	// If you want raw access to the PDF player
	if (auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(mv->getPlayer())) {
		// In the case of PDF players, getPDF() returns the player itself
		// this is because the next/back/set page functions have been overriden
		// so we can pre-prender regular pdf pages
		if (auto thePdf = pdfPlayer->getPDF()) {
			// thePdf->setPageNum(1);
			/// or do other things with the pdf
		}
	}

	mEngine.getRootSprite().addChildPtr(mv);
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::pdf_example_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
