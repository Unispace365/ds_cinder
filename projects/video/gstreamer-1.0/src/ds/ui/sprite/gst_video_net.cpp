#include "gst_video_net.h"
#include "gst_video.h" //ugh.

#include <ds/data/data_buffer.h>
#include <ds/app/blob_registry.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_reader.h>
#include <ds/app/app.h>

namespace {
static struct Initializer {
    Initializer() {
        ds::App::AddStartup([](ds::Engine& engine) {
            ds::ui::GstVideoNet::installSprite(engine);
        });
    }
} INIT;
}

namespace ds {
namespace ui {

char GstVideoNet::mBlobType = 0;
const char GstVideoNet::mParamsAtt = 99;
const DirtyState& GstVideoNet::mParamsDirty = newUniqueDirtyState();

GstVideoNet::GstVideoNet(GstVideo& video)
    : mVideoSprite(video)
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

void GstVideoNet::writeAttributesTo(DataBuffer& buf)
{
    buf.add(mParamsAtt);
    buf.add(mVideoSprite.getLoadedVideoPath());
    buf.add(mVideoSprite.getAutoStart());
    buf.add(mVideoSprite.getIsLooping());
    buf.add(mVideoSprite.getIsMuted());
    buf.add(mVideoSprite.getCurrentStatus().mCode);
    buf.add(mVideoSprite.getVolume());
    buf.add(mVideoSprite.getCurrentTime());
}

void GstVideoNet::readAttributeFrom(DataBuffer& buf)
{
    auto video_path     = buf.read<std::string>();
    auto auto_start     = buf.read<bool>();
    auto is_looping     = buf.read<bool>();
    auto is_muted       = buf.read<bool>();
    auto status_code    = buf.read<int>();
    auto volume_level   = buf.read<float>();

    if (mVideoSprite.getLoadedVideoPath() != video_path) mVideoSprite.loadVideo(video_path);
    if (mVideoSprite.getAutoStart() != auto_start) mVideoSprite.setAutoStart(auto_start);
    if (mVideoSprite.getIsLooping() != is_looping) mVideoSprite.setLooping(is_looping);
    if (mVideoSprite.getIsMuted() != is_muted) mVideoSprite.setMute(is_muted);
    if (mVideoSprite.getVolume() != volume_level) mVideoSprite.setVolume(volume_level);

    if (mVideoSprite.getCurrentStatus() != status_code)
    {
        if (status_code == GstVideo::Status::STATUS_PAUSED)
        {
            mVideoSprite.pause();
        }
        else if (status_code == GstVideo::Status::STATUS_STOPPED)
        {
            mVideoSprite.stop();
        }
        else if (status_code == GstVideo::Status::STATUS_PLAYING)
        {
            mVideoSprite.play();
        }
    }
    // To do: latency check.
}

}
} //!ds:ui