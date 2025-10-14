#pragma once

#include "ds/app/event_client.h"
#include "ds/ui/media/media_interface.h"
#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

namespace ds::ui {

class ImageButton;
class LayoutButton;
class Text;
class Web;
class SoftKeyboard;
class EntryField;
class LayoutSprite;
class ToggleContainer;

/**
 * \class WebInterface
 *			Implements forward/back and refresh
 */
class WebInterface : public MediaInterface {
  public:
	WebInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
				 const ci::Color& buttonColor, const ci::Color& backgroundColor,
				 const ds::ui::SoftKeyboardSettings& settings = ds::ui::SoftKeyboardSettings(true));

	void animateOff() override;
	void onUpdateServer(const ds::UpdateParams& p) override;

	void linkWeb(ds::ui::Web* linkedWeb);
	void updateWidgets();

	void setKeyboardKeyScale(const float newKeyScale);
	void setKeyboardAllow(const bool keyboardAllowed);
	void setAllowNativeKeyboard(bool nativeKeyboardAllowed);
	void setAllowNativeKeyboardOnly(bool nativeKeyboardOnlyAllowed);
	void setKeyboardAbove(const bool keyboardAbove);
	void setKeyboardOnTop(const bool keyboardOnTop);
	void setKeyboardStateCallback(std::function<void(const bool onscreen)> func) { mKeyboardStatusCallback = func; }

	void setMessageCallback(std::function<void(const std::string&, const std::string&, int line)> func) {
		mMessageCallback = func;
	}

	void setAllowTouchToggle(const bool allowTouchToggling) override;
	void toggleTouch() override; // what the "touch lock" does
	void startTouch() override;	 // web is tappable
	void stopTouch() override;	 // web is not tappable

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

	virtual void setToggleLockedImage(const std::string& imgPath);
	virtual void setToggleUnlockedImage(const std::string& imgPath);
	virtual void setToggleLockedColor(const ci::ColorAf& color);
	virtual void setToggleUnlockedColor(const ci::ColorAf& color);

	ds::ui::ToggleContainer* getKeyboardButton() const { return mKeyboardButton; }
	ds::ui::LayoutButton*	 getBackButton() const { return mBackButton; }
	ds::ui::LayoutButton*	 getForwardButton() const { return mForwardButton; }
	ds::ui::LayoutButton*	 getRefreshButton() const { return mRefreshButton; }
	ds::ui::ToggleContainer* getTouchToggleButton() const { return mTouchToggle; }
	ds::ui::Sprite*			 getKeyboardArea() const { return mKeyboardArea; }

	ds::ui::SoftKeyboard* getSoftKeyboard() const { return mKeyboard; }
	void				  setSoftKeyboardSettings(ds::ui::SoftKeyboardSettings& keyb) { mKeyboardSettings = keyb; }

  protected:
	void onLayout() override;

	ds::EventClient mEventClient;

	ds::ui::Web* mLinkedWeb;

	ds::ui::Sprite*				 mKeyboardArea;
	ds::ui::SoftKeyboard*		 mKeyboard;
	ds::ui::SoftKeyboardSettings mKeyboardSettings;
	bool						 mKeyboardShowing;
	bool						 mKeyboardAllowed;
	bool						 mKeyboardAbove;
	bool						 mKeyboardOnTop;
	bool						 mKeyboardAutoDisablesTimeout;
	std::function<void(bool)>	 mKeyboardStatusCallback = nullptr;

	std::function<void(const std::string&, const std::string&, int line)> mMessageCallback = nullptr;

	float mKeyboardKeyScale;

	bool mKeyboardVisible		   = false;
	bool mEnableNativeKeyboard	   = false;
	bool mEnableNativeKeyboardOnly = false;
	bool mAbleToTouchToggle;

	ds::ui::ToggleContainer* mKeyboardButton;
	ds::ui::LayoutButton*	 mBackButton;
	ds::ui::LayoutButton*	 mForwardButton;
	ds::ui::LayoutButton*	 mRefreshButton;
	ds::ui::ToggleContainer* mTouchToggle;

	bool				  mAuthorizing;
	ds::ui::LayoutSprite* mAuthLayout;
	ds::ui::EntryField*	  mUserField;
	ds::ui::EntryField*	  mPasswordField;
	std::string			  mToggleLockedImage   = "%APP%/data/images/media_interface/touch_locked.png";
	std::string			  mToggleUnlockedImage = "%APP%/data/images/media_interface/touch_unlocked.png";
	ci::ColorAf			  mToggleLockedColor   = ci::ColorAf(0.0, 0.0, 0.0, 1.0);
	ci::ColorAf			  mToggleUnlockedColor = ci::ColorAf(1.0, 1.0, 1.0, 1.0);

	std::string mLastUrl = "";
	int			mInitialSize;
};

struct WebKeyboardEvent {
	WebKeyboardEvent(WebInterface* interface, Sprite* keyboard)
	  : mInterface(interface)
	  , mKeyboard(keyboard) {}

	bool hasParent(const Sprite* parent) const {
		Sprite* sprite = mKeyboard;
		while (sprite) {
			if (sprite == parent) return true;
			sprite = sprite->getParent();
		}
		return false;
	}

	WebInterface* mInterface = nullptr;
	Sprite*		  mKeyboard	 = nullptr;
};

struct WebKeyboardShownEvent : public WebKeyboardEvent, public ds::RegisteredEvent<WebKeyboardShownEvent> {
	WebKeyboardShownEvent(WebInterface* interface, Sprite* keyboard)
	  : WebKeyboardEvent(interface, keyboard) {}
};

struct WebKeyboardHiddenEvent : public WebKeyboardEvent, public ds::RegisteredEvent<WebKeyboardHiddenEvent> {
	WebKeyboardHiddenEvent(WebInterface* interface, Sprite* keyboard)
	  : WebKeyboardEvent(interface, keyboard) {}
};

} // namespace ds::ui
