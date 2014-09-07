#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class ObservedSprite : public ds::ui::Sprite {
public:
	ObservedSprite(ds::ui::SpriteEngine& engine)
		: inherited(engine)
		, mMemeber(0)
		, mProTip("This is a sample showing you how to make your classes/sprites observable via Observer API. Take a look at sprite.cpp and observer.h for integration details.")
	{
		setName("Observed Sprite");
		installObserver();
		
		// You can use observer API for fast registration of class members
		observe<double>("mMember", &mMemeber, 0, 100);

		// You can also use native AntTweakBar API
		TwAddButton(getObserver(), "Protip", NULL, NULL, (" label='"+mProTip+"' ").c_str());
	}

private:
	typedef ds::ui::Sprite inherited;
	std::string			mProTip;
	double				mMemeber;
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
	
	auto s = mEngine.findSprite("Observed Sprite");
	s->setTransparent(false);
	s->setPosition(350, 100);
	s->setColor(1, 1, 0);
	s->setSize(250, 250);
	s->setCornerRadius(8);
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( ObersverApp, RendererGl )
