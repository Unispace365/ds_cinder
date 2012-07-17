#pragma once
#ifndef DS_APP_ENGINECLIENT_H_
#define DS_APP_ENGINECLIENT_H_

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/app/engine.h"
#include "ds/data/data_buffer.h"
#include "ds/thread/gl_thread.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class ds::EngineClient
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineClient : public Engine {
  public:
    EngineClient(const ds::cfg::Settings&);

    virtual ds::WorkManager       &getWorkManager()         { return mWorkManager; }
    virtual ui::LoadImageService  &getLoadImageService()    { return mLoadImageService; }
    virtual ds::sprite_id_t        nextSpriteId();

    virtual void				          setup();
    virtual void                  setupTuio(ds::App&);
    virtual void	                update();
    virtual void                  draw();

  private:
    typedef Engine inherited;
    ds::BlobRegistry              mBlobRegistry;
    WorkManager                   mWorkManager;
    GlThread                      mLoadImageThread;
    ui::LoadImageService          mLoadImageService;

    ds::DataBuffer                mReceiveBuffer;
    ds::BlobReader                mBlobReader;

    void                          receiveHeader(ds::DataBuffer&);
    void                          receiveCommand(ds::DataBuffer&);
};

} // namespace ds

#endif // DS_APP_ENGINESERVER_H_