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

    void			writeAttributesTo(DataBuffer&);
    void			readAttributeFrom(DataBuffer&);

private:
    GstVideo&                   mVideoSprite;

public:
    const ds::ui::DirtyState&   mParamsDirty;
    const char                  mParamsAtt;
};

}} //!ds::ui

#endif //!DS_VIDEO_GST_VIDEO_NET_H_
