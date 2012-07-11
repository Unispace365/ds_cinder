#pragma once
#ifndef DS_APP_ENGINECLIENTSERVER_H_
#define DS_APP_ENGINECLIENTSERVER_H_

#include "ds/app/engine.h"
#include "ds/thread/work_manager.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 * Container and manager for all views.
 */
class EngineClientServer : public Engine {
  public:
    EngineClientServer(const ds::cfg::Settings&);

    virtual ds::WorkManager     &getWorkManager()   { return mWorkManager; }

    virtual void	              update();

  private:
    typedef Engine inherited;
    WorkManager				          mWorkManager;
};

} // namespace ds

#endif // DS_APP_ENGINE_H_