#pragma once
#ifndef _FULLSTARTER_APP_MODEL_ALLDATA_H_
#define _FULLSTARTER_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace fullstarter {

/**
 * \class fullstarter::AllData
 */
class AllData {
public:
	
	AllData(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_MODEL_ALLDATA_H_
