<<<<<<< HEAD
#pragma once
#ifndef DS_APP_AUTOUPDATE_H_
#define DS_APP_AUTOUPDATE_H_

#include <vector>
#include <Poco/Timestamp.h>

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
	AutoUpdate(ds::ui::SpriteEngine&);
	virtual ~AutoUpdate();

protected:
	virtual void		update(const ds::UpdateParams&) = 0;

private:
	friend class AutoUpdateList;
	AutoUpdate();

	AutoUpdateList&		mOwner;
};

} // namespace ds

=======
#pragma once
#ifndef DS_APP_AUTOUPDATE_H_
#define DS_APP_AUTOUPDATE_H_

#include <vector>
#include <Poco/Timestamp.h>

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
	AutoUpdate(ds::ui::SpriteEngine&);
	virtual ~AutoUpdate();

protected:
	virtual void		update(const ds::UpdateParams&) = 0;

private:
	friend class AutoUpdateList;
	AutoUpdate();

	AutoUpdateList&		mOwner;
};

} // namespace ds

>>>>>>> origin/master
#endif // DS_APP_AUTOUPDATE_H_