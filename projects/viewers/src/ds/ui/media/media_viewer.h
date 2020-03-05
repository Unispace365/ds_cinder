#pragma once

#include "ds/ui/panel/base_panel.h"
#include <ds/ui/sprite/image.h>
#include "media_viewer_settings.h"
#include <ds/ui/media/player/web_player.h>

namespace ds {
namespace ui {
class MediaPlayer;
class MediaInterface;

/**
* \class MediaViewer
*			This is MediaPlayer in a BasePanel: plays the typical media types (Web / PDF / Video / Stream / Image) in a moveable/sizeable/bounds-checked panel.
*/
class MediaViewer : public BasePanel  {
public:
	MediaViewer(ds::ui::SpriteEngine& eng, const bool embedInterface = false);
	MediaViewer(ds::ui::SpriteEngine& eng, const std::string& mediaPath, const bool embedInterface = false);
	MediaViewer(ds::ui::SpriteEngine& eng, const ds::Resource& reccy, const bool embedInterface = false);

	void					setSettings(const MediaViewerSettings& newSettings);
	MediaViewerSettings&	getSettings(){ return mMediaViewerSettings; }

	// unloads any current media
	void					loadMedia(const std::string& mediaPath, const bool initializeImmediately = true);
	void					loadMedia(const ds::Resource& reccy, const bool initializeImmediately = true);

	/// Support the base sprite setting function, which is the same as loadMedia and initialize immediately
	virtual void			setResource(const ds::Resource& reccy) { loadMedia(reccy); }

	/// Returns the data model for the currently set media (may be blank or errored)
	ds::Resource			getResource();

	// Sets the area for the initial default size calculation. must be called before initialize or load media to have an effect
	void					setDefaultBounds(const float defaultWidth, const float defaultHeight);
	void					setWebViewSize(const ci::vec2 webSize);
	
	/// Actually loads the media set in constructor or loadMedia. if the media is already loaded, this does nothing.
	void					initialize();

	/// unloads any media and interface already loaded. initialize could be called again after this and load the same content
	void					uninitialize();

	void					setCacheImages(bool cacheImages);

	virtual void			onLayout();
	void					enter();
	void					exit();

	// stops loading web pages, stops videos
	void					stopContent();

	/// Origin is a convenience position, primarily used in media slideshow. 
	/// It's a position where the panel starts from, uncoupled from the viewer's current position
	void					setOrigin(const ci::vec3& origin){ mOrigin = origin; }
	const ci::vec3&		getOrigin(){ return mOrigin; }

	/// Returns any current player. Will need to be dynamic casted to be used
	/// Definitely can return nullptr, so check before using
	ds::ui::Sprite*			getPlayer();

	void					showInterface();
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


	/// Will do standard functions based on media type:
	/// Web: Click the web content
	/// PDF: Advance to the next page
	/// Video: Toggle play / pause
	void					handleStandardClick(const ci::vec3& globalPos);

	/// Sets a tap function to enable the above handling
	void					enableStandardClick();

protected:

	virtual void			userInputReceived();

	MediaViewerSettings		mMediaViewerSettings;
	MediaPlayer*			mMediaPlayer;

	ci::vec3				mOrigin;

private:
	void					setDefaultProperties();
};

} // namespace ui
} // namespace ds
