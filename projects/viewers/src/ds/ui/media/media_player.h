#pragma once
#ifndef DS_UI_MEDIA_MEDIA_PLAYER
#define DS_UI_MEDIA_MEDIA_PLAYER

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/image.h>
#include "media_viewer_settings.h"
#include <ds/ui/media/player/web_player.h>

namespace ds {
namespace ui {
class PanoramicVideoPlayer;
class VideoPlayer;
class PDFPlayer;
class StreamPlayer;
class MediaInterface;

/**
* \class ds::MediaPlayer
*			This will load an appropriate media player after deducing the type: PDF, Web, Image, Video file or Video stream.
*			After loading a piece of media and initializing, the size of this sprite will match the size of the loaded media.
*			After initializing, you can set the size of this sprite to resize the media. 
*			Not recommended to scale this sprite, set it's size instead.
*
*			Note: for PDF thumbnail viewer to show up, the PDF needs to be loaded via a Resource
*					that has a children vector of resources of the thumbnails set, and the children need to have the correct parentIndex (i.e. page number) set.
*/
class MediaPlayer : public ds::ui::Sprite  {
public:
	MediaPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface = false);
	MediaPlayer(ds::ui::SpriteEngine& eng, const std::string& mediaPath, const bool embedInterface = false);
	MediaPlayer(ds::ui::SpriteEngine& eng, const ds::Resource& reccy, const bool embedInterface = false);

	void					setSettings(const MediaViewerSettings& newSettings);
	MediaViewerSettings&	getSettings(){ return mMediaViewerSettings; }

	/// Unloads any current media. If initialize immediately is true, it will deduce the media type and load the media
	void					loadMedia(const std::string& mediaPath, const bool initializeImmediately = true);
	void					loadMedia(const ds::Resource& reccy, const bool initializeImmediately = true);

	/// Returns the data model for the currently set media (may be blank or errored)
	const ds::Resource&		getResource(){ return mResource; }

	/// Sets the area for the initial default size calculation. must be called before initialize or load media to have an effect
	void					setDefaultBounds(const float defaultWidth, const float defaultHeight);

	/// Sets the area for the initial default web size calculation. must be called before initialize or load media to have an effect
	void					setWebViewSize(const ci::vec2 webSize);

	/// Actually loads the media set in constructor or loadMedia. if the media is already loaded, this does nothing.
	void					initialize();

	/// Unloads any media and interface already loaded. initialize could be called again after this and load the same content
	void					uninitialize();

	/// If true, the player has been initialized and it has media. False and it hasn't, and is empty, though may have a Resource set.
	const bool				getInitialized(){ return mInitialized; }

	/// When true, will cache any image players. Has no effect on other media types
	void					setCacheImages(bool cacheImages) { mMediaViewerSettings.mCacheImages = cacheImages; }

	/// The player starts being active, which will play videos
	void					enter();

	/// The player stops being active, pausing any videos
	void					exit();

	/// stops loading web pages, stops videos
	void					stopContent();

	/// Really only for videos at the moment
	void					playContent();
	void					pauseContent();
	void					toggleMute();

	/// Returns any current player. Will need to be dynamic casted to be used
	/// Definitely can return nullptr, so check before using
	ds::ui::Sprite*			getPlayer();

	/// Returns a media interface of a player.
	/// Only returns if there is currently a player and that player has an interface
	ds::ui::MediaInterface*	getMediaInterface();

	/// Display the interface on the media player, if it was embedded
	void					showInterface();

	/// Undisplay the interface on the media player, if it was embedded
	void					hideInterface();

	/// Called when any component failed to load it's media. or failed during running.
	/// Note that the message may be technical and not appropriate to show
	/// Errors also will be logged, o you may want to show a generic "Sorry, something went wrong"
	void					setErrorCallback(std::function<void(const std::string& msg)>);

	/// Lets you know when stuff is all good.
	/// Image: Image has been loaded
	/// Video: Video started playing
	/// PDF: Page has finished rasterizing
	/// Web: Page document has loaded
	void					setStatusCallback(std::function<void(const bool isGood)>);

	/// Called after a new piece of media has been initialized / loaded
	void					setInitializedCallback(std::function<void()> func);

	/// If the media loaded inside this player changes sizes (such as a pdf with different page sizes)
	void					setMediaSizeChangedCallback(std::function<void(const ci::vec2& newSize)> func){ mMediaSizeChangedCallback = func; }


	/// Will do standard functions based on media type:
	/// Web: Click the web content
	/// PDF: Advance to the next page
	/// Video: Toggle play / pause
	void					handleStandardClick(const ci::vec3& globalPos);

	/// Sets a tap function to enable the above handling
	void					enableStandardClick();

	/// Returns the width / height of the content that's loaded. If there's no content loaded, will return whatever the previous ratio was
	const float				getContentAspectRatio(){ return mContentAspectRatio; }

protected:

	/// override to do any custom layout functions
	virtual void			onLayout(){};
	virtual void			userInputReceived();
	virtual void			onSizeChanged();

	MediaViewerSettings		mMediaViewerSettings;

	bool					mEmbedInterface;
	bool					mInitialized;
	ds::Resource			mResource;
	float					mContentAspectRatio;
	float					mAnimDuration;

	PDFPlayer*				mPDFPlayer;
	StreamPlayer*			mStreamPlayer;
	VideoPlayer*			mVideoPlayer;
	PanoramicVideoPlayer*	mPanoramicPlayer;
	WebPlayer*				mWebPlayer;
	ds::ui::Image*			mThumbnailImage;
	ds::ui::Image*			mPrimaryImage;

	std::function<void(const std::string& msg)>	mErrorCallback;
	std::function<void(const bool isGood)>		mStatusCallback;
	std::function<void()>						mInitializedCallback;
	std::function<void(const ci::vec2&)>		mMediaSizeChangedCallback;

private:
	void					layout();
	void					setDefaultProperties();
};

} // namespace ui
} // namespace ds

#endif
