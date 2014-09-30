#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>
#include <Poco/Path.h>

#include <ds/app/app.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>
#include <ds/data/resource_list.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/sprite/web.h>

using namespace std;
using namespace ci;
using namespace ci::app;

namespace {
const std::string               FONT_NAME("din-medium");

// Test sending sort order by putting to the front anyone that's touched
class FrontVideo : public ds::ui::Video {
public:
	FrontVideo(ds::ui::SpriteEngine &e) : ds::ui::Video(e) { }

	virtual void			userInputReceived() {
		ds::ui::Sprite::userInputReceived();
		sendToFront();
	}
};

}

class CsApp : public ds::App {
public:
	CsApp();

	void				setupServer();

private:
	typedef ds::App		inherited;
};

CsApp::CsApp() {
	try {
		// Setup my custom fonts. This is purely optional, but is used to optimize
		// server->client communication. If the client and server have different font
		// folders, that's fine. You'll need to construct each on its own, and in the
		// same order.
		// It's maybe worth nothing that if the client and server do have different
		// font folders, then you HAVE to do this, to abstract that info.
		mEngine.editFonts().install(ds::Environment::getAppFolder("data/fonts", "DIN-Medium.otf"), FONT_NAME);
	} catch (std::exception const& ex) {
		cout << "ERROR in app constructor=" << ex.what() << endl;
	}
}

void CsApp::setupServer() {
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
 
	ds::ui::Web*			web(new ds::ui::Web(mEngine, 400.0f, 400.0f));
	if (web) {
		web->setUrl("http://google.com/");
		rootSprite.addChild(*web);
	}

	ds::ui::Video*			vid(new FrontVideo(mEngine));
	if (vid) {
		vid->setLooping(true);
		vid->loadVideo("%APP%/data/video/jci_video_test_small.mp4");
		vid->enable(true);
		vid->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
		vid->setPosition(400.0f, 400.0f);
		rootSprite.addChild(*vid);
	}
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( CsApp, RendererGl )
