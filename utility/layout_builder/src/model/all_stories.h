#ifndef _LAYOUT_BUILDER_APP_MODEL_ALLSTORIES_H_
#define _LAYOUT_BUILDER_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace layout_builder {

/**
* \class layout_builder::AllData
* This is kind of silly, but If we expand anything I want this here
*/
class AllStories {
public:

	AllStories(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace layout_builder

#endif // !_LAYOUT_BUILDER_APP_MODEL_ALLSTORIES_H_