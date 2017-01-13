#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/cfg/cfg_text.h>
#include <ds/ui/sprite/text.h>

using namespace ci;

class TextApp : public ds::App {
public:
	TextApp();

	void				setupServer();

private:
	typedef ds::App		inherited;
};

TextApp::TextApp() {
	enableCommonKeystrokes();
}

void TextApp::setupServer() {
	ds::ui::Sprite&										root = mEngine.getRootSprite();
	// add sprites...

	// For convenience, define one or more lines that associate a text config with a line of text.
	typedef std::pair<ds::cfg::Text, std::string>		Entry;
	std::vector<Entry>									vec;
	vec.push_back(Entry(ds::cfg::Text(ds::Environment::getAppFile("data/fonts/MaureaJci_RgL__reg.ttf"), 30.0f, 1.0f, ci::ColorA(1, 1, 1, 1)),
						"Get outta town"));
	float			x = 10.0f, y = 10.0f;
	for (auto it=vec.begin(), end=vec.end(); it!=end; ++it) {
		ds::ui::Text&		s = it->first.createOrThrow(mEngine, &root);
		s.setText(it->second);
		s.setPosition(x, y);
		y += s.getHeight() + 10.0f;
	}
}

// This line tells Cinder to actually create the application
CINDER_APP( TextApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_8) )
