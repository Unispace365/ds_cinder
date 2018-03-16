#include "https_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/text.h>

#include <cinder/Rand.h>
#include <cinder/app/RendererGl.h>

namespace example {

https_example::https_example()
	: mHttpsRequest(mEngine)
{


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");

	registerKeyPress("Paste http(s) request link", [this] {auto fileNameOrig = ds::Environment::getClipboard();	mHttpsRequest.makeGetRequest(fileNameOrig, false, false); }, ci::app::KeyEvent::KEY_v, false, true);
	registerKeyPress("Sample post data", [this] {
		std::string datay = "{ \"data\": { \"type\": \"collection_links\", \"attributes\": { \"story_type\": \"Achievement\", \"story_id\": \"13\" }}}";
		std::vector<std::string> headers;
		headers.push_back("Accept: application/json");
		headers.push_back("Content-Type: application/json");
		mHttpsRequest.makePostRequest("https://example.com", datay, true, true, "", headers);
	}, ci::app::KeyEvent::KEY_l);
}

void https_example::setupServer() {

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

	ds::ui::Text* descriptionText = new ds::ui::Text(mEngine);
	descriptionText->setFont("Arial", 24);
	descriptionText->setPosition(100.0f, 100.0f);
	descriptionText->setText("See the console for https request output. <br>Press L to post a new request. <br>Control-v to test a link with a get request.");
	mEngine.getRootSprite().addChildPtr(descriptionText);


	// You can also control-v (aka paste) a link into the app and see the reply it sends
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::https_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
