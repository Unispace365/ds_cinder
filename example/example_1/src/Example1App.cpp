#include <cinder/app/AppBasic.h>
#include <cinder/Timeline.h>

#include <ds/app/engine.h>

#include <ds/thread/runnable_client.h>
#include <ds/query/query_client.h>
#include <ds/config/settings.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class BasicTweenApp : public AppBasic {
  public:
	BasicTweenApp();

	void				setup();
	void				mouseDown( MouseEvent event );
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
		console() << "I'm doing stuff!!" << endl;
	}
};

BasicTweenApp::BasicTweenApp()
	: mClient(mEngine)
	,  mQuery(mEngine)
{
	mClient.setResultHandler([](std::unique_ptr<Poco::Runnable>&){ cout << "I could do something" << endl; });
	mQuery.setResultHandler([](const ds::query::Result& r, ds::query::Talkback&){ cout << "query finished rowSize=" << r.getRowSize() << endl; });

//  ds::cfg::Settings   settings;
//  settings.readFrom("C:\\Users\\erich\\Desktop\\work\\git\\northeastern_wall\\bin\\data\\layout_ew.xml");
//  cout << "SETTINGS COLOR COUNT=" << settings.getColorSize("row_bg") << endl;
}

void BasicTweenApp::setup()
{	
	mBlackPos = mWhitePos = getWindowCenter();
}

void BasicTweenApp::mouseDown( MouseEvent event )
{
	mClient.run(std::unique_ptr<Poco::Runnable>(new MessageRunnable()));
	mQuery.runAsync("C:\\Users\\erich\\Documents\\downstream\\northeastern\\db\\northeastern.sqlite", "SELECT * FROM person");

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
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
