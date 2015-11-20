#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_INTERFACE
#define DS_UI_MEDIA_VIEWER_WEB_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class Web;

/**
* \class ds:ui:::WebInterface
*			Implements forward/back and refresh
*/
class WebInterface : public MediaInterface  {
public:
	WebInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	void						linkWeb(ds::ui::Web* linkedWeb);
	void						updateWidgets();

protected:

	virtual void				onLayout();

	ds::ui::Web*				mLinkedWeb;

	ds::ui::ImageButton*		mBackButton;
	ds::ui::ImageButton*		mForwardButton;
	ds::ui::ImageButton*		mRefreshButton;
	ds::ui::ImageButton*		mTouchToggle;

};

} // namespace ui
} // namespace ds

#endif
