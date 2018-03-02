#pragma  once
#ifndef _GENERIC_DATA_MODEL_APP_QUERY_DATA_QUERY_H_
#define _GENERIC_DATA_MODEL_APP_QUERY_DATA_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/data_model.h"

namespace downstream {

/**
* \class downstream::DataQuery
*/
class DataQuery : public Poco::Runnable {
public:
	DataQuery();


	virtual void			run();

	ds::model::DataModelRef readXml();
	void					readXmlNode(ci::XmlTree& tree, ds::model::DataModelRef& parentData, int& id);

	void					getDataFromTable(ds::model::DataModelRef parentModel, const std::string& theTable);
	void					getDataFromTable(ds::model::DataModelRef parentModel, ds::model::DataModelRef tableDescription, const std::string& dbLocation, std::unordered_map<int, ds::Resource>& allResources);

	ds::model::DataModelRef					mData;

	std::string								mLastUpdatedResource;
	std::unordered_map<int, ds::Resource>	mAllResources;
	std::string								mCmsDatabase;
	std::string								mResourceLocation;
	std::string								mXmlDataModel;
};

} // !namespace downstream

#endif //!_GENERIC_DATA_MODEL_APP_QUERY_INDUSTRY_QUERY_H_
