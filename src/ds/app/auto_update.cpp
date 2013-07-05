#include "ds/app/auto_update.h"

#include <algorithm>
#include "ds/app/auto_update_list.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class ds::AutoUpdate
 */
AutoUpdate::AutoUpdate(ds::ui::SpriteEngine& se)
	: mOwner(se.getAutoUpdateList())
{
	try {
		mOwner.mUpdate.push_back(this);
	} catch (std::exception const&) {
	}
}

AutoUpdate::~AutoUpdate()
{
	try {
		std::vector<AutoUpdate*>& v(mOwner.mUpdate);
		v.erase(std::remove(v.begin(), v.end(), this), v.end());
	} catch (std::exception const&) {
	}
}

} // namespace ds
