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

const char GstVideoNet::mMuteAtt        = 'S';
const char GstVideoNet::mStatusAtt      = 'E';
const char GstVideoNet::mVolumeAtt      = 'P';
const char GstVideoNet::mLoopingAtt     = '3';
const char GstVideoNet::mAutoStartAtt   = 'H';
const char GstVideoNet::mPathAtt        = 'R';
const char GstVideoNet::mPosAtt         = 'L';

const DirtyState& GstVideoNet::mMuteDirty       = newUniqueDirtyState();
const DirtyState& GstVideoNet::mStatusDirty     = newUniqueDirtyState();
const DirtyState& GstVideoNet::mVolumeDirty     = newUniqueDirtyState();
const DirtyState& GstVideoNet::mLoopingDirty    = newUniqueDirtyState();
const DirtyState& GstVideoNet::mAutoStartDirty  = newUniqueDirtyState();
const DirtyState& GstVideoNet::mPathDirty       = newUniqueDirtyState();
const DirtyState& GstVideoNet::mPosDirty        = newUniqueDirtyState();

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

void GstVideoNet::writeAttributesTo(const DirtyState& dirty, DataBuffer& buf)
{
    if (dirty.has(mPathDirty))
    {
        buf.add(mPathAtt);
        buf.add(mVideoSprite.getLoadedFilename());
    }

    if (dirty.has(mAutoStartDirty))
    {
        buf.add(mAutoStartAtt);
        buf.add(mVideoSprite.getAutoStart());
    }

    if (dirty.has(mLoopingDirty))
    {
        buf.add(mLoopingAtt);
        buf.add(mVideoSprite.getIsLooping());
    }

    if (dirty.has(mStatusDirty))
    {
        buf.add(mStatusAtt);
        buf.add(mVideoSprite.getCurrentStatus().mCode);
    }

    if (dirty.has(mVolumeDirty))
    {
        buf.add(mVolumeAtt);
        buf.add(mVideoSprite.getVolume());
    }

    if (dirty.has(mMuteDirty))
    {
        buf.add(mMuteAtt);
        buf.add(mVideoSprite.getIsMuted());
    }

    if (dirty.has(mPosDirty))
    {
        buf.add(mPosAtt);
        buf.add(mVideoSprite.getCurrentTimeMs());
    }
}

bool GstVideoNet::readAttributeFrom(const char attrid, DataBuffer& buf)
{
    bool read_attrib = true;

    if (attrid == mPathAtt) {
        auto video_path = buf.read<std::string>();
        if (mVideoSprite.getLoadedFilename() != video_path)
            mVideoSprite.loadVideo(video_path);
    }
    else if (attrid == mAutoStartAtt) {
        auto auto_start = buf.read<bool>();
        if (mVideoSprite.getAutoStart() != auto_start)
            mVideoSprite.setAutoStart(auto_start);
    }
    else if (attrid == mLoopingAtt) {
        auto is_looping = buf.read<bool>();
        if (mVideoSprite.getIsLooping() != is_looping)
            mVideoSprite.setLooping(is_looping);
    }
    else if (attrid == mMuteAtt) {
        auto is_muted = buf.read<bool>();
        if (mVideoSprite.getIsMuted() != is_muted)
            mVideoSprite.setMute(is_muted);
    }
    else if (attrid == mVolumeAtt) {
        auto volume_level = buf.read<float>();
        if (mVideoSprite.getVolume() != volume_level)
            mVideoSprite.setVolume(volume_level);
    }
    else if (attrid == mPosAtt) {
        auto server_video_pos = buf.read<double>();
        mVideoSprite.syncWithServer(server_video_pos);
    }
    else if (attrid == mStatusAtt) {
        auto status_code = buf.read<int>();
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
    }
    else {
        read_attrib = false;
    }
    // To do: latency check.

    return read_attrib;
}

}
} //!ds:ui