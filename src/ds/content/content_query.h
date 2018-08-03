#pragma  once
#ifndef DS_CONTENT_CONTENT_QUERY
#define DS_CONTENT_CONTENT_QUERY

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "content_model.h"

namespace ds {

/**
* \class ContentQuery
* \brief Reads a content model xml descriptor, queries a Sqlite database from that descriptor, and assembles the data model
*/
class ContentQuery : public Poco::Runnable {
public:

	ContentQuery();

	virtual void							run();

	void									assembleModels(ds::model::ContentModelRef tablesParent);
	void									updateResourceCache();

	ds::model::ContentModelRef				readXml();
	void									readXmlNode(ci::XmlTree& tree, ds::model::ContentModelRef& parentData, int& id);

	void									getDataFromTable(ds::model::ContentModelRef parentModel, const std::string& theTable);
	void									getDataFromTable(ds::model::ContentModelRef parentModel, ds::model::ContentModelRef tableDescription, const std::string& dbLocation, std::unordered_map<int, ds::Resource>& allResources, const int depth, const int parentModelId);

	ds::model::ContentModelRef				mData;

	std::string								mLastUpdatedResource;
	std::unordered_map<int, ds::Resource>	mAllResources;
	std::string								mCmsDatabase;
	std::string								mResourceLocation;
	std::string								mXmlDataModel;

	int										mTableId;
};

} 

#endif
