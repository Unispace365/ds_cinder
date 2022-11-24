#pragma once
#ifndef DS_APP_AUTOUPDATELIST_H_
#define DS_APP_AUTOUPDATELIST_H_

#include <vector>

namespace ds {
class AutoUpdate;
class UpdateParams;

/**
 * \class AutoUpdateList
 * Store a collection of auto update objects.
 */
class AutoUpdateList {
  public:
	AutoUpdateList();

	void update(const ds::UpdateParams&);

  private:
	void addWaiting(AutoUpdate*);
	void remove(AutoUpdate*);

	friend class AutoUpdate;
	std::vector<AutoUpdate*> mRunning;
	/// A list of pending update objects -- when an update is added,
	/// it starts here, then gets placed in the real update list
	/// at the start of the next update cycle. This prevents a
	/// a bug where the vector gets modified during an update.
	std::vector<AutoUpdate*> mWaiting;
};

} // namespace ds

#endif // DS_APP_AUTOUPDATELIST_H_
