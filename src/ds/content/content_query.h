#pragma once
#ifndef DS_CONTENT_CONTENT_QUERY
#define DS_CONTENT_CONTENT_QUERY

#include <Poco/Runnable.h>

#include <ds/content/content_model.h>
#include <ds/query/query_result.h>

#include <functional>

namespace ds {

/**
 * \class ContentQuery
 * \brief Reads a content model xml descriptor, queries a Sqlite database from that descriptor, and assembles the data
 * model
 */
class ContentQuery : public Poco::Runnable {
  public:
	ContentQuery();

	void run() override;

	void assembleModels(model::ContentModelRef tablesParent);
	void updateResourceCache();

	model::ContentModelRef readXml();
	static void			   readXmlNode(ci::XmlTree& tree, model::ContentModelRef& parentData, int& id);

	void getDataFromTable(model::ContentModelRef parentModel, const std::string& theTable) const;
	void getDataFromTable(model::ContentModelRef parentModel, const model::ContentModelRef& tableDescription, const std::string& dbLocation,
						  std::unordered_map<int, Resource>& allResources, int depth, int parentModelId);

	const model::ContentModelRef& getData() const { return mData; }

	void setCmsDatabase(const std::string& cmsDatabase) { mCmsDatabase = cmsDatabase; }
	void setResourceLocation(const std::string& resourceLocation) { mResourceLocation = resourceLocation; }
	void setXmlDataModel(const std::string& xmlDataModel) { mXmlDataModel = xmlDataModel; }

  private:
	model::ContentModelRef mData;

	std::string						  mLastUpdatedResource;
	std::unordered_map<int, Resource> mAllResources;

	bool										 mCheckUpdatedResources = true;
	std::unordered_map<std::string, std::string> mResourceRemap			= {
		{"table_name", "resources"},   {"id", "resourcesid"},		  {"type", "resourcestype"},		 {"duration", "resourcesduration"},
		{"width", "resourceswidth"},   {"height", "resourcesheight"}, {"filename", "resourcesfilename"}, {"path", "resourcespath"},
		{"thumb", "resourcesthumbid"}, {"updated", "updated_at"}};

	std::string mCmsDatabase;
	std::string mResourceLocation;
	std::string mXmlDataModel;

	int mTableId;
};

} // namespace ds

#endif
