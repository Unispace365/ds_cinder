#pragma once
#ifndef DS_APP_ENGINESERVER_H_
#define DS_APP_ENGINESERVER_H_

#include "ds/app/engine.h"
#include "ds/thread/work_manager.h"

namespace ds {

/**
 * \class ds::EngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineServer : public Engine {
  public:
    EngineServer(const ds::cfg::Settings&);

    virtual ds::WorkManager     &getWorkManager()   { return mWorkManager; }

    virtual void	              update();

  private:
    typedef Engine inherited;
    WorkManager				          mWorkManager;
};

} // namespace ds

#endif // DS_APP_ENGINESERVER_H_