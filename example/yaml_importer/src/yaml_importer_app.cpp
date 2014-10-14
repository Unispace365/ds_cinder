#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>

#include <ds/ui/sprite/multiline_text.h>

#include <ds/ui/sprite/text_layout.h>

#include "service/model_maker.h"

namespace ds{

class YamlImporterApp : public ds::App {
public:
	YamlImporterApp();

	void				setupServer();

private:
	typedef ds::App		inherited;

};



YamlImporterApp::YamlImporterApp(){
}

void YamlImporterApp::setupServer()
{
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...

	ModelMaker mm;
	//mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/stories.yml");
	mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/general.yml");
	mm.run();
}
}
// This line tells Cinder to actually create the application
CINDER_APP_BASIC(ds::YamlImporterApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))
