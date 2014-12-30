#ifndef _PERSPECTIVEPICKING_APP_MODEL_ALLSTORIES_H_
#define _PERSPECTIVEPICKING_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace perspective_picking {

/**
* \class perspective_picking::AllData
* This is kind of silly, but If we expand anything I want this here
*/
class AllStories {
public:

	AllStories(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace perspective_picking

#endif // !_PERSPECTIVEPICKING_APP_MODEL_ALLSTORIES_H_