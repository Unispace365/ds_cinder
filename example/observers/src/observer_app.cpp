#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class ObservedSprite : public ds::ui::Sprite {
public:
	ObservedSprite(ds::ui::SpriteEngine& engine) : inherited(engine) {
		installObserver();
	}

private:
	typedef ds::ui::Sprite inherited;
};

class ObersverApp : public ds::App {
public:
	ObersverApp();

	void				setupServer();

private:
	typedef ds::App   inherited;
	ObservedSprite&	  mObservedSprite;
};

ObersverApp::ObersverApp()
	: mObservedSprite(ds::ui::Sprite::make<ObservedSprite>(mEngine, &mEngine.getRootSprite())) {

}

void ObersverApp::setupServer() {
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( ObersverApp, RendererGl )