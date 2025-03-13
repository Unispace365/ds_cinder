#pragma once
#ifndef DS_APP_AUTOUPDATE_H_
#define DS_APP_AUTOUPDATE_H_

#include <ds/app/app_defs.h>

namespace ds {
class AutoUpdateList;
class UpdateParams;
namespace ui {
	class SpriteEngine;
}

/**
 * \class AutoUpdate
 * Automatically run an update operation. Handle managing myself
 * in my containing list.
 */
class AutoUpdate {
  public:
	AutoUpdate(ui::SpriteEngine&, int mask = AutoUpdateType::SERVER);
	virtual ~AutoUpdate();

	AutoUpdate(const AutoUpdate&)			 = delete;
	AutoUpdate(AutoUpdate&&)				 = delete;
	AutoUpdate& operator=(const AutoUpdate&) = delete;
	AutoUpdate& operator=(AutoUpdate&&)		 = delete;

  protected:
	friend class AutoUpdateList;
	virtual void update(const UpdateParams&) = 0;

	ui::SpriteEngine& mEngine;

  private:
	const int mMask;
};

} // namespace ds

#endif // DS_APP_AUTOUPDATE_H_
