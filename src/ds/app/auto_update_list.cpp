#include "ds/app/auto_update_list.h"

#include <Poco/Timestamp.h>
#include "ds/app/auto_update.h"
#include "ds/params/update_params.h"

namespace ds {

/**
 * \class ds::AutoUpdateList
 */
AutoUpdateList::AutoUpdateList()
{
  mUpdate.reserve(16);
}

void AutoUpdateList::update( const ds::UpdateParams &updateParams )
{
  if (mUpdate.empty()) return;

  for (auto it=mUpdate.begin(), end=mUpdate.end(); it != end; ++it) {
    (*it)->update(updateParams);
  }
}

} // namespace ds
