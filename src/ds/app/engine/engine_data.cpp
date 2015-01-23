#include "ds/app/engine/engine_data.h"

#include "ds/app/engine/engine_service.h"

namespace ds
{

/**
 * \class ds::EngineData
 */
EngineData::EngineData(const ds::cfg::Settings& engine_settings)
	: mEngineCfg(engine_settings)
	, mMinTouchDistance(10.0f)
	, mMinTapDistance(30.0f)
	, mSwipeQueueSize(4)
	, mSwipeMinVelocity(800.0f)
	, mSwipeMaxTime(0.5f)
	, mDoubleTapTime(0.1f)
	, mFrameRate(60.0f)
{
}

void EngineData::clearServices() {
	if (mServices.empty()) return;

	for (auto it=mServices.begin(), end=mServices.end(); it!=end; ++it) {
		delete it->second;
	}
	mServices.clear();
}

} // namespace ds
