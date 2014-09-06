#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class ObersverApp : public ds::App {
public:
	ObersverApp();

	void				setupServer();

private:
	typedef ds::App   inherited;
};

ObersverApp::ObersverApp() {
}

void ObersverApp::setupServer() {
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( ObersverApp, RendererGl )
