#ifndef DS_VIDEO_GST_VIDEO_NET_H_
#define DS_VIDEO_GST_VIDEO_NET_H_

namespace ds {
class BlobRegistry;
class Engine;
namespace ui {
class GstVideo;
class DirtyState;

class GstVideoNet
{
public:
    GstVideoNet(GstVideo&);
    // Sprite blob type registered with engine.
    static char                 mBlobType;

    static void		installAsServer(BlobRegistry&);
    static void		installAsClient(BlobRegistry&);
    static void		installSprite(Engine&);

private:
    GstVideo&                   mVideoSprite;
    const ds::ui::DirtyState&   mParamsDirty;
    const char                  mParamsAtt;
};

}} //!ds::ui

#endif //!DS_VIDEO_GST_VIDEO_NET_H_
