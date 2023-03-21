#include "pngsequenceexample_app.h"

#include <Poco/String.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/text.h>

#include <cinder/Rand.h>
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"


namespace example {

PngSequenceExample::PngSequenceExample()
  : inherited()
  , mGlobals(mEngine) {

	/// NOTE! you have to drop a series of images onto the app to display a png sequence

	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
}

void PngSequenceExample::setupServer() {

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	ds::ui::Sprite& rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	rootSprite.enable(true);
	rootSprite.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	rootSprite.setTapCallback([this](ds::ui::Sprite*, const ci::vec3&) {
		if (mPngSequence) {
			if (mPngSequence->isPlaying()) {
				mPngSequence->pause();
			} else {
				mPngSequence->setCurrentFrameIndex(0);
				mPngSequence->play();
			}
		}
	});

	// add sprites

	mPngSequence = new ds::ui::PngSequenceSprite(mEngine);
	mPngSequence->setFrameTime(0.032f);
	mPngSequence->enable(true);
	mPngSequence->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	mPngSequence->setSize(400.0f, 400.0f);
	rootSprite.addChildPtr(mPngSequence);
}


void PngSequenceExample::onKeyDown(ci::app::KeyEvent event) {
	using ci::app::KeyEvent;
	if (event.getCode() == KeyEvent::KEY_l) {
		if (mPngSequence->getLoopStyle() == ds::ui::PngSequenceSprite::Loop) {
			mPngSequence->setLoopStyle(ds::ui::PngSequenceSprite::Once);
		} else {
			mPngSequence->setLoopStyle(ds::ui::PngSequenceSprite::Loop);
		}
	}
}

void PngSequenceExample::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		std::cout << (*it).string() << std::endl;
		paths.push_back((*it).string());
	}

	std::sort(paths.begin(), paths.end());

	if (mPngSequence) {
		mPngSequence->setImages(paths);
		mPngSequence->play();
	}
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::PngSequenceExample, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
