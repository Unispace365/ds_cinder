#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_INTERFACE
#define DS_UI_MEDIA_VIEWER_WEB_INTERFACE

#include "ds/ui/media/media_interface.h"
#include "ds/app/event_client.h"

namespace ds {
namespace ui {

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
class WebInterface : public MediaInterface  {
public:
	WebInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void				animateOff();

	void						linkWeb(ds::ui::Web* linkedWeb);
	void						updateWidgets();

	void						setKeyboardKeyScale(const float newKeyScale);
	void						setKeyboardAllow(const bool keyboardAllowed);
	void						setKeyboardAbove(const bool kerboardAbove);
	void						setKeyboardStateCallback(std::function<void(const bool onscreen)> func) { mKeyboardStatusCallback = func; }

	void						setAllowTouchToggle(const bool allowTouchToggling);
	
	/// If true, will keep the interface onscreen when the keyboard is on
	/// If false, will allow timeouts when the keyboard is on (note: recommend to use this with setCanTimeout(false) on the base MediaInterface to persist the interface)
	void						setKeyboardDisablesTimeout(const bool doAutoTimeout);

	/// The browser has request a login (like when you hit an ftp site)
	void						startAuthCallback(const std::string& host, const std::string& realm);
	void						cancelAuth();
	void						authComplete();

	void						showKeyboard(bool show);
	void                        toggleKeyboard();


	ds::ui::ImageButton*		getKeyboardButton() { return mKeyboardButton; }
	ds::ui::ImageButton*		getBackButton() { return mBackButton; }
	ds::ui::ImageButton*		getForwardButton() { return mForwardButton; }
	ds::ui::ImageButton*		getRefreshButton() { return mRefreshButton; }
	ds::ui::ImageButton*		getTouchToggleButton() { return mTouchToggle; }

protected:

	virtual void				onLayout();

	ds::EventClient				mEventClient;

	ds::ui::Web*				mLinkedWeb;

	ds::ui::Sprite*				mKeyboardArea;
	ds::ui::SoftKeyboard*		mKeyboard;
	bool						mKeyboardShowing;
	bool						mKeyboardAllowed;
	bool						mKeyboardAbove;
	bool						mKeyboardAutoDisablesTimeout;
	std::function<void(bool)>	mKeyboardStatusCallback = nullptr;

	float						mKeyboardKeyScale;

	bool						mAbleToTouchToggle;
	bool						mWebLocked;

	ds::ui::ImageButton*		mKeyboardButton;
	ds::ui::ImageButton*		mBackButton;
	ds::ui::ImageButton*		mForwardButton;
	ds::ui::ImageButton*		mRefreshButton;
	ds::ui::ImageButton*		mTouchToggle;

	bool						mAuthorizing;
	ds::ui::LayoutSprite*		mAuthLayout;
	ds::ui::EntryField*			mUserField;
	ds::ui::EntryField*			mPasswordField;

};

} // namespace ui
} // namespace ds

#endif
