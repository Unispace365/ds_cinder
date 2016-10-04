#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_INTERFACE
#define DS_UI_MEDIA_VIEWER_WEB_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class Web;
class SoftKeyboard;

/**
* \class ds:ui:::WebInterface
*			Implements forward/back and refresh
*/
class WebInterface : public MediaInterface  {
public:
	WebInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void				animateOff();

	void						linkWeb(ds::ui::Web* linkedWeb);
	void						updateWidgets();

	void						setKeyboardKeyScale(const float newKeyScale);
	void						setKeyboardAllow(const bool keyboardAllowed);
	void						setKeyboardAbove(const bool kerboardAbove);

	void						setAllowTouchToggle(const bool allowTouchToggling);

protected:

	virtual void				onLayout();
	void						showKeyboard(bool show);

	ds::ui::Web*				mLinkedWeb;

	ds::ui::Sprite*				mKeyboardArea;
	ds::ui::SoftKeyboard*		mKeyboard;
	bool						mKeyboardShowing;
	bool						mKeyboardAllowed;
	bool						mKeyboardAbove;

	float						mKeyboardKeyScale;

	bool						mAbleToTouchToggle;

	ds::ui::ImageButton*		mKeyboardButton;
	ds::ui::ImageButton*		mBackButton;
	ds::ui::ImageButton*		mForwardButton;
	ds::ui::ImageButton*		mRefreshButton;
	ds::ui::ImageButton*		mTouchToggle;

};

} // namespace ui
} // namespace ds

#endif
