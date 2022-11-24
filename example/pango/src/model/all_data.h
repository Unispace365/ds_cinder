#pragma once
#ifndef _PANGO_APP_MODEL_ALLDATA_H_
#define _PANGO_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace pango {

/**
 * \class pango::AllData
 */
class AllData {
  public:
	AllData(){};

	std::vector<ds::model::StoryRef> mStories;
};

} // namespace pango

#endif // !_PANGO_APP_MODEL_ALLDATA_H_
