#ifndef _PANORAMICVIDEO_APP_MODEL_ALLDATA_H_
#define _PANORAMICVIDEO_APP_MODEL_ALLDATA_H_

#include <ds/data/resource.h>

namespace panoramic {

/**
 * \class panoramic::AllData
 */
class AllData {
public:
	
	AllData(){};

	std::vector<ds::Resource>		mAllVideos;

};

} // !namespace panoramic

#endif // !_PANORAMICVIDEO_APP_MODEL_ALLDATA_H_


