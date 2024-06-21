#include "stdafx.h"

#include "video_volume_control.h"


#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/media/player/youtube_player.h>
#include <ds/ui/sprite/video.h>

namespace ds::ui {

VideoVolumeControl::VideoVolumeControl(ds::ui::SpriteEngine& eng, const float theSize, const float buttHeight,
									   const ci::Color interfaceColor, VideoVolumeStyle style)
  : ds::ui::Sprite(eng, theSize * 1.5f, theSize)
  , mStyle(style)
  , mLinkedVideo(nullptr)
  , mLinkedYouTube(nullptr)
  , mInterfaceColor(interfaceColor)
  , mTheSize(theSize)
  , mButtHeight(buttHeight)
  , mOffOpacity(0.2f) {

	setStyle(mStyle);
}

void VideoVolumeControl::setStyle(VideoVolumeStyle newStyle) {
	mStyle = newStyle;

	clearChildren();
	mBars.clear();
	if (mStyle == VideoVolumeStyle::CLASSIC) {
		setSize(mTheSize * 1.5f, mTheSize);
		const int barCount = 5;

		const float floatNumBars = (float)(barCount);
		const float gapPading	 = getWidth() / (floatNumBars * 3.0f);
		float		barWiddy	 = (getWidth() - (floatNumBars - 1.0f) * gapPading) / floatNumBars;
		float		xp			 = 0.0f;

		for (int k = 0; k < barCount; ++k) {
			ds::ui::Sprite* s = new ds::ui::Sprite(mEngine, barWiddy, mButtHeight * (float)(k + 1) / floatNumBars);
			if (!s) continue;
			s->setTransparent(false);
			s->setCenter(0.0f, 1.0f);
			s->setColor(mInterfaceColor);
			s->setPosition(xp, getHeight() / 2.0f + mButtHeight / 2.0f);
			mBars.push_back(s);
			addChild(*s);

			xp += barWiddy + gapPading;
		}

	} else if (mStyle == VideoVolumeStyle::SLIDER) {
		setSize(mTheSize * 2.f, mTheSize);
		const auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;
		// Slider is made up of 3 parts:
		// 'mute' - Button to toggle between muted / unmuted
		// 'track' - the background of the slider showing it's overall length
		// 'fill' - the filled portion of the slider
		// 'nub' - the visual handle at the current slider position
		mSliderSprites.mMuteButton =
			new ds::ui::ImageButton(mEngine, "", "", (mTheSize - mButtHeight) / 2.0f);
		mSliderSprites.mMuteButton->setNormalImage(mVolumeHighImage, imageFlags);
		mSliderSprites.mMuteButton->setHighImage(mMuteImage, imageFlags);
		mSliderSprites.mMuteButton->setScale((mTheSize - (mButtHeight * 0.5f)) /
											 mSliderSprites.mMuteButton->getHeight());
		mSliderSprites.mMuteButton->setCenter(0.5f, 0.5f);
		mSliderSprites.mMuteButton->setPosition(0.0f, getHeight() / 2.f);
		mSliderSprites.mMuteButton->setClickFn([this] {
			if (mMuted) {
				setVolume(mLastVolume);
			} else {
				setVolume(0.f);
			}
		});
		addChildPtr(mSliderSprites.mMuteButton);

		auto muteOffset = mSliderSprites.mMuteButton->getScaleWidth() / 2.f; // Mute button is centered, only need half
		const auto padding	= mSliderHeight;
		ci::vec2   trackPos = ci::vec2(muteOffset + padding, getHeight() / 2.f);

		mSliderSprites.mSliderTrack = new ds::ui::Sprite(mEngine, getWidth() - (trackPos.x), mSliderHeight);
		mSliderSprites.mSliderTrack->setTransparent(false);
		mSliderSprites.mSliderTrack->setColor(mInterfaceColor);
		mSliderSprites.mSliderTrack->setCenter(0.f, 0.5f);
		mSliderSprites.mSliderTrack->setOpacity(mOffOpacity);
		mSliderSprites.mSliderTrack->setPosition(trackPos);
		addChildPtr(mSliderSprites.mSliderTrack);

		mSliderSprites.mSliderFill = new ds::ui::Sprite(mEngine, getWidth() - (trackPos.x), mSliderHeight);
		mSliderSprites.mSliderFill->setTransparent(false);
		mSliderSprites.mSliderFill->setColor(mInterfaceColor);
		mSliderSprites.mSliderFill->setCenter(0.f, 0.5f);
		mSliderSprites.mSliderFill->setPosition(trackPos);
		addChildPtr(mSliderSprites.mSliderFill);

		mSliderSprites.mSliderNub = new ds::ui::Sprite(mEngine, mNubSize, mNubSize);
		mSliderSprites.mSliderNub->setTransparent(false);
		mSliderSprites.mSliderNub->setColor(mInterfaceColor);
		mSliderSprites.mSliderNub->setCenter(0.5f, 0.5f);
		mSliderSprites.mSliderNub->setPosition(trackPos);
		addChildPtr(mSliderSprites.mSliderNub);
	}

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
		if (ti.mFingerIndex != 0 || ti.mPhase == ds::ui::TouchInfo::Removed) return;

		float local_x = 0.f;
		float local_w = getWidth();
		if (mStyle == VideoVolumeStyle::CLASSIC) {
			local_x = globalToLocal(ti.mCurrentGlobalPoint).x;
		} else if (mStyle == VideoVolumeStyle::SLIDER) {
			local_x = mSliderSprites.mSliderTrack->globalToLocal(ti.mCurrentGlobalPoint).x;
			local_w = mSliderSprites.mSliderTrack->getWidth();
		}
		if (local_x <= 0.0f) {
			setVolume(0.0f);
		} else if (local_x > local_w) {
			setVolume(1.0f);
		} else {
			setVolume(local_x / local_w);
		}
	});
}

