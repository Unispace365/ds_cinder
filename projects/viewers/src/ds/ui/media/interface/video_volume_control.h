#pragma once

#include "ds/ui/media/media_interface.h"

namespace ds::ui {
class GstVideo;
class YouTubeWeb;
class ImageButton;

enum class VideoVolumeStyle { CLASSIC, SLIDER };
struct VideoVolumeSliderSprites {
	ds::ui::ImageButton* mMuteButton  = nullptr;
	ds::ui::Sprite*		 mSliderTrack = nullptr;
	ds::ui::Sprite*		 mSliderFill  = nullptr;
	ds::ui::Sprite*		 mSliderNub	  = nullptr;
};
/**
 * \class VideoVolumeControl
 *			Control audio volume for a video
 */
class VideoVolumeControl : public ds::ui::Sprite {
  public:
	VideoVolumeControl(ds::ui::SpriteEngine& eng, const float theSize = 50.0f, const float buttHeight = 25.0f,
					   const ci::Color	interfaceColor = ci::Color::white(),
					   VideoVolumeStyle style		   = VideoVolumeStyle::CLASSIC);

	void linkVideo(ds::ui::GstVideo* linkedVideo);
	void linkYouTube(ds::ui::YouTubeWeb* linkedYouTube);
	void setVolume(float volume) override;

	void			 setStyle(VideoVolumeStyle newStyle);
	VideoVolumeStyle getStyle() { return mStyle; }
	// do not release the bars here, this is to modify their values
	std::vector<ds::ui::Sprite*>& getBars() { return mBars; }
	VideoVolumeSliderSprites&	  getSliderSprites() { return mSliderSprites; }

	void setSliderHeight(float height) { mSliderHeight = height; }
	void setNubSize(float height) { mNubSize = height; }
	void setMuteImage(const std::string& imgLocation) { mMuteImage = imgLocation; }
	void setVolumeLowImage(const std::string& imgLocation) { mVolumeLowImage = imgLocation; }
	void setVolumeHighImage(const std::string& imgLocation) { mVolumeHighImage = imgLocation; }

  protected:
	virtual void onUpdateServer(const ds::UpdateParams& updateParams) override;

	VideoVolumeStyle mStyle = VideoVolumeStyle::CLASSIC;

	ds::ui::GstVideo*	mLinkedVideo;
	ds::ui::YouTubeWeb* mLinkedYouTube;

	ci::Color mInterfaceColor;
	float	  mTheSize	  = 50.f;
	float	  mButtHeight = 25.f;

	// For classic style
	std::vector<ds::ui::Sprite*> mBars;

	// For Slider style
	VideoVolumeSliderSprites mSliderSprites;
	float					 mSliderHeight	  = 8.f;
	float					 mNubSize		  = 12.f;
	std::string				 mMuteImage		  = "%APP%/data/images/media_interface/mute.png";
	std::string				 mVolumeLowImage  = "%APP%/data/images/media_interface/volume_low.png";
	std::string				 mVolumeHighImage = "%APP%/data/images/media_interface/volume_high.png";


	float mOffOpacity;
	float mLastVolume = -1.f;
	bool  mMuted	  = false;
};

} // namespace ds::ui
