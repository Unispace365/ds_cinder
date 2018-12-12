#pragma once

#ifndef DS_UI_SPRITE_GST_PLAYER_SPRITE_H_
#define DS_UI_SPRITE_GST_PLAYER_SPRITE_H_

#include <cinder/gl/Texture.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>

namespace gst {
namespace video {
class GstPlayer;
}
}

namespace ds {
namespace ui {

/**
* \class ds::ui::GstVideo
* \brief Video playback.
* Note: If you want to use multiple playback systems simultaneously, use this
* uniquely-named class. If you want to have one playback system that you can
* swap out easily, include "video.h" and just use the Video class.
*/
class GstPlayerSprite : public Sprite
{

public:

	GstPlayerSprite(SpriteEngine&);
	virtual ~GstPlayerSprite();

	// Loads a video from a file path.
	GstPlayerSprite&			loadVideo(const std::string &filename);
	GstPlayerSprite&			loadPipeline(const std::string &pipeline);

	/// Playback control API 
	virtual void		play();
	virtual void		stop();

protected:
	virtual void		drawLocalClient() override;
	void				updateVideoTexture();


private:

	ci::gl::TextureRef	mFrameTexture;
	ci::ivec2			mVideoSize;

	gst::video::GstPlayer*	mGstPlayer;
};

} //!namespace ui
} //!namespace ds

#endif //!DS_UI_SPRITE_GST_VIDEO_H_