void VideoVolumeControl::linkVideo(ds::ui::GstVideo* vid) {
	mLinkedVideo = vid;
}


void VideoVolumeControl::linkYouTube(ds::ui::YouTubeWeb* linkedYouTube) {
	mLinkedYouTube = linkedYouTube;
}

void VideoVolumeControl::setVolume(float volume) {
	Sprite::setVolume(volume);
	if (mLinkedVideo) {
		if (mLinkedVideo->getIsMuted() && volume > 0.0f) {
			mLinkedVideo->setMute(false);
		}
		mLinkedVideo->setVolume(volume);
	}

	if (mLinkedYouTube) {
		mLinkedYouTube->setVolume(volume);
	}
}

void VideoVolumeControl::onUpdateServer(const ds::UpdateParams& updateParams) {
	float vol = 0.0f;
	if (mLinkedVideo) {
		vol = mLinkedVideo->getVolume();
		if (mLinkedVideo->getIsMuted()) {
			vol = 0.0f;
		}
	}

	if (mLinkedYouTube) {
		vol = mLinkedYouTube->getVolume();
	}

	bool isMuted = (vol <= std::numeric_limits<float>::epsilon());

	if (isMuted != mMuted || abs(mLastVolume - vol) > std::numeric_limits<float>::epsilon()) {
		if (mStyle == VideoVolumeStyle::CLASSIC) {
			for (int k = 0; k < mBars.size(); ++k) {
				const float bar_v = static_cast<float>(k + 1) / static_cast<float>(mBars.size());
				if (vol >= bar_v) {
					mBars[k]->setOpacity(1.0f);
				} else {
					mBars[k]->setOpacity(mOffOpacity);
				}
			}
		} else if (mStyle == VideoVolumeStyle::SLIDER) {
			const auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

			if (isMuted || vol <= std::numeric_limits<float>::epsilon()) {
				mSliderSprites.mMuteButton->setNormalImage(mMuteImage, imageFlags);

				const auto& highImage = (mLastVolume < 0.5f) ? mVolumeLowImage : mVolumeHighImage;
				mSliderSprites.mMuteButton->setHighImage(highImage, imageFlags);
			} else if (vol < 0.5f) {
				mSliderSprites.mMuteButton->setNormalImage(mVolumeLowImage, imageFlags);
				mSliderSprites.mMuteButton->setHighImage(mMuteImage, imageFlags);
			} else {
				mSliderSprites.mMuteButton->setNormalImage(mVolumeHighImage, imageFlags);
				mSliderSprites.mMuteButton->setHighImage(mMuteImage, imageFlags);
			}

			mSliderSprites.mSliderFill->setSize(vol * mSliderSprites.mSliderTrack->getWidth(),
												mSliderSprites.mSliderFill->getHeight());

			mSliderSprites.mSliderNub->setPosition(mSliderSprites.mSliderTrack->getPosition().x +
													   (vol * mSliderSprites.mSliderTrack->getWidth()),
												   mSliderSprites.mSliderNub->getPosition().y);
		}

		if (!isMuted) {
			// Save the last volume when muting to restore when unmuting
			mLastVolume = vol;
		}
		mMuted = isMuted;
	}
}

} // namespace ds::ui
