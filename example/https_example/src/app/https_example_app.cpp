#include "https_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>


#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/story/story_view.h"

namespace example {

https_example::https_example()
	: inherited(ds::RootList()

	// Note: this is where you'll customize the root list
								.ortho() 
								.pickColor()

								.persp() 
								.perspFov(60.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 10.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mHttpsRequest(mEngine)
{


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");

	enableCommonKeystrokes(true);
}

void https_example::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();

	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	rootSprite.addChildPtr(new StoryView(mGlobals));



	/* ---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
		Here's the important parts!
		---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
		---------------------------------------------------------------------------------
	*/

	// The https request class handles threading for you
	// So you set the reply function, and it'll come back on the main thread when the request returns sometime later
	// Errored means something bad happened, and the reply will contain the curl error
	// If errored = false, the reply is whatever the server sent back
	mHttpsRequest.setReplyFunction([this](const bool errored, const std::string& reply, const long httpCode){
		// Handle errors
		std::cout << "Https request reply: " << errored << " " << reply << std::endl << std::endl << "http status: " << httpCode << std::endl;
	});

	// Make an example request. 
	// The reply function above will be called when this succeeds or not
	// The two boolean functions turn down the secure-ness to allow connection to invalid SSL stuff
	mHttpsRequest.makeGetRequest("https://example.com", false, false);


	// You can also control-v (aka paste) a link into the app and see the reply it sends
}

void https_example::update() {
	inherited::update();

	if( mEngine.isIdling() && !mIdling ){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify( IdleStartedEvent() );
	} else if ( !mEngine.isIdling() && mIdling ){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify( IdleEndedEvent() );
	}

}

void https_example::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();

	} else if(event.getCode() == KeyEvent::KEY_v && event.isControlDown()){
		auto fileNameOrig = ds::Environment::getClipboard();
		mHttpsRequest.makeGetRequest("https://example.com", false, false);


	} else if(event.getCode() == KeyEvent::KEY_p){
		std::string datay = "{ \"data\": { \"type\": \"collection_links\", \"attributes\": { \"story_type\": \"Achievement\", \"story_id\": \"13\" }}}";
		std::vector<std::string> headers;
		headers.push_back("Accept: application/json");
		headers.push_back("Content-Type: application/json");
		mHttpsRequest.makePostRequest("https://example.com", datay, true, true, "", headers);

	// Shows all enabled sprites with a label for class type
	} else if(event.getCode() == KeyEvent::KEY_f){

		const int numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(int i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
					sprite.setTransparent(false);
					sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
					sprite.setOpacity(0.95f);

					ds::ui::Text* labelly = mGlobals.getText("media_viewer:title").create(mEngine, &sprite);
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	}
}

void https_example::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void https_example::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void https_example::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void https_example::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());
	}
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::https_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))
