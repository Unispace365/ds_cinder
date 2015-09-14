#pragma once
#ifndef DS_SERVICE_MODEL_MAKER
#define DS_SERVICE_MODEL_MAKER

#include <functional>
#include <Poco/Runnable.h>
#include "model/model_model.h"

#include "service/yaml_load_service.h"

namespace ds {

/**
* \class ds::ModelMaker
*				Loads a compliant yaml data model file into c++ ModelModel models (using yaml_load_service)
				Then spits out c++ headers and implementations to match

* NOTE: this is not a service in the formal ds_cinder sense of a service. More of the concept of a service
*/
class ModelMaker : public Poco::Runnable {
public:
	ModelMaker();

	virtual void				run();
	static std::string			replaceString(std::string &fullString, std::string toReplace, std::string replaceWith);
	static std::string			replaceAllString(std::string& fullString, std::string toReplace, std::string replaceWith);
	std::string					getFileName(const std::string& tableName, const bool isHeader);
	std::string					mYamlFileLocation;

	std::string					mOutputText;
private:
	
	std::stringstream			mOutputStream;
	YamlLoadService				mYamlLoadService;
};

} // namespace ds

#endif