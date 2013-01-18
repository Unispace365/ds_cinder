#include "ds/app/auto_update_list.h"

#include <Poco/Timestamp.h>
#include "ds/app/auto_update.h"

namespace ds {

/**
 * \class ds::AutoUpdateList
 */
AutoUpdateList::AutoUpdateList()
{
  mUpdate.reserve(16);
}

void AutoUpdateList::update()
{
  if (mUpdate.empty()) return;

  const Poco::Timestamp::TimeVal t(Poco::Timestamp().epochMicroseconds());
  for (auto it=mUpdate.begin(), end=mUpdate.end(); it != end; ++it) {
    (*it)->update(t);
  }
}

} // namespace ds
