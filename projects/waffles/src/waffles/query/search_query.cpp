#include "stdafx.h"

#include "search_query.h"

#include <algorithm>
#include <map>
#include <sstream>

#include <Poco/Path.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>
#include <waffles/util/waffles_helper.h>

#include "app/waffles_app_defs.h"

namespace waffles {

bool alphaSort(ds::model::ContentModelRef& a, ds::model::ContentModelRef& b) {
	std::string upperA = a.getName();
	std::transform(upperA.begin(), upperA.end(), upperA.begin(), ::toupper);
	std::string upperB = b.getName();
	std::transform(upperB.begin(), upperB.end(), upperB.begin(), ::toupper);

	return upperA < upperB;
}

SearchQuery::SearchQuery() {
}

void SearchQuery::setInput(ds::ui::SpriteEngine& eng, const std::string& leQuery, const std::string& filterType,
						   const int& resourceFilter) {
	mEngine			= &eng;
	mInput			= leQuery;
	mFilterType		= filterType;
	mResourceFilter = resourceFilter;

	std::transform(mInput.begin(), mInput.end(), mInput.begin(), ::toupper);
}

void SearchQuery::run() {
	mOutput.clear();
	try {
		queryGeneral();
	} catch (std::exception const&) {}
}

void SearchQuery::queryGeneral() {
	auto helper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper>();
	// Add all media items matching the search query to the output
	if (mFilterType.empty() || mFilterType == "record" || mFilterType == "media") {
		//auto allValid = mEngine->mContent.getKeyReferences(ds::model::VALID_MAP);
		auto allValid = helper->getContentForPlatform();
		for (auto item : allValid) {
			recursiveMatch(item);
		}

		std::sort(mOutput.begin(), mOutput.end(), [](auto& a, auto& b) { return alphaSort(a, b); });
	}
}


void SearchQuery::recursiveMatch(ds::model::ContentModelRef item) {
	for (auto child : item.getChildren()) {
		recursiveMatch(child);
	}
	auto helper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper>();
	
	if (mFilterType == "media" || mFilterType.empty()) {
		if (helper->isValidMedia(item, "waffles!")) {
			auto mediaKey = helper->getMediaPropertyKey(item, "waffles!");
			if (mediaKey.empty()) mediaKey = "media";

			if (!item.getPropertyResource(mediaKey).empty()) {

				bool doResourceFilter = mResourceFilter != 0;
				bool matchesFilter = true;

				if (doResourceFilter) {

					matchesFilter = item.getPropertyResource(mediaKey).getType() == mResourceFilter;
					if (mResourceFilter == ds::Resource::VIDEO_TYPE) {
						matchesFilter |= item.getPropertyResource(mediaKey).getType() == ds::Resource::YOUTUBE_TYPE;
					}
				}

				if (!matchesFilter) return;

				auto name = item.getPropertyString("record_name");
				ds::to_uppercase(name);

				name = item.getPropertyResource(mediaKey).getFileName();
				ds::to_uppercase(name);
				if (name.find(mInput) != std::string::npos) {
					auto fake = item.duplicate();
					fake.setProperty("type_key", std::string("media"));
					fake.setProperty("type_uid", std::string("media"));
					fake.setProperty("record_name", item.getPropertyString("record_name") + " (" +
						item.getPropertyResource("media").getFileName() + ")");
					mOutput.push_back(fake);
					return;
				}
			}
		}
	} 
	if (mFilterType == "record" || mFilterType.empty()) {
		if (helper->isValidPlaylist(item, "presentation")) {
			auto name = item.getPropertyString("record_name");
			ds::to_uppercase(name);
			if (name.find(mInput) != std::string::npos) {

				auto fake = item.duplicate();
				//fake.setProperty("type_key", std::string("record"));
				//fake.setProperty("type_uid", std::string("record"));
				mOutput.push_back(fake);
				return;
			}
		}
	}
}


} // namespace waffles
