#pragma once
#ifndef DS_SERVICE_YAML_LOAD_SERVICE
#define DS_SERVICE_YAML_LOAD_SERVICE

#include <functional>
#include <Poco/Runnable.h>
#include "model/model_model.h"

#include "yaml-cpp/yaml.h"

namespace ds {

/**
* \class ds::YamlLoadService
*				Loads a compliant yaml data model file into c++ ModelModel models

* NOTE: this is not a service in the formal ds_cinder sense of a service. More of the concept of a service
*/
class YamlLoadService : public Poco::Runnable {
public:
	YamlLoadService();

	virtual void				run();

	std::string					mFileLocation;
	std::vector<ModelModel>		mOutput;

private:
	void						printYamlRecursive(YAML::Node doc, const int level);
	void						parseTable(YAML::Node doc);
	void						parseColumn(YAML::Node mappedNode, ModelModel& modelModel);

	bool						parseBool(const std::string& value);
};

} // namespace ds

#endif