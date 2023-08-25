#include "pango_app.h"

#include <Poco/String.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>


#include <cinder/Rand.h>
#include <cinder/app/RendererGl.h>

#include "app/globals.h"

#include "events/app_events.h"

#include "ds/util/markdown_to_pango.h"
#include "ds/util/string_util.h"

#include "ui/story/story_view.h"


namespace pango {

PangoApp::PangoApp()
  : ds::App()
  , mGlobals(mEngine, mAllData)
  , mQueryHandler(mEngine, mAllData)
  , mEventClient(mEngine.getNotifier(), [this](const ds::Event* m) {
	  if (m) this->onAppEvent(*m);
  }) {

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(StoryDataUpdatedEvent::NAME(),
											   [this]() -> ds::Event* { return new StoryDataUpdatedEvent(); });

	/// Load a local font file, and let the engine know what it's font name is. We can now refer to it by it's full
	/// name, Chlorinar Bold Italic
	mEngine.editFonts().installFont(ds::Environment::expand("%APP%/data/fonts/CHLORINR.ttf"), "Chlorinar Bold Italic");

	/// by entering the "title" as the third parameter, now we can refer to the font by that name or FreightSans Light.
	/// This is for convenience, as you could refer to "title" everywhere needed, and just replace this line to replace
	/// all the title fonts.
	mEngine.editFonts().installFont(ds::Environment::expand("%APP%/data/fonts/FreightSans-Light.ttf"),
									"FreightSans Light,", "title");

	// Fonts links together a font name and a physical font file
	// Then the "text.xml" and TextCfg will use those font names to specify visible settings (size, color, leading)
	mEngine.loadSettings("FONTS", "fonts.xml");

	mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& setting) {
		// this is a way to register a font as well, which registers the font name (for example, Noto Sans Bold) to the
		// short name (for example noto-bold). So in your layout files, you can now set the font_name to be noto-bold OR
		// Noto Sans Bold.
		mEngine.editFonts().installFont(ds::Environment::expand(setting.mRawValue), setting.mRawValue, setting.mName);
	});
}

void PangoApp::setupServer() {


	mQueryHandler.runInitialQueries(true);

	ds::ui::Sprite& rootSprite = mEngine.getRootSprite(0);
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.9f, 0.9f, 0.9f));

	// add sprites
	rootSprite.addChildPtr(new StoryView(mGlobals));

	// auto secondStory = new StoryView(mGlobals);
	// secondStory->setPosition(200.0f, 500.0f);
	// rootSprite.addChildPtr(secondStory);
}

void PangoApp::onAppEvent(const ds::Event& in_e) {}

void PangoApp::onKeyDown(ci::app::KeyEvent event) {
	using ci::app::KeyEvent;
}


void PangoApp::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		mEngine.getRootSprite().clearChildren();

		// read the file into a string
		std::ifstream t((*it).string().c_str());
		std::string	  str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		// parse it for pango
		std::string pangoMd = ds::ui::markdown_to_pango(str);


		ds::ui::Text* texty = new ds::ui::Text(mEngine);
		texty->setFont("Noto Sans");
		texty->setFontSize(10.0f);
		texty->setResizeLimit(mEngine.getWorldWidth() - 200.0f);
		texty->setLeading(1.2f);
		texty->setText(pangoMd);
		texty->setPosition(100.0f, 100.0f);
		texty->setColor(ci::Color(0.1f, 0.1f, 0.1f));

		texty->enable(true);
		texty->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION_Y);
		mEngine.getRootSprite().addChildPtr(texty);
	}
}

} // namespace pango

// This line tells Cinder to actually create the application
CINDER_APP(pango::PangoApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
