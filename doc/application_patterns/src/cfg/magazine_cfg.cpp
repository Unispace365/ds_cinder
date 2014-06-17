#include "na/ui/magazine/magazine_cfg.h"

#include <xml/settings.h>

namespace ui {
namespace magazine {

/**
* \class ui::magazine::Cfg
*/
Cfg::Cfg()
  : mSnapToPage(false)
  , mSnapCutoff(2.0f)
  , mSnapWeight(0.15f)
{
}

Cfg::~Cfg()
{
}

/**
* \class ui::magazine::CfgKeys
*/
CfgKeys::CfgKeys( const std::string& snapToPage, const std::string& snapCutoff, const std::string& snapWeight,
                  const std::string& previousId, const std::string& nextId)
  : mSnapToPageKey(snapToPage)
  , mSnapCutoffKey(snapCutoff)
  , mSnapWeightKey(snapWeight)
  , mPreviousIdKey(previousId)
  , mNextIdKey(nextId)
{
}

/**
* \class ui::magazine::CfgReader
*/
CfgReader::CfgReader(const CfgKeys& k, const ds::xml::Settings& s)
{
  mSnapToPage = s.getBool(k.mSnapToPageKey, 0, mSnapToPage);
  mSnapCutoff = s.getFloat(k.mSnapCutoffKey, 0, mSnapCutoff);
  mSnapWeight = s.getFloat(k.mSnapWeightKey, 0, mSnapWeight);
  mPreviousId = s.getResourceId(k.mPreviousIdKey, 0, mPreviousId);
  mNextId = s.getResourceId(k.mNextIdKey, 0, mNextId);
}

} // namespace magazine
} // namespace ui
