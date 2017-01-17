#include <cinder/app/App.h>

#include <cinder/app/FileDropEvent.h>

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
	
	//! Override to receive file-drop events.	
	virtual void		fileDrop(ci::app::FileDropEvent event);

private:
	typedef ds::App		inherited;

};



YamlImporterApp::YamlImporterApp(){
}

void YamlImporterApp::setupServer()
{
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...

// 	ModelMaker mm;
// 	//mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/stories.yml");
// 	mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/general.yml");
// 	mm.run();
}

void YamlImporterApp::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		ModelMaker mm;
		mm.mYamlFileLocation = (*it).string();
		mm.run();
	}
}

}
// This line tells Cinder to actually create the application
CINDER_APP(ds::YamlImporterApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))


