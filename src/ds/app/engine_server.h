#pragma once
#ifndef DS_APP_ENGINESERVER_H_
#define DS_APP_ENGINESERVER_H_

#include "ds/app/engine.h"
#include "ds/thread/gl_thread.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class ds::EngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineServer : public Engine {
  public:
    EngineServer(const ds::cfg::Settings&);

    virtual ds::WorkManager       &getWorkManager()         { return mWorkManager; }
    virtual ui::LoadImageService  &getLoadImageService()    { return mLoadImageService; }

    virtual void                  setupTuio(ds::App&);
    virtual void	                update();
    virtual void                  draw();

  private:
    typedef Engine inherited;
    WorkManager				            mWorkManager;
    GlNoThread                    mLoadImageThread;
    ui::LoadImageService          mLoadImageService;
};

} // namespace ds

#endif // DS_APP_ENGINESERVER_H_