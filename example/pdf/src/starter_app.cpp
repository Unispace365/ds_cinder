#include <cinder/app/AppBasic.h>
#include <ds/app/environment.h>
#include <ds/app/app.h>
#include <ds/ui/sprite/pdf.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class BasicTweenApp : public ds::App {
public:
	BasicTweenApp();

	void				setupServer();

private:
	typedef ds::App		inherited;
};

BasicTweenApp::BasicTweenApp() {
}

void BasicTweenApp::setupServer() {
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	ds::ui::Pdf&	pdf = ds::ui::Sprite::makeAlloc<ds::ui::Pdf>([this]()->ds::ui::Pdf*{return new ds::ui::Pdf(this->mEngine);}, &rootSprite);
	pdf.setResourceFilename(ds::Environment::getAppFolder("data", "bitcoin.pdf"));
	pdf.setCenter(0.5f, 0.5f);
	pdf.setPosition(floorf(mEngine.getWorldWidth()/2.0f), floorf(mEngine.getWorldHeight()/2.0f));
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
