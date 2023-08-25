#pragma once

#include "ds/ui/media/media_interface.h"

namespace ds { namespace ui {
	class GstVideo;
	class YouTubeWeb;

	/**
	 * \class VideoVolumeControl
	 *			Control audio volume for a video
	 */
	class VideoVolumeControl : public ds::ui::Sprite {
	  public:
		VideoVolumeControl(ds::ui::SpriteEngine& eng, const float theSize = 50.0f, const float buttHeight = 25.0f,
						   const ci::Color interfaceColor = ci::Color::white());

		void linkVideo(ds::ui::GstVideo* linkedVideo);
		void linkYouTube(ds::ui::YouTubeWeb* linkedYouTube);
		void setVolume(const float v);

		// do not release the bars here, this is to modify their values
		std::vector<ds::ui::Sprite*>& getBars() { return mBars; }

	  protected:
		virtual void onUpdateServer(const ds::UpdateParams& updateParams) override;

		ds::ui::GstVideo*	mLinkedVideo;
		ds::ui::YouTubeWeb* mLinkedYouTube;

		std::vector<ds::ui::Sprite*> mBars;
		float						 mOffOpacity;
	};

}} // namespace ds::ui
