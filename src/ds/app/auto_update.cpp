#include "ds/app/auto_update.h"

#include "ds/app/auto_update_list.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class ds::AutoUpdate
 */
AutoUpdate::AutoUpdate(ds::ui::SpriteEngine &se)
		: mOwner(se.getAutoUpdateList()) {
	try {
		mOwner.addWaiting(this);
	} catch (std::exception const&) {
	}
}

AutoUpdate::~AutoUpdate() {
	try {
		mOwner.remove(this);
	} catch (std::exception const&) {
	}
}

} // namespace ds
