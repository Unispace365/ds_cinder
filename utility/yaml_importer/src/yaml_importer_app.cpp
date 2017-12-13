#include <cinder/app/App.h>

#include <cinder/app/FileDropEvent.h>
#include <cinder/app/RendererGl.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/text.h>

#include <Poco/File.h>
#include <Poco/Path.h>

#include "service/model_maker.h"

namespace ds{

class YamlImporterApp : public ds::App {
public:
	class ds::ui::Text;

	YamlImporterApp();

	void				setupServer();	
	
	//! Override to receive file-drop events.	
	virtual void		fileDrop(ci::app::FileDropEvent event);
	void parseFile(std::string fileLocation);
private:
	typedef ds::App		inherited;
	ds::ui::Text*	mMessage;

};



YamlImporterApp::YamlImporterApp(){
	// Fonts links together a font name and a physical font file
	// Then the "text.xml" and TextCfg will use those font names to specify visible settings (size, color, leading)
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachSetting([this](ds::cfg::Settings::Setting& theSetting){
		mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
	});
}

void YamlImporterApp::setupServer()
{
	mEngine.loadTextCfg("text.xml");

	std::stringstream startMessage;
	startMessage << "Drop a yml file onto this window to generate .h and .cpp files for that model.";

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...

	//mMessage = mEngine.getEngineCfg().getText("sample:config").createMultiline(mEngine, &rootSprite);	
	mMessage = new ds::ui::Text(mEngine);
	rootSprite.addChildPtr(mMessage);

	if(mMessage){
		auto cfg = mEngine.getEngineCfg().getText("sample:config");
		cfg.configure(*mMessage);
		mMessage->setPosition(150.0f, 150.0f);
		mMessage->setText(startMessage.str());
	}
// 	ModelMaker mm;
// 	//mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/stories.yml");
// 	mm.mYamlFileLocation = ds::Environment::expand("%APP%/data/general.yml");
// 	mm.run();


	std::vector<std::string> theArgs = ci::app::App::get()->getCommandLineArgs();// getArgs();
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
CINDER_APP(ds::YamlImporterApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })


