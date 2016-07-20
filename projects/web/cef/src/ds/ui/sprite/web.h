#pragma once
#ifndef DS_UI_SPRITE_WEB_H_
#define DS_UI_SPRITE_WEB_H_

#include <cinder/app/KeyEvent.h>
#include <cinder/app/MouseEvent.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/text.h"


namespace ds {
namespace web {
	class Service;
}

namespace ui {
/**
 * \class ds::ui::Web
 * \brief Display a web page.
 */
class Web : public ds::ui::Sprite {
public:
	Web(ds::ui::SpriteEngine &engine, float width = 0.0f, float height = 0.0f);
	~Web();

	virtual void			updateClient(const ds::UpdateParams&);
	virtual void			updateServer(const ds::UpdateParams&);
	virtual void			drawLocalClient();

	// Loads the new url in the main frame (what you'd expect to happen)
	void					loadUrl(const std::wstring &url);
	void					loadUrl(const std::string &url);

	// setURL is identical to loadUrl. No exceptions will be thrown from any set or load url function.
	// load/set url are all kept around for compatibility with the old APIs
	void					setUrl(const std::string&);
	void					setUrlOrThrow(const std::string&);

	std::string				getUrl();

	// untested!
	void					sendKeyDownEvent(const ci::app::KeyEvent &event);
	// untested!
	void					sendKeyUpEvent(const ci::app::KeyEvent &event);

	// This web sprite handles touch-to-mouse events by default.
	// Though you can use these to roll your own touch stuff
	void					sendMouseDownEvent(const ci::app::MouseEvent &event);
	void					sendMouseDragEvent(const ci::app::MouseEvent &event);
	void					sendMouseUpEvent(const ci::app::MouseEvent &event);

	void					sendMouseClick(const ci::Vec3f& globalClickPoint);

	// DEPRECATED: This is for API-compatibility with the old Awesomium. Always draws while loading now.
	void					setDrawWhileLoading(const bool){};

	// Clients can listen to touch events. Kind of a hack to
	// try and sync client/server arrangements.
//	void					setTouchListener(const std::function<void(const ds::ui::TouchEvent&)>&);
	// Intended to be set as a result of the server sending out events from setTouchListener results.
	//void					handleListenerTouchEvent(const ds::ui::TouchEvent&);


	// Get the zoom level, where 1 = 100%, 0.25 = 25% etc.
	void					setZoom(const double);
	double					getZoom() const;

	// Actions
	void					goBack();
	void					goForward();
	void					reload();
	bool					isLoading();
	void					stop();
	bool					canGoBack();
	bool					canGoForward();

	// For now, a simple communication about when the address changes.
	// In the future I'd like to have a richer mechanism in place.
	void					setAddressChangedFn(const std::function<void(const std::string& new_address)>&);
	void					setDocumentReadyFn(const std::function<void(void)>&);
	void					setErrorCallback(std::function<void(const std::string&)> func){ mErrorCallback = func; }

	// If the sprite is being touched by mDragScrollMinFingers or more, will send mouse scroll events to the web view.
	void					setDragScrolling(const bool doScrolling){ mDragScrolling = doScrolling; }
	void					setDragScrollingMinimumFingers(const int numFingers){ mDragScrollMinFingers = numFingers; }

	// method to show an error message
	void					setErrorMessage(const std::string &message);
	void					clearError();

	// Convenience to access various document properties. Note that
	// the document probably needs to have passed onLoaded() for this
	// to be reliable.
	ci::Vec2f				getDocumentSize();
	ci::Vec2f				getDocumentScroll();

	// Scripting.
	// Send function to object with supplied args. For example, if you want to just invoke the global
	// function "makeItHappen()" you'd call: RunJavaScript("window", "makeItHappen", ds::web::ScriptTree());
	//ds::web::ScriptTree		runJavaScript(	const std::string& object, const std::string& function,
	//										const ds::web::ScriptTree& args);
	// Register a handler for a callback from javascript
	//void					registerJavaScriptMethod(	const std::string& class_name, const std::string& method_name,
	//													const std::function<void(const ds::web::ScriptTree&)>&);

	void					executeJavascript(const std::string& theScript);

	/// Lets you disable clicking, but still scroll via "mouse wheel"
	void					setAllowClicks(const bool doAllowClicks);

	/// If true, any transparent web pages will be blank, false will have a white background for pages
	void					setWebTransparent(const bool isTransparent);

	void					handleNativeKeyEvent(const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character);

protected:
	virtual void			onSizeChanged();
	virtual void			writeAttributesTo(ds::DataBuffer&);
	virtual void			readAttributeFrom(const char attributeId, ds::DataBuffer&);
	bool					webViewDirty();

private:
	void					update(const ds::UpdateParams&);
	void					onUrlSet(const std::string&);
	void					onDocumentReady();
	void					handleTouch(const ds::ui::TouchInfo&);
//	void					sendTouchEvent(const int x, const int y, const ds::web::TouchEvent::Phase&);

	ds::web::Service&		mService;

	bool					mHasBuffer;
	ci::gl::Texture			mWebTexture;

	ci::Vec3f				mPreviousTouchPos;
	bool					mAllowClicks;
	bool					mClickDown;
	bool					mDragScrolling;
	int						mDragScrollMinFingers;
	// Cache the page size and scroll during touch events
	ci::Vec2f				mPageSizeCache,
							mPageScrollCache;
	// Prevent the scroll from being cached more than once in an update.
	int32_t					mPageScrollCount;

	std::function<void(void)>
							mDocumentReadyFn;
	std::function<void(const std::string& msg)>
							mErrorCallback;


	// Replicated state
	std::string				mUrl;

	bool					mHasError;
	std::string				mErrorMessage;

	int						mBrowserId;

	unsigned char *			mBuffer;

	// Initialization
public:
	static void				installAsServer(ds::BlobRegistry&);
	static void				installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_WEB_H_