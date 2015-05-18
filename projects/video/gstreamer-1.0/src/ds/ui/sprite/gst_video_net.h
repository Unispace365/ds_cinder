#ifndef DS_VIDEO_GST_VIDEO_NET_H_
#define DS_VIDEO_GST_VIDEO_NET_H_

#include <ds/ui/sprite/dirty_state.h>

namespace ds {
class BlobRegistry;
class DataBuffer;
class Engine;
namespace ui {
class GstVideo;

class GstVideoNet
{
public:
    GstVideoNet(GstVideo&);
    // Sprite blob type registered with engine.
    static char                 mBlobType;

    static void		installAsServer(BlobRegistry&);
    static void		installAsClient(BlobRegistry&);
    static void		installSprite(Engine&);

    void			writeAttributesTo(const DirtyState&, DataBuffer&);
    bool			readAttributeFrom(const char attrib, DataBuffer&);

private:
    GstVideo&                   mVideoSprite;

public:
    static const DirtyState&   mPathDirty;
    static const DirtyState&   mAutoStartDirty;
    static const DirtyState&   mLoopingDirty;
    static const DirtyState&   mVolumeDirty;
    static const DirtyState&   mStatusDirty;
    static const DirtyState&   mMuteDirty;
    static const DirtyState&   mPosDirty;

    static const char          mPathAtt;
    static const char          mAutoStartAtt;
    static const char          mLoopingAtt;
    static const char          mVolumeAtt;
    static const char          mStatusAtt;
    static const char          mMuteAtt;
    static const char          mPosAtt;
};

}} //!ds::ui

#endif //!DS_VIDEO_GST_VIDEO_NET_H_
