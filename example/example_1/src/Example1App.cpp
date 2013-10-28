#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>
#include <Poco/Path.h>

#include <ds/app/app.h>
#include <ds/app/environment.h>
#include <ds/data/resource_list.h>
#include <ds/thread/runnable_client.h>
#include <ds/query/query_client.h>
#include <ds/cfg/settings.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/ui/sprite/image.h>

using namespace std;
using namespace ci;
using namespace ci::app;

namespace {
const char                      EXAMPLE_DB_TYPE = ds::Resource::Id::CUSTOM_TYPE;
// A hardcoded resource from the example database
const ds::Resource::Id          KITTY_RES_ID(EXAMPLE_DB_TYPE, 44);

// Custom database info
std::string                     CUSTOM_RESOURCE_PATH;
std::string                     CUSTOM_DB_PATH;
std::string                     EMPTY_CUSTOM_PATH("");
}

class BasicTweenApp : public ds::App {
  public:
    BasicTweenApp();

    void				setupServer();
    void				mouseDown( MouseEvent event );
    void				draw();


    Anim<Vec2f>			mBlackPos, mWhitePos;

    ds::RunnableClient	mClient;
    ds::query::Client	mQuery;

  private:
    typedef ds::App   inherited;
};

class MessageRunnable : public Poco::Runnable {
public:
	MessageRunnable() { }

	virtual void			run() {
		std::cout << "Example of a generic worker runnable, running in another thread, doing something cool" << endl;
	}
};

BasicTweenApp::BasicTweenApp()
	: mClient(mEngine)
	, mQuery(mEngine)
{
  try {
    // Example worker
	  mClient.setResultHandler([](std::unique_ptr<Poco::Runnable>&){ cout << "Example worker just finished doing something cool" << endl; });
    // Example query
	  mQuery.setResultHandler([](const ds::query::Result& r, ds::query::Talkback&){ cout << "query finished rowSize=" << r.getRowSize() << endl; });

    // Example settings
    ds::cfg::Settings   settings;
    settings.readFrom(ds::Environment::getAppFolder("data", "example_settings.xml"));
    cout << "settings background color=" << settings.getColor("background") << endl;

    // Setup my custom database info
    CUSTOM_RESOURCE_PATH = ds::Environment::getAppFolder("data", "resources");
    CUSTOM_DB_PATH = ds::Environment::getAppFolder("data", "resources/db/db.sqlite");
    ds::Resource::Id::setupCustomPaths( [](const ds::Resource::Id& id)->const std::string&{ if (id.mType == EXAMPLE_DB_TYPE) return CUSTOM_RESOURCE_PATH; return EMPTY_CUSTOM_PATH; },
                                        [](const ds::Resource::Id& id)->const std::string&{ if (id.mType == EXAMPLE_DB_TYPE) return CUSTOM_DB_PATH; return EMPTY_CUSTOM_PATH; });
  } catch (std::exception const& ex) {
    cout << "ERROR in app constructor=" << ex.what() << endl;
  }
}

void BasicTweenApp::setupServer()
{
  mBlackPos = mWhitePos = getWindowCenter();

  ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
  rootSprite.setColor(0.3333f, 0.3333f, 0.3333f);
  rootSprite.setTransparent(false);

  ds::ui::Image   *imgSprite;
 
  // Example image sprite from a hardcoded filename.
  imgSprite = new ds::ui::Image(mEngine, ds::Environment::getAppFolder("data", "lorem_kicksum.w_1170.h_1146.png"));
  imgSprite->setScale(0.25f, 0.25f);
  imgSprite->enable(true);
  imgSprite->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
  rootSprite.addChild(*imgSprite);

  // Example image sprite from a resource.
  imgSprite = new ds::ui::Image(mEngine, KITTY_RES_ID);
  imgSprite->setScale(0.5f, 0.5f);
  imgSprite->setPosition(200, 200);
  imgSprite->enable(true);
  imgSprite->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
  rootSprite.addChild(*imgSprite);

  ds::ui::Sprite *child = new ds::ui::Sprite(mEngine, 100.0f, 100.0f);
  child->setPosition(getWindowWidth() / 4.0f, getWindowHeight() / 4.0f);
  child->setCenter(0.5f, 0.5f);
  child->setColor(1.0f, 1.0f, 0.0f);
  child->setTransparent(false);
  child->enable(true);
  child->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
  rootSprite.addChild(*child);

	// Example image sprite inside a rotated sprite.
	ds::ui::Sprite&		rotated = ds::ui::Sprite::makeAlloc<ds::ui::Sprite>([this]()->ds::ui::Sprite*{return new ds::ui::Sprite(this->mEngine);}, &rootSprite);
	rotated.setPosition(200.0f, 200.0f);
	rotated.setSize(300.0f, 300.0f);
	rotated.setRotation(90.0f);
	rotated.setTransparent(false);
	rotated.setColor(0.34f, 0.22f, 0.21f);
	ds::ui::Image&		ri = ds::ui::Sprite::makeAlloc<ds::ui::Image>([this]()->ds::ui::Image*{return new ds::ui::Image(this->mEngine, ds::Environment::getAppFolder("data", "btn_left_up.png"));}, &rotated);
	ri.setPosition(60.0f, 60.0f);
	ri.enable(true);
	ri.enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
}

void BasicTweenApp::mouseDown( MouseEvent event )
{
  inherited::mouseDown(event);

	mClient.run(std::unique_ptr<Poco::Runnable>(new MessageRunnable()));
	mQuery.runAsync(ds::Environment::getAppFolder("data", "example_db.sqlite"), "SELECT * FROM person");

	// the call to apply() replaces any existing tweens on mBlackPos with this new one
	timeline().apply( &mBlackPos, (Vec2f)event.getPos(), 2.0f, EaseInCubic() );
	// the call to appendTo causes the white circle to start when the black one finishes
	timeline().apply( &mWhitePos, (Vec2f)event.getPos(), 0.35f, EaseOutQuint() ).appendTo( &mBlackPos );
}

void BasicTweenApp::draw()
{
  inherited::draw();
	
	gl::color( Color::black() );
	gl::drawSolidCircle( mBlackPos, 20.0f );
	
	gl::color( Color::white() );
	gl::drawSolidCircle( mWhitePos, 16.0f );
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
