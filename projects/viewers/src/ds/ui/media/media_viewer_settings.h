#pragma once
#ifndef VIEWERS_UI_MEDIA_MEDIA_VIEWER_SETTINGS
#define VIEWERS_UI_MEDIA_MEDIA_VIEWER_SETTINGS

#include <string>
#include <cinder/Vector.h>
#include <cinder/Color.h>

namespace ds {
namespace ui { 

/// MediaViewerSettings
/// A set of things you can change about MediaViewers
/// Most of these need to be set before loading the media
/// This is a struct cause everything is public
struct MediaViewerSettings {

	MediaViewerSettings()
		: mDefaultBounds(0.0f, 0.0f)
		, mShowInterfaceAtStart(true)
		, mWebDefaultSize(0.0f, 0.0f)
		, mWebKeyboardKeyScale(1.0f)
		, mWebKeyboardAbove(true)
		, mWebAllowKeyboard(true)
		, mWebAllowTouchToggle(true)
		, mCacheImages(false)
		, mPdfCacheNextPrev(true)
		, mVideoPanning(0.0f)
		, mVideoAutoSync(true)
		, mVideoAutoPlayFirstFrame(true)
		, mVideoAllowOutOfBoundsMuting(true)
		, mVideoLoop(true)
		, mVideoStreamingLatency(0.2)
	{}

	//--------------------Overall Settings -----------------------------------------//
	/// The size to be calculated to fit inside when initially loading content
	ci::vec2					mDefaultBounds;

	/// Whether the interface should be visible immediately or not
	bool						mShowInterfaceAtStart;

	//--------------------Web Settings ---------------------------------------------//

	/// The size to load the web page, which will then get scaled up or down when the viewer changes size
	ci::vec2					mWebDefaultSize;		
	// the scale of the keyboard
	float						mWebKeyboardKeyScale;
	// If we should show the keyboard button at all. Default = true
	bool						mWebAllowKeyboard;
	// If we should show the button to allowing touching web sites. Default=true
	bool						mWebAllowTouchToggle;
	// If true, the keyboard appears above the interface. False will show below
	bool						mWebKeyboardAbove;

	//--------------------Image Settings -------------------------------------------//

	/// whether to cache the primary and thumb images. Default = false
	bool						mCacheImages;

	//--------------------PDF Settings ---------------------------------------------//

	/// whether to auto cache the next/previous pdf page. default = true
	bool						mPdfCacheNextPrev;

	//--------------------Video Settings -------------------------------------------//

	/// From -1.0f (left) to 1.0f (right). Default = 0.0f (center)
	float						mVideoPanning;

	/// A list of clients to not load the video on
	/// Default is empty, which loads everywhere
	/// See GstVideo header for more info
	std::vector<std::string>	mVideoPlayableInstances;

	/// Whether to auto-synchronize video across clients (no effect in standalone mode)
	/// Default = true
	bool						mVideoAutoSync;

	/// Set this to false in a some-instances-only environment, otherwise clients might never get audio
	/// Automatically displays the first frame of the video
	bool						mVideoAutoPlayFirstFrame;

	/// If the video goes out of the current bounds of the window, mute the video automatically (or don't have that functionality)
	bool						mVideoAllowOutOfBoundsMuting;

	/// Whether the video should loop or not by default
	bool						mVideoLoop;

	/// The latency when opening a stream in seconds
	double						mVideoStreamingLatency;



};

} // namespace ui
} // namespace ds

#endif
