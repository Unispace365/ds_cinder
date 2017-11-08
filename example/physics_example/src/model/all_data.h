#pragma once
#ifndef _PHYSICS_EXAMPLE_APP_MODEL_ALLDATA_H_
#define _PHYSICS_EXAMPLE_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace physics {

/**
 * \class physics::AllData
 */
class AllData {
public:
	
	AllData(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace physics

#endif // !_PHYSICS_EXAMPLE_APP_MODEL_ALLDATA_H_

