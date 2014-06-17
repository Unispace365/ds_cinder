#pragma once
#ifndef NA_UI_MAGAZINE_MAGAZINECFG_H_
#define NA_UI_MAGAZINE_MAGAZINECFG_H_

#include <ofxWorldAndRenderEngineShared.h>
#include "na/ui/magazine/magazine_content_manager.h"

namespace ds {
namespace xml {
class Settings;
}
}

namespace ui {
namespace magazine {

/**
 * \class ui::magazine::Cfg
 * Standard magazine config.
 */
class Cfg {
  public:
    Cfg();
    virtual ~Cfg();

    bool                      mSnapToPage;
    // The velocity cutoff below which a snap takes place. Think of
    // it as roughly pixels per second. 2.0 is a good value.
    float                     mSnapCutoff;
    // A value between 0 - 1 that determines how far into the
    // page a throw occurs before it will automatically complete
    // in the thrown direction. For example, if 0.15, then anything
    // that lands 15% or more towards the next page will go to the
    // next, otherwise it will snap back to the current.
    float                     mSnapWeight;
    // The previous / next page buttons
    ds::resource_id           mPreviousId, mNextId;
};

/**
 * \class ui::magazine::CfgKeys
 */
class CfgKeys {
  public:
    CfgKeys(  const std::string& snapToPage, const std::string& snapCutoff, const std::string& snapWeight,
              const std::string& previousId, const std::string& nextId);

    const std::string         mSnapToPageKey;
    const std::string         mSnapCutoffKey;
    const std::string         mSnapWeightKey;
    const std::string         mPreviousIdKey;
    const std::string         mNextIdKey;
};

/**
 * \class ui::magazine::CfgReader
 * Populate a cfg using the supplied keys.
 */
class CfgReader : public Cfg {
  public:
    CfgReader(const CfgKeys&, const ds::xml::Settings&);
};

} // namespace magazine
} // namespace ui

#endif // NA_UI_MAGAZINE_MAGAZINECFG_H_