#pragma once
#ifndef DS_UI_SPRITE_WEB_H_
#define DS_UI_SPRITE_WEB_H_

#include <cinder/app/KeyEvent.h>
#include <cinder/app/MouseEvent.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/web_listener.h"

namespace Awesomium {
class WebCore;
class WebView;
}

namespace ds {
namespace web {
class Service;
class WebViewListener;
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

	void updateServer(const ds::UpdateParams &updateParams);
	void drawLocalClient();

	// After setting a URL, you need to call activate() to see anything. Not
	// sure I like that API but that's what it is for now.
	void					loadUrl(const std::wstring &url);
	void					loadUrl(const std::string &url);
	std::string				getUrl();

	// untested!
	void sendKeyDownEvent(const ci::app::KeyEvent &event);
	// untested!
	void sendKeyUpEvent(const ci::app::KeyEvent &event);

	// This web sprite handles touch-to-mouse events by default.
	// Though you can use these to roll your own touch stuff
	void sendMouseDownEvent(const ci::app::MouseEvent &event);
	void sendMouseDragEvent(const ci::app::MouseEvent &event);
	void sendMouseUpEvent(const ci::app::MouseEvent &event);
	// Clients can listen to touch events. Kind of a hack to
	// try and sync client/server arrangements.
	void					setTouchListener(const std::function<void(const ds::web::TouchEvent&)>&);
	// Intended to be set as a result of the server sending out events from setTouchListener results.
	void					handleListenerTouchEvent(const ds::web::TouchEvent&);

	bool isActive() const;
	bool isLoading();
	void setTransitionTime(const float transitionTime);
	void activate();
	void deactivate();

	// Get the zoom level, where 1 = 100%, 0.25 = 25% etc.
	void					setZoom(const double);
	double					getZoom() const;

	// Actions
	void					goBack();
	void					goForward();
	void					reload();
	bool					canGoBack();
	bool					canGoForward();			

	// For now, a simple communication about when the address changes.
	// In the future I'd like to have a richer mechanism in place.
	void					setAddressChangedFn(const std::function<void(const std::string& new_address)>&);

	// allows the view to be updated while the page is still being loaded. default=false
	void					setDrawWhileLoading(const bool doDrawing){ mDrawWhileLoading = doDrawing; }

	// If the sprite is being touched by mDragScrollMinFingers or more, will send mouse scroll events to the web view.
	void					setDragScrolling(const bool doScrolling){ mDragScrolling = doScrolling; }
	void					setDragScrollingMinimumFingers(const int numFingers){ mDragScrollMinFingers = numFingers; }

	// Convenience to access various document properties. Note that
	// the document probably needs to have passed onLoaded() for this
	// to be reliable.
	ci::Vec2f				getDocumentSize();
	ci::Vec2f				getDocumentScroll();

void testJs();

protected:
	virtual void			onSizeChanged();

private:
	void					handleTouch(const ds::ui::TouchInfo&);
	void					sendTouchEvent(const int x, const int y, const ds::web::TouchEvent::Phase&);

	typedef ds::ui::Sprite	inherited;

	ds::web::Service&		mService;
	Awesomium::WebView*		mWebViewPtr;
	std::unique_ptr<ds::web::WebViewListener>
							mWebViewListener;

	ci::gl::Texture			mWebTexture;
	ci::gl::Texture			mLoadingTexture;

	float					mLoadingAngle;
	bool					mActive;
	float					mTransitionTime;
	bool					mDrawWhileLoading;

	ci::Vec3f				mPreviousTouchPos;
	bool					mClickDown;
	bool					mDragScrolling;
	int						mDragScrollMinFingers;

	std::function<void(const ds::web::TouchEvent&)>
							mTouchListener;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_WEB_H_
