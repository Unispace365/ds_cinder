#ifndef _FULLSTARTER_APP_MODEL_ALLSTORIES_H_
#define _FULLSTARTER_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace fullstarter {

/**
* \class fullstarter::AllData
* This is kind of silly, but If we expand anything I want this here
*/
class AllStories {
public:

	AllStories(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_MODEL_ALLSTORIES_H_