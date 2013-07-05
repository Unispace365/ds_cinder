#include "ds/app/engine/engine_data.h"

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
{
}

} // namespace ds
