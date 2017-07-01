#include "story_query.h"

#include <map>
#include <sstream>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>

#include <ds/app/environment.h>

namespace fullstarter {

/**
 * \class fullstarter::StoryQuery
 */
StoryQuery::StoryQuery() {
}

void StoryQuery::run() {
	mOutput.mStories.clear();
	try {
		query(mOutput);
	} catch (std::exception const&) {
	}
}

void StoryQuery::query(AllData& output) {
	// Sample to hard-code data:
	ds::model::StoryRef newStory;
	newStory.setId(1);
	newStory.setTitle(L"full_starter");
	newStory.setBody(L"Sample body");
	newStory.setPrimaryResource(ds::Resource(ds::Environment::expand("%APP%/data/images/temp/sample_image.png")));
	output.mStories.push_back(newStory);



	// Sample to query from a db.
	// You'll need to set resource_location and resource_db in engine.xml for this to work (or supply your own db path)
	// resource_db is a relative path to resource_location
	/* 

	const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
	ds::query::Result result;
	ds::query::Result recResult;
	std::string dbPath = cms.getDatabasePath();
	std::string resourcesPath = cms.getResourcePath();

	std::map<int, ds::Resource> allResources;
	//									0			1				2				3				4				5					6			7
	std::string recyQuery = "SELECT resourcesid, resourcestype,resourcesduration,resourceswidth,resourcesheight,resourcesfilename,resourcespath,resourcesthumbid FROM Resources";
	if(ds::query::Client::query(dbPath, recyQuery, recResult)) {
		ds::query::Result::RowIterator	rit(recResult);
		while(rit.hasValue()) {
			ds::Resource reccy;
			int mediaId = rit.getInt(0);
			reccy.setDbId(mediaId);
			reccy.setTypeFromString(rit.getString(1));
			reccy.setDuration(rit.getFloat(2));
			reccy.setWidth(rit.getFloat(3));
			reccy.setHeight(rit.getFloat(4));
			if(reccy.getType() == ds::Resource::WEB_TYPE){
				reccy.setLocalFilePath(rit.getString(5));
			} else {
				std::stringstream loclPath;
				loclPath << resourcesPath << rit.getString(6) << rit.getString(5);
				reccy.setLocalFilePath(loclPath.str());
			}
			reccy.setThumbnailId(rit.getInt(7));
			allResources[mediaId] = reccy;
			++rit;
		}
		}

	*/
}

} // !namespace fullstarter
