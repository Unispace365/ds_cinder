#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>

#include "yaml-cpp/yaml.h"


class YamlImporterApp : public ds::App {
public:
	YamlImporterApp();

	void				parseNode(YAML::Node doc, const int level);
	void				setupServer();

private:
	typedef ds::App   inherited;
};

void YamlImporterApp::parseNode(YAML::Node doc, const int level){
	if(!doc) return;
	for(int i = 0; i < level; i++){
		std::cout << "\t";
	}
	switch(doc.Type()) {
		case YAML::NodeType::Null: // ...
			std::cout << "Null " << std::endl;
			break;
		case YAML::NodeType::Scalar: // ...
			std::cout << "Scalar: " << doc.as<std::string>() << std::endl;
			break;
		case YAML::NodeType::Sequence: // ...
			for(auto dit = doc.begin(); dit != doc.end(); ++dit) {
				parseNode((*dit), level + 1);
			}
			//std::cout << "Sequence " << std::endl;
			break;
		case YAML::NodeType::Map: // ...
			//std::cout << "Map " << std::endl;
			for(auto dit = doc.begin(); dit != doc.end(); ++dit) {
				std::string key, value;
				key = (*dit).first.as<std::string>();
				std::cout << std::endl;
				for(int i = 0; i < level; i++){
					std::cout << "\t";
				}
				std::cout << "Map Key: " << key << std::endl;

				YAML::Node node = (*dit).second;
				parseNode(node, level + 1);
			}
			break;
		case YAML::NodeType::Undefined: // ...
			std::cout << "Undefined " << std::endl;
			break;
	}
}

YamlImporterApp::YamlImporterApp(){
	std::ifstream iff("stories.yml");
	std::vector<YAML::Node> nodes = YAML::LoadAll(iff);
	for(auto it = nodes.begin(); it < nodes.end(); ++it){
		parseNode((*it), 0);
	}
}

void YamlImporterApp::setupServer()
{
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	// add sprites...
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(YamlImporterApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))
