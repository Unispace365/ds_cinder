#pragma once
#ifndef UI_BUTTON_IMAGEBUTTON_H_
#define UI_BUTTON_IMAGEBUTTON_H_

#include <ds/ui/sprite/image.h>
#include "ds/ui/touch/button_behaviour.h"

namespace fullstarter {
class Globals;

/**
 * \class na::ImageButton2
 * \brief A button that is composed of an image or two
 * It defaults to a center of 0.5, 0.5.
 */
class ImageButton : public ds::ui::Sprite {
public:
	ImageButton(Globals&, const std::string& image_filename = "", const int flags = 0);

	void					setDownImage(const std::string& filename);

	// Clients use this to configure the image and text components to taste.
	// Once finished, the button will automatically be laid out and resized.
	void					configure(const std::function<void(ds::ui::Image&)>&);
	void					setOnClicked(const std::function<void(void)>& fn);

	void					setButtonEnabled(const bool = true);

protected:
	float					mTouchPadding;

private:
	void					layout();
	void					onDownFn();
	void					onEnterFn();
	void					onExitFn();
	void					onUpFn();

	typedef ds::ui::Sprite  inherited;
	Globals&				mGlobals;
	ds::ButtonBehaviour		mButtonBehaviour;
    ds::ui::Image&			mImage;
	ds::ui::Image*			mImageDown;
	static const enum		State { kStateEnabled, kStateDisabled };
	State					mState;

	const float				mTouchShrink;
	const float				mAnimDuration;
};

} // namespace fullstarter

#endif
