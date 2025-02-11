#pragma once

#include "ds/app/event_client.h"
#include "ds/ui/media/media_interface.h"
#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

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
				 const ci::Color buttonColor, const ci::Color backgroundColor, ds::ui::SoftKeyboardSettings settings= ds::ui::SoftKeyboardSettings(true));

	virtual void animateOff();
	virtual void onUpdateServer(const ds::UpdateParams& p) override;

	void linkWeb(ds::ui::Web* linkedWeb);
	void updateWidgets();

	void setKeyboardKeyScale(const float newKeyScale);
	void setKeyboardAllow(const bool keyboardAllowed);
	void setAllowNativeKeyboard(bool nativeKeyboardAllowed);
	void setAllowNativeKeyboardOnly(bool nativeKeyboardOnlyAllowed);
	void setKeyboardAbove(const bool kerboardAbove);
	void setKeyboardOnTop(const bool keyboardOnTop);
	void setKeyboardStateCallback(std::function<void(const bool onscreen)> func) { mKeyboardStatusCallback = func; }

	void setMessageCallback(std::function<void(const std::string&, const std::string&, int line)> func) {
		mMessageCallback = func;
	}

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

	virtual void setToggleLockedImage(const std::string& imgPath);
	virtual void setToggleUnlockedImage(const std::string& imgPath);
	virtual void setToggleLockedColor(const ci::ColorAf& color);
	virtual void setToggleUnlockedColor(const ci::ColorAf& color);

	ds::ui::ImageButton* getKeyboardButton() { return mKeyboardButton; }
	ds::ui::ImageButton* getBackButton() { return mBackButton; }
	ds::ui::ImageButton* getForwardButton() { return mForwardButton; }
	ds::ui::ImageButton* getRefreshButton() { return mRefreshButton; }
	ds::ui::ImageButton* getTouchToggleButton() { return mTouchToggle; }
	ds::ui::Sprite*		 getKeyboardArea() { return mKeyboardArea; }

	ds::ui::SoftKeyboard* getSoftKeyboard() { return mKeyboard; }
	void				  setSoftKeyboardSettings(ds::ui::SoftKeyboardSettings& keyb) { mKeyboardSettings = keyb; }
  protected:
	virtual void onLayout();

	ds::EventClient mEventClient;

	ds::ui::Web* mLinkedWeb;

	ds::ui::Sprite*			  mKeyboardArea;
	ds::ui::SoftKeyboard*	  mKeyboard;
	ds::ui::SoftKeyboardSettings mKeyboardSettings;
	bool					  mKeyboardShowing;
	bool					  mKeyboardAllowed;
	bool					  mKeyboardAbove;
	bool					  mKeyboardOnTop;
	bool					  mKeyboardAutoDisablesTimeout;
	std::function<void(bool)> mKeyboardStatusCallback = nullptr;

	std::function<void(const std::string&, const std::string&, int line)> mMessageCallback = nullptr;

	float mKeyboardKeyScale;

	bool mEnableNativeKeyboard = false;
	bool mEnableNativeKeyboardOnly = false;
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
	std::string			  mToggleLockedImage   = "%APP%/data/images/media_interface/touch_locked.png";
	std::string			  mToggleUnlockedImage = "%APP%/data/images/media_interface/touch_unlocked.png";
	ci::ColorAf			  mToggleLockedColor   = ci::ColorAf(0.0, 0.0, 0.0, 1.0);
	ci::ColorAf			  mToggleUnlockedColor   = ci::ColorAf(1.0, 1.0, 1.0, 1.0);

	std::string			  mLastUrl			   = "";
	int					  mInitialSize;
};

struct WebKeyboardEvent : public ds::RegisteredEvent<WebKeyboardEvent> {
	WebKeyboardEvent(Sprite* keyboard)
	  : mKeyboard(keyboard) {}
	bool hasParent(const Sprite* parent) const {
		Sprite* sprite = mKeyboard;
		while (sprite) {
			if (sprite == parent) return true;
			sprite = sprite->getParent();
		}
		return false;
	}

	Sprite* mKeyboard = nullptr;
};

struct WebKeyboardShownEvent : public WebKeyboardEvent {
	WebKeyboardShownEvent(Sprite* keyboard)
	  : WebKeyboardEvent(keyboard) {}
};

struct WebKeyboardHiddenEvent : public WebKeyboardEvent {
	WebKeyboardHiddenEvent(Sprite* keyboard)
	  : WebKeyboardEvent(keyboard) {}
};

} // namespace ds::ui
