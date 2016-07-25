#pragma once
#ifndef DS_UI_SPRITE_WEB_H_
#define DS_UI_SPRITE_WEB_H_

#include <cinder/app/KeyEvent.h>
#include <cinder/app/MouseEvent.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/text.h"


namespace ds {
namespace web {
	class WebCefService;
}

namespace ui {
/**
 * \class ds::ui::Web
 * \brief Display a web page using Chromium Embedded Framework: https://bitbucket.org/chromiumembedded/cef
 */
class Web : public ds::ui::Sprite {
public:
	Web(ds::ui::SpriteEngine &engine, float width = 0.0f, float height = 0.0f);
	~Web();


	// Loads the new url in the main frame (what you'd expect to happen)
	void										loadUrl(const std::wstring &url);
	void										loadUrl(const std::string &url);

	// setURL is identical to loadUrl. No exceptions will be thrown from any set or load url function.
	// load/set url are all kept around for compatibility with the old APIs
	void										setUrl(const std::string&);
	void										setUrlOrThrow(const std::string&);

	std::string									getUrl();

	// -------- Input Controls -------------------------------- //
	// If the sprite is being touched by mDragScrollMinFingers or more, will send mouse scroll events to the web view.
	void										setDragScrolling(const bool doScrolling){ mDragScrolling = doScrolling; }
	void										setDragScrollingMinimumFingers(const int numFingers){ mDragScrollMinFingers = numFingers; }

	void										sendKeyDownEvent(const ci::app::KeyEvent &event);
	void										sendKeyUpEvent(const ci::app::KeyEvent &event);

	// This web sprite handles touch-to-mouse events by default.
	// Though you can use these to roll your own touch stuff
	void										sendMouseDownEvent(const ci::app::MouseEvent &event);
	void										sendMouseDragEvent(const ci::app::MouseEvent &event);
	void										sendMouseUpEvent(const ci::app::MouseEvent &event);

	void										sendMouseClick(const ci::Vec3f& globalClickPoint);

	// DEPRECATED: This is for API-compatibility with the old Awesomium. Always draws while loading now.
	void										setDrawWhileLoading(const bool){};


	// Set the zoom level, where 1 = 100%, 0.25 = 25% etc.
	// Note: this is not the same value as CEF zoom levels (where 0.0 == 100%). This is percentage, like Chrome
	void										setZoom(const double percent);
	// Get the zoom level, where 1 = 100%, 0.25 = 25% etc.
	// Note: this is not the same value as CEF zoom levels (where 0.0 == 100%). This is percentage, like Chrome
	double										getZoom() const;

	// Actions
	void										goBack();
	void										goForward();
	void										reload(const bool ignoreCache = false);
	bool										isLoading();
	void										stop();
	bool										canGoBack();
	bool										canGoForward();

	const std::wstring&							getPageTitle(){ return mTitle; }

	//--- Page Callbacks ----------------------------------- //
	// The page's title has been updated (this may happen multiple times per page load)
	void										setTitleChangedFn(const std::function<void(const std::wstring& newTitle)>&);

	// The page has been navigated to a new address
	void										setAddressChangedFn(const std::function<void(const std::string& new_address)>&);

	// The page has finished loading. This also updates the canNext / canBack properties
	void										setDocumentReadyFn(const std::function<void(void)>&);

	// Something went wrong and the page couldn't load. 
	// Passes back a string with some info (should probably pass back a more complete package of info at some point)
	void										setErrorCallback(std::function<void(const std::string&)> func);

	// The page entered or exited fullscreen. The bool will be true if in fullscreen. 
	// The content that's fullscreen'ed will take up the entire web instance. 
	void										setFullscreenChangedCallback(std::function<void(const bool)> func);

	// An error has occurred. No longer displays a text sprite for errors, simply calls back the error callback.
	// You're responsible for displaying the error message yourself
	void										setErrorMessage(const std::string &message);
	void										clearError();

	// Convenience to access various document properties. Note that
	// the document probably needs to have passed onLoaded() for this
	// to be reliable.
	ci::Vec2f									getDocumentSize();
	ci::Vec2f									getDocumentScroll();

	// Scripting.
	// Send function to object with supplied args. For example, if you want to just invoke the global
	// function "makeItHappen()" you'd call: RunJavaScript("window", "makeItHappen", ds::web::ScriptTree());
	//ds::web::ScriptTree		runJavaScript(	const std::string& object, const std::string& function,
	//										const ds::web::ScriptTree& args);
	// Register a handler for a callback from javascript
	//void					registerJavaScriptMethod(	const std::string& class_name, const std::string& method_name,
	//													const std::function<void(const ds::web::ScriptTree&)>&);

	void										executeJavascript(const std::string& theScript);

	/// Lets you disable clicking, but still scroll via "mouse wheel"
	void										setAllowClicks(const bool doAllowClicks);

	/// If true, any transparent web pages will be blank, false will have a white background for pages
	void										setWebTransparent(const bool isTransparent);


	virtual void								updateClient(const ds::UpdateParams&);
	virtual void								updateServer(const ds::UpdateParams&);
	virtual void								drawLocalClient();

protected:
	virtual void								onSizeChanged();
	virtual void								writeAttributesTo(ds::DataBuffer&);
	virtual void								readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:
	void										update(const ds::UpdateParams&);
	void										handleTouch(const ds::ui::TouchInfo&);

	void										initializeBrowser();

	ds::web::WebCefService&						mService;

	int											mBrowserId;
	unsigned char *								mBuffer;
	bool										mHasBuffer;
	ci::Vec2i									mBrowserSize; // basically the w/h of this sprite, but tracked so we only recreate the buffer when needed
	ci::gl::Texture								mWebTexture;

	ci::Vec3f									mPreviousTouchPos;
	bool										mAllowClicks;
	bool										mClickDown;
	bool										mDragScrolling;
	int											mDragScrollMinFingers;
	// Cache the page size and scroll during touch events
	ci::Vec2f									mPageSizeCache,
												mPageScrollCache;
	// Prevent the scroll from being cached more than once in an update.
	int32_t										mPageScrollCount;

	std::function<void(void)>					mDocumentReadyFn;
	std::function<void(const std::string&)>		mErrorCallback;
	std::function<void(const std::string&)>		mAddressChangedCallback;
	std::function<void(const std::wstring&)>	mTitleChangedCallback;
	std::function<void(const bool)>				mFullscreenCallback;


	// Replicated state
	std::string				mUrl;

	bool					mHasError;
	std::string				mErrorMessage;

	// CEF state, cached here so we don't need to (or can't) query the other threads/processes
	std::wstring			mTitle;
	bool					mIsLoading;
	bool					mCanBack;
	bool					mCanForward;


	// Initialization
public:
	static void				installAsServer(ds::BlobRegistry&);
	static void				installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_WEB_H_