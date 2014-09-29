#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>
#include <Poco/Path.h>

#include <ds/app/app.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>
#include <ds/data/resource_list.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/web.h>

using namespace std;
using namespace ci;
using namespace ci::app;

namespace {
const std::string               FONT_NAME("din-medium");
const char                      EXAMPLE_DB_TYPE = ds::Resource::Id::CUSTOM_TYPE;
// A hardcoded resource from the example database
const ds::Resource::Id          KITTY_RES_ID(EXAMPLE_DB_TYPE, 44);

// Custom database info
std::string                     CUSTOM_RESOURCE_PATH;
std::string                     CUSTOM_DB_PATH;
std::string                     EMPTY_CUSTOM_PATH("");

// Test sending sort order by putting to the front anyone that's touched
class FrontSprite : public ds::ui::Sprite {
public:
	FrontSprite(ds::ui::SpriteEngine &e, float width = 0.0f, float height = 0.0f) : ds::ui::Sprite(e, width, height) { }

	virtual void			userInputReceived() {
		ds::ui::Sprite::userInputReceived();
		sendToFront();
	}
};

class FrontImage : public ds::ui::Image {
public:
	FrontImage(ds::ui::SpriteEngine &e, const std::string &fn, const int flags = 0) : ds::ui::Image(e, fn, flags) { }
	FrontImage(ds::ui::SpriteEngine &e, const ds::Resource::Id &id, const int flags = 0) : ds::ui::Image(e, id, flags) { }

	virtual void			userInputReceived() {
		ds::ui::Image::userInputReceived();
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
 
//	ds::ui::Web*			web(new ds::ui::Web(mEngine, 1024.0f, 768.0f));
	ds::ui::Web*			web(new ds::ui::Web(mEngine, 400.0f, 400.0f));
	if (web) {
//		web->setUrl("http://www.pinterest.com/cutiesnfuzzies/cute-n-fuzzy-kitties/");
		web->setUrl("http://google.com/");
		rootSprite.addChild(*web);
	}
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( CsApp, RendererGl )
