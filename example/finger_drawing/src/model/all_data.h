#pragma once
#ifndef _FINGER_DRAWING_APP_MODEL_ALLDATA_H_
#define _FINGER_DRAWING_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace example {

/**
 * \class example::AllData
 */
class AllData {
public:
	
	AllData(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace example

#endif // !_FINGER_DRAWING_APP_MODEL_ALLDATA_H_
