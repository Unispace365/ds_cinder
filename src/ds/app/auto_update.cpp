#include "stdafx.h"

#include "ds/app/auto_update.h"

#include "ds/app/auto_update_list.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class ds::AutoUpdate
 */
AutoUpdate::AutoUpdate(ds::ui::SpriteEngine &e, const int mask)
		: mEngine(e)
		, mMask(mask) {
	try {
		if ((mask&AutoUpdateType::SERVER) != 0) e.getAutoUpdateList(AutoUpdateType::SERVER).addWaiting(this);
		if ((mask&AutoUpdateType::CLIENT) != 0) e.getAutoUpdateList(AutoUpdateType::CLIENT).addWaiting(this);
	} catch (std::exception const&) {
		DS_LOG_ERROR("AutoUpdate() on illegal mask (" << mask << ")");
	}
}

AutoUpdate::~AutoUpdate() {
	try {
		if ((mMask&AutoUpdateType::SERVER) != 0) mEngine.getAutoUpdateList(AutoUpdateType::SERVER).remove(this);
		if ((mMask&AutoUpdateType::CLIENT) != 0) mEngine.getAutoUpdateList(AutoUpdateType::CLIENT).remove(this);
	} catch (std::exception const&) {
	}
}

} // namespace ds
