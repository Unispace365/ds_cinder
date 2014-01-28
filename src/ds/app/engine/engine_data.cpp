#include "ds/app/engine/engine_data.h"

#include "ds/app/engine/engine_service.h"

namespace ds
{

/**
 * \class ds::EngineData
 */
EngineData::EngineData()
	: mMinTouchDistance(10.0f)
	, mMinTapDistance(30.0f)
	, mSwipeQueueSize(4)
	, mDoubleTapTime(0.1f)
	, mFrameRate(60.0f)
	, mCameraDirty(false)
{
}

void EngineData::clearServices()
{
	if (mServices.empty()) return;

	for (auto it=mServices.begin(), end=mServices.end(); it!=end; ++it) {
		delete it->second;
	}
	mServices.clear();
}

} // namespace ds
