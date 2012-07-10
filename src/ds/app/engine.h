#pragma once
#ifndef DS_APP_ENGINE_H_
#define DS_APP_ENGINE_H_

#include "ds/app/app_defs.h"
#include "ds/thread/work_manager.h"

namespace ds {
class WorkClient;

/**
 * \class ds::Engine
 * Container and manager for all views.
 */
class Engine {
  public:
    Engine();
    ~Engine();

    void					update();

  private:
#if defined DS_PLATFORM_SERVER || defined DS_PLATFORM_SERVERCLIENT
    friend class WorkClient;
    WorkManager				mWorkManager;
#endif
};

} // namespace ds

#endif // DS_APP_ENGINE_H_