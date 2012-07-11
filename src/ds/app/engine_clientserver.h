#pragma once
#ifndef DS_APP_ENGINECLIENTSERVER_H_
#define DS_APP_ENGINECLIENTSERVER_H_

#include "ds/app/engine.h"
#include "ds/thread/work_manager.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 * The ClientServer engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
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

#endif // DS_APP_ENGINECLIENTSERVER_H_