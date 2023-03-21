#pragma once

#include "ds/app/event_client.h"
#include "ds/ui/media/media_interface.h"

namespace ds::ui {

class ImageButton;
class Text;
class Web;
class SoftKeyboard;
class EntryField;
class LayoutSprite;

/**
 * \class WebInterface
 *			Implements forward/back and refresh
 */
class WebInterface : public MediaInterface {
  public:
	WebInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
				 const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void animateOff();

	void linkWeb(ds::ui::Web* linkedWeb);
	void updateWidgets();

	void setKeyboardKeyScale(const float newKeyScale);
	void setKeyboardAllow(const bool keyboardAllowed);
	void setKeyboardAbove(const bool kerboardAbove);
	void setKeyboardStateCallback(std::function<void(const bool onscreen)> func) { mKeyboardStatusCallback = func; }

	void setAllowTouchToggle(const bool allowTouchToggling);

	/// If true, will keep the interface onscreen when the keyboard is on
	/// If false, will allow timeouts when the keyboard is on (note: recommend to use this with setCanTimeout(false)
	/// on the base MediaInterface to persist the interface)
	void setKeyboardDisablesTimeout(const bool doAutoTimeout);

	/// The browser has request a login (like when you hit an ftp site)
	void startAuthCallback(const std::string& host, const std::string& realm);
	void cancelAuth();
	void authComplete();

	void showKeyboard(bool show);
	void toggleKeyboard();

	void toggleTouch(); // what the "touch lock" does
	void startTouch();	// web is tappable
	void stopTouch();	// web is not tappable

	bool isLocked() { return mWebLocked; }


	ds::ui::ImageButton* getKeyboardButton() { return mKeyboardButton; }
	ds::ui::ImageButton* getBackButton() { return mBackButton; }
	ds::ui::ImageButton* getForwardButton() { return mForwardButton; }
	ds::ui::ImageButton* getRefreshButton() { return mRefreshButton; }
	ds::ui::ImageButton* getTouchToggleButton() { return mTouchToggle; }

	ds::ui::SoftKeyboard* getSoftKeyboard() { return mKeyboard; }

  protected:
	virtual void onLayout();

	ds::EventClient mEventClient;

	ds::ui::Web* mLinkedWeb;

	ds::ui::Sprite*			  mKeyboardArea;
	ds::ui::SoftKeyboard*	  mKeyboard;
	bool					  mKeyboardShowing;
	bool					  mKeyboardAllowed;
	bool					  mKeyboardAbove;
	bool					  mKeyboardAutoDisablesTimeout;
	std::function<void(bool)> mKeyboardStatusCallback = nullptr;

	float mKeyboardKeyScale;

	bool mAbleToTouchToggle;
	bool mWebLocked;

	ds::ui::ImageButton* mKeyboardButton;
	ds::ui::ImageButton* mBackButton;
	ds::ui::ImageButton* mForwardButton;
	ds::ui::ImageButton* mRefreshButton;
	ds::ui::ImageButton* mTouchToggle;

	bool				  mAuthorizing;
	ds::ui::LayoutSprite* mAuthLayout;
	ds::ui::EntryField*	  mUserField;
	ds::ui::EntryField*	  mPasswordField;
};

} // namespace ds::ui
