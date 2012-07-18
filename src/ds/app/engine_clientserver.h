#pragma once
#ifndef DS_APP_ENGINECLIENTSERVER_H_
#define DS_APP_ENGINECLIENTSERVER_H_

#include "ds/app/engine.h"
#include "ds/thread/gl_thread.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 * The ClientServer engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
 */
class EngineClientServer : public Engine {
  public:
    EngineClientServer(const ds::cfg::Settings&);

    virtual ds::WorkManager       &getWorkManager()         { return mWorkManager; }
    virtual ui::LoadImageService  &getLoadImageService()    { return mLoadImageService; }

    virtual void                  installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
                                                const std::function<void(ds::BlobRegistry&)>& asClient);

    virtual void				          setup(ds::App&);
    virtual void                  setupTuio(ds::App&);
    virtual void	                update();
    virtual void                  draw();

  private:
    typedef Engine inherited;
    WorkManager				            mWorkManager;
    GlThread                      mLoadImageThread;
    ui::LoadImageService          mLoadImageService;
};

} // namespace ds

#endif // DS_APP_ENGINECLIENTSERVER_H_