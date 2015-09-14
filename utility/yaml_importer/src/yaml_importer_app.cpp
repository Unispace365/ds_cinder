#include <cinder/app/AppBasic.h>

#include <cinder/app/FileDropEvent.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>

#include <ds/ui/sprite/multiline_text.h>

#include <ds/ui/sprite/text_layout.h>

#include <Poco/File.h>
#include <Poco/Path.h>

#include "service/model_maker.h"

namespace ds{

class YamlImporterApp : public ds::App {
public:
	YamlImporterApp();

	void				setupServer();	
	
	//! Override to receive file-drop events.	
	virtual void		fileDrop(ci::app::FileDropEvent event);
	void parseFile(std::string fileLocation);
private:
	typedef ds::App		inherited;
	ds::ui::MultilineText*	mMessage;

};



YamlImporterApp::YamlImporterApp(){
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Regular.ttf"), "noto-regular");
}

void YamlImporterApp::setupServer()
{
	mEngine.loadTextCfg("text.xml");

	std::stringstream startMessage;
	startMessage << "Drop a yml file onto this window to generate .h and .cpp files for that model.";

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...

	mMessage = mEngine.getEngineCfg().getText("sample:config").createMultiline(mEngine, &rootSprite);
	if(mMessage){
		mMessage->setPosition(150.0f, 150.0f);
		mMessage->setText(startMessage.str());
	}
// 	ModelMaker mm;
// 	//mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/stories.yml");
// 	mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/general.yml");
// 	mm.run();


	std::vector<std::string> theArgs = getArgs();
	for(auto it = theArgs.begin(); it < theArgs.end(); ++it){
		Poco::File filey = Poco::File((*it));
		if(filey.exists() && !filey.isDirectory()){
			Poco::Path pathy = Poco::Path(filey.path());
			if(pathy.getExtension() == ".yml" || pathy.getExtension() == "yml"){
				parseFile(filey.path());
			}
		}

	}
}

void YamlImporterApp::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		parseFile((*it).string());
	}
}

void YamlImporterApp::parseFile(std::string fileLocation){
	ModelMaker mm;
	mm.mYamlFileLocation = fileLocation;

	if(mMessage){
		std::stringstream ss;
		ss << "Loading yaml file: " << mm.mYamlFileLocation;
		mMessage->setText(ss.str());
	}

	mm.run();

	if(mMessage){
		std::stringstream ss;
		ss << "Finished loading file: " << mm.mYamlFileLocation << std::endl;
		ss << mm.mOutputText;
		mMessage->setText(ss.str());
	}

}

}
// This line tells Cinder to actually create the application
CINDER_APP_BASIC(ds::YamlImporterApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))
