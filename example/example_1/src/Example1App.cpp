#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>
#include <Poco/Path.h>

#include <ds/app/engine.h>

#include <ds/thread/runnable_client.h>
#include <ds/query/query_client.h>
#include <ds/config/settings.h>

using namespace std;
using namespace ci;
using namespace ci::app;

// Answer the path to my local data directory, optionally to a filename
static std::string    get_data_path(const std::string& fn = "")
{
  Poco::Path			p(Poco::Path::expand("%DS_PLATFORM%"));
  p.append("example").append("example_1").append("data");
  if (!fn.empty()) p.append(fn);
  return p.toString();
}

class BasicTweenApp : public AppBasic {
  public:
	BasicTweenApp();

  void        prepareSettings( Settings *settings );
	void				setup();
  void				mouseDown( MouseEvent event );
  void        touchesBegan( TouchEvent event );
  void        touchesMoved( TouchEvent event );
  void        touchesEnded( TouchEvent event );
	void				update();
	void				draw();
  
	ds::Engine			mEngine;
	Anim<Vec2f>			mBlackPos, mWhitePos;

	ds::RunnableClient	mClient;
	ds::query::Client	mQuery;
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
	,  mQuery(mEngine)
{
  try {
    // Example worker
	  mClient.setResultHandler([](std::unique_ptr<Poco::Runnable>&){ cout << "Example worker just finished doing something cool" << endl; });
    // Example query
	  mQuery.setResultHandler([](const ds::query::Result& r, ds::query::Talkback&){ cout << "query finished rowSize=" << r.getRowSize() << endl; });

    // Example settings
    ds::cfg::Settings   settings;
    settings.readFrom(get_data_path("example_settings.xml"));
    cout << "settings background color=" << settings.getColor("background") << endl;
  } catch (std::exception const& ex) {
    cout << "ERROR in app constructor=" << ex.what() << endl;
  }
}

void BasicTweenApp::setup()
{	
  mEngine.setup();
	mBlackPos = mWhitePos = getWindowCenter();

  tuio::Client &tuioClient = mEngine.getTuioClient();
  tuioClient.registerTouches(this);
  tuioClient.connect();

  ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

  ds::ui::Sprite *child = new ds::ui::Sprite(100.0f, 100.0f);
  child->setPosition(getWindowWidth() / 4.0f, getWindowHeight() / 4.0f);
  child->setCenter(0.5f, 0.5f);
  child->setColor(1.0f, 1.0f, 0.0f);
  child->setTransparent(false);
  child->enable(true);
  rootSprite.addChild(child);
}

void BasicTweenApp::mouseDown( MouseEvent event )
{
	mClient.run(std::unique_ptr<Poco::Runnable>(new MessageRunnable()));
	mQuery.runAsync(get_data_path("example_db.sqlite"), "SELECT * FROM person");

	// the call to apply() replaces any existing tweens on mBlackPos with this new one
	timeline().apply( &mBlackPos, (Vec2f)event.getPos(), 2.0f, EaseInCubic() );
	// the call to appendTo causes the white circle to start when the black one finishes
	timeline().apply( &mWhitePos, (Vec2f)event.getPos(), 0.35f, EaseOutQuint() ).appendTo( &mBlackPos );
}

void BasicTweenApp::update()
{
	mEngine.update();
}

void BasicTweenApp::draw()
{
	gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
	
	gl::color( Color::black() );
	gl::drawSolidCircle( mBlackPos, 20.0f );
	
	gl::color( Color::white() );
	gl::drawSolidCircle( mWhitePos, 16.0f );

  mEngine.draw();
}

void BasicTweenApp::prepareSettings( Settings *settings )
{
  mEngine.loadCinderSettings(settings);
}

void BasicTweenApp::touchesBegan( TouchEvent event )
{
  mEngine.touchesBegin(event);
}

void BasicTweenApp::touchesMoved( TouchEvent event )
{
  mEngine.touchesMoved(event);
}

void BasicTweenApp::touchesEnded( TouchEvent event )
{
  mEngine.touchesEnded(event);
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
