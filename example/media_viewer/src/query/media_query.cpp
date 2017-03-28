#include "media_query.h"

#include <map>
#include <sstream>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>

#include <ds/app/environment.h>

namespace mv {

/**
 * \class mv::MediaQuery
 */
MediaQuery::MediaQuery() {
}

void MediaQuery::run() {
	mOutput.mAllMedia.clear();
	try {
		query(mOutput);
	} catch (std::exception const&) {
	}
}

void MediaQuery::query(AllData& output) {
	//output.mIndustries.push_back(IndustryModelRef())


}

} // !namespace mv


