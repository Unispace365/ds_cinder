#pragma once
#ifndef _NWM COMMUNITY MOSAIC_APP_MODEL_ALLDATA_H_
#define _NWM COMMUNITY MOSAIC_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace nwm {

/**
 * \class nwm::AllData
 */
class AllData {
public:
	
	AllData(){};

	std::vector<ds::model::StoryRef>	mStories;

};

} // !namespace nwm

#endif // !_NWM COMMUNITY MOSAIC_APP_MODEL_ALLDATA_H_
