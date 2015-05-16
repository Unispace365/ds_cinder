#include "gst_video_net.h"

#include <ds/ui/sprite/dirty_state.h>
#include <ds/app/blob_registry.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/app/engine/engine.h>

namespace ds {
namespace ui {

char GstVideoNet::mBlobType = 0;

GstVideoNet::GstVideoNet(GstVideo& video)
    : mParamsDirty(ds::ui::INTERNAL_A_DIRTY)
    , mParamsAtt(99)
    , mVideoSprite(video)
{}

void GstVideoNet::installAsServer(BlobRegistry& registry)
{
    mBlobType = registry.add([](ds::BlobReader& r)
    {
        Sprite::handleBlobFromClient(r);
    });
}

void GstVideoNet::installAsClient(BlobRegistry& registry)
{
    mBlobType = registry.add([](ds::BlobReader& r)
    {
        Sprite::handleBlobFromServer<GstVideo>(r);
    });
}

void GstVideoNet::installSprite(Engine& engine)
{
    engine.installSprite(installAsServer, installAsClient);
}

}
} //!ds:ui