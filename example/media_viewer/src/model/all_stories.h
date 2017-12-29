#ifndef _MEDIAVIEWER_APP_MODEL_ALLSTORIES_H_
#define _MEDIAVIEWER_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace mv {

/**
* \class mv::AllData
* This is kind of silly, but If we expand anything I want this here
*/
class AllStories {
public:

	AllStories(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace mv

#endif // !_MEDIAVIEWER_APP_MODEL_ALLSTORIES_H_

