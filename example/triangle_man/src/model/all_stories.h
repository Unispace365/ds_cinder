#ifndef _TRIANGLE_MAN_APP_MODEL_ALLSTORIES_H_
#define _TRIANGLE_MAN_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace nwm {

/**
* \class nwm::AllData
* This is kind of silly, but If we expand anything I want this here
*/
class AllStories {
public:

	AllStories(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace nwm

#endif // !_TRIANGLE_MAN_APP_MODEL_ALLSTORIES_H_

