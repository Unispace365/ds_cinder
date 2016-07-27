#pragma once
#ifndef DS_UI_MEDIA_VIDEO_PLAYER_VIDEO_PLAYER
#define DS_UI_MEDIA_VIDEO_PLAYER_VIDEO_PLAYER


#include <ds/ui/sprite/sprite.h>

namespace ds {

class Resource;

namespace ui {

class GstVideo;
class VideoInterface;
class MediaInterface;

/**
* \class ds::ui::VideoPlayer
*			Creates a video and puts an interface on top of it.
*/
class VideoPlayer : public ds::ui::Sprite  {
public:
	VideoPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface = true);

	void								setMedia(const std::string mediaPath);
	void								clear();

	void								layout();

	void								play();
	void								pause();
	void								stop();
	void								togglePlayPause();

	void								showInterface();
	void								setShowInterfaceAtStart(bool showInterfaceAtStart);

	ds::ui::GstVideo*					getVideo();

	void								setGoodStatusCallback(std::function<void()> func){ mGoodStatusCallback = func; }
	void								setErrorCallback(std::function<void(const std::string&)> func){ mErrorMsgCallback = func; }

	/// See the function of the same name on GstVideo, Set any time, will remember between loading videos
	void								setPan(const float newPan);

	/// See the function of the same name on GstVideo, Whether to synchronize across client/servers, default = true
	void								setAutoSynchronize(const bool doSync);

	/// See the function of the same name on GstVideo
	void								setPlayableInstances(const std::vector<std::string> instanceNames);

	/// Automatically plays the first frame of the video so you can see what the video is.
	/// Must be set before playing the video
	/// Default = true
	void								setAutoPlayFirstFrame(const bool playFirstFrame);

	/// If the video goes out-of-bounds, whether to auto-mute or not
	void								allowOutOfBoundsMuted(const bool allowMuting);

	/// Sets looping on the video
	void								setVideoLoop(const bool doLooping);

protected:

	virtual void								onSizeChanged();
	VideoInterface*								mVideoInterface;
	ds::ui::GstVideo*							mVideo;
	bool										mEmbedInterface;
	bool										mShowInterfaceAtStart;
	std::function<void(void)>					mGoodStatusCallback;
	std::function<void(const std::string&)>		mErrorMsgCallback;

	/// Settings - these are kept locally here so the settings can be applied at any time
	float										mPanning;
	bool										mAutoSyncronize;
	bool										mAutoPlayFirstFrame;
	bool										mAllowOutOfBoundsMuted;
	std::vector<std::string>					mPlayableInstances;
	bool										mLooping;
};

} // namespace ui
} // namespace ds

#endif
