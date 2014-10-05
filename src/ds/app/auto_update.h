#pragma once
#ifndef DS_APP_AUTOUPDATE_H_
#define DS_APP_AUTOUPDATE_H_

#include <vector>
#include <Poco/Timestamp.h>
#include <ds/app/app_defs.h>

namespace ds {
class AutoUpdateList;
class UpdateParams;
namespace ui {
class SpriteEngine;
}

/**
 * \class ds::AutoUpdate
 * Automatically run an update operation. Handle managing myself
 * in my containing list.
 */
class AutoUpdate {
public:
	AutoUpdate(ds::ui::SpriteEngine&, const int mask = AutoUpdateType::SERVER);
	virtual ~AutoUpdate();

protected:
	friend class			AutoUpdateList;
	virtual void			update(const ds::UpdateParams&) = 0;

	ds::ui::SpriteEngine&	mEngine;

private:
	AutoUpdate();

	const int				mMask;
};

} // namespace ds

#endif // DS_APP_AUTOUPDATE_H_