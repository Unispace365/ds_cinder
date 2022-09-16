#pragma once


#include <ds/ui/sprite/sprite.h>
#include <gstreamer/gstreamer_audio_device.h>
#include <ds/ui/media/player/video_player.h>

namespace ds {



namespace ui {



/**
 * \class VideoPlayer
 *			Creates a video and puts an interface on top of it.
 */
class SplitAlphaVideoPlayer : public ds::ui::VideoPlayer {
  public:
	SplitAlphaVideoPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface = true);
	virtual void onUpdateServer(const ds::UpdateParams& up) override;
	virtual void drawLocalClient() override;
	virtual void setResource(const ds::Resource& resource) override;
	virtual void onSizeChanged() override;
	ci::gl::Texture2dRef mTex;
	ci::gl::GlslProgRef mSplitAlphaShader;
	
private:

};

}  // namespace ui
}  // namespace ds

