#include "ds/app/auto_draw.h"

#include <algorithm>
#include "ds/app/auto_update_list.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class ds::AutoDraw
 */
AutoDraw::AutoDraw(ds::ui::SpriteEngine& se)
		: mOwner(se.getService<AutoDrawService>("AUTODRAW")) {
	try {
		mOwner.mUpdate.push_back(this);
	} catch (std::exception const&) {
	}
}

AutoDraw::~AutoDraw() {
	try {
		std::vector<AutoDraw*>& v(mOwner.mUpdate);
		v.erase(std::remove(v.begin(), v.end(), this), v.end());
	} catch (std::exception const&) {
	}
}

/**
 * \class ds::AutoDrawService
 */
AutoDrawService::AutoDrawService() {
	mUpdate.reserve(16);
}

void AutoDrawService::drawClient(const ci::mat4& t, const DrawParams& d) {
	if (mUpdate.empty()) return;

	for (auto it=mUpdate.begin(), end=mUpdate.end(); it != end; ++it) {
		(*it)->drawClient(t, d);
	}
}
} // namespace ds
