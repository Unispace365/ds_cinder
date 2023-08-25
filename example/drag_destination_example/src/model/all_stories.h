#ifndef _DRAG_DESTINATION_EXAMPLE_APP_MODEL_ALLSTORIES_H_
#define _DRAG_DESTINATION_EXAMPLE_APP_MODEL_ALLSTORIES_H_

#include "model/generated/story_model.h"

namespace example {

/**
 * \class example::AllData
 * This is kind of silly, but If we expand anything I want this here
 */
class AllStories {
  public:
	AllStories(){};

	std::vector<ds::model::StoryRef> mStories;
};

} // namespace example

#endif // !_DRAG_DESTINATION_EXAMPLE_APP_MODEL_ALLSTORIES_H_