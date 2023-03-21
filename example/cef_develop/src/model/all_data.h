#ifndef _CEFDEVELOP_APP_MODEL_ALLDATA_H_
#define _CEFDEVELOP_APP_MODEL_ALLDATA_H_

#include "model/generated/story_model.h"

namespace cef {

/**
 * \class cef::AllData
 */
class AllData {
  public:
	AllData(){};

	std::vector<ds::model::StoryRef> mStories;
};

} // namespace cef

#endif // !_CEFDEVELOP_APP_MODEL_ALLDATA_H_
