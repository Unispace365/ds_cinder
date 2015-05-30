#include "story_query.h"

#include <map>
#include <sstream>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>

#include <ds/app/environment.h>

namespace example {

/**
 * \class example::StoryQuery
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

void StoryQuery::query(AllStories& output) {
	// sane people might query from a database here or something

	std::wstringstream ss;
	for(int i = 0; i < 100; i++){
		ds::model::StoryRef story;
		story.setId(i);
		
		ss.str(L"");
		if(i % 5 == 0){
			ss << L"Harf! ";
		}

		if(i % 7 == 0){
			ss << L"Buzz! ";
		}
		ss << L"Fantastic story " << i;

		story.setName(ss.str());
		output.mStories.push_back(story);
	}



}

} // !namespace example
