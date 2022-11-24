#ifndef _MEDIAVIEWER_APP_MODEL_ALLDATA_H_
#define _MEDIAVIEWER_APP_MODEL_ALLDATA_H_

#include "model/generated/media_model.h"

namespace mv {

/**
 * \class mv::AllData
 */
class AllData {
  public:
	AllData(){};

	std::vector<ds::model::MediaRef> mAllMedia;
};

} // namespace mv

#endif // !_MEDIAVIEWER_APP_MODEL_ALLDATA_H_
