#pragma once
#ifndef DS_UI_MEDIA_MEDIA_VIEWER
#define DS_UI_MEDIA_MEDIA_VIEWER


#include "ds/ui/panel/base_panel.h"
#include <ds/ui/sprite/image.h>

namespace ds {
namespace ui {
class VideoPlayer;
class PDFPlayer;
class StreamPlayer;
class WebPlayer;
class MediaInterface;

/**
* \class ds::MediaViewer
*			A container that can show the usual suspects with interfaces: Web, PDF, Image, Video
*			The goal here is to have an independent class that can be moved to the framework.
*/
class MediaViewer : public BasePanel  {
public:
	MediaViewer(ds::ui::SpriteEngine& eng, const bool embedInterface = false);
	MediaViewer(ds::ui::SpriteEngine& eng, const std::string& mediaPath, const bool embedInterface = false);
	MediaViewer(ds::ui::SpriteEngine& eng, const ds::Resource& reccy, const bool embedInterface = false);

	// unloads any current media
	void				loadMedia(const std::string& mediaPath, const bool initializeImmediately = true);
	void				loadMedia(const ds::Resource& reccy, const bool initializeImmediately = true);

	// Sets the area for the initial default size calculation. must be called before initialize or load media to have an effect
	void				setDefaultBounds(const float defaultWidth, const float defaultHeight);
	
	/// Actually loads the media set in constructor or loadMedia. if the media is already loaded, this does nothing.
	void				initialize();

	/// unloads any media and interface already loaded. initialize could be called again after this and load the same content
	void				uninitialize();

	void				setCacheImages(bool cacheImages) { mCacheImages = cacheImages; }

	virtual void		onLayout();
	void				enter();
	void				exit();

	// stops loading web pages, stops videos
	void				stopContent();

	void				setOrigin(const ci::Vec3f& origin){ mOrigin = origin; }
	const ci::Vec3f&	getOrigin(){ return mOrigin; }

	// Returns any current player. Will need to be dynamic casted to be used
	// Definitely can return nullptr, so check before using
	ds::ui::Sprite*		getPlayer();

	void				showInterface();

	/// Called when any component failed to load it's media. or failed during running.
	/// Note that the message may be technical and not appropriate to show
	/// Errors also will be logged, o you may want to show a generic "Sorry, something went wrong"
	void				setErrorCallback(std::function<void(const std::string& msg)>);

	/// Lets you know when stuff is all good.
	/// Image: Image has been loaded
	/// Video: Video started playing
	/// TODO: PDF and Web finished loading
	void				setStatusCallback(std::function<void(const bool isGood)>);

protected:

	virtual void		userInputReceived();

	bool				mEmbedInterface;
	bool				mInitialized;
	ds::Resource		mResource;

	PDFPlayer*			mPDFPlayer;
	StreamPlayer*		mStreamPlayer;
	VideoPlayer*		mVideoPlayer;
	WebPlayer*			mWebPlayer;
	ds::ui::Image*		mThumbnailImage;
	ds::ui::Image*		mPrimaryImage;

	bool				mCacheImages;


	ci::Vec3f			mOrigin;

	float				mDefaultBoundWidth;
	float				mDefaultBoundHeight;

	std::function<void(const std::string& msg)>	mErrorCallback;
	std::function<void(const bool isGood)> mStatusCallback;
};

} // namespace ui
} // namespace ds

#endif
