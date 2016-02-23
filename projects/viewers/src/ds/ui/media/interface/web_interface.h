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
	struct Params {
		Params(){
			panelSize.set(800.0f, 500.0f);
			scale = 1.0f;
		}

		ci::Vec2f panelSize;
		float scale;
	};

	WebInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor, const Params& params = Params());

	virtual void				animateOff();

	void						linkWeb(ds::ui::Web* linkedWeb);
	void						updateWidgets();

protected:

	virtual void				onLayout();
	void						showKeyboard(bool show);

	Params						mParams;
	ds::ui::Web*				mLinkedWeb;

	ds::ui::Sprite*				mKeyboardArea;
	ds::ui::SoftKeyboard*		mKeyboard;
	bool						mKeyboardShowing;

	ds::ui::ImageButton*		mKeyboardButton;
	ds::ui::ImageButton*		mBackButton;
	ds::ui::ImageButton*		mForwardButton;
	ds::ui::ImageButton*		mRefreshButton;
	ds::ui::ImageButton*		mTouchToggle;

};

} // namespace ui
} // namespace ds

#endif
