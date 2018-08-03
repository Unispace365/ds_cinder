#pragma once
#ifndef DS_UI_MEDIA_MEDIA_INTERFACE
#define DS_UI_MEDIA_MEDIA_INTERFACE


#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/**
* \class MediaInterface
*			Abstract base class for the other media interfaces (PDF, Web, Video)
*			In this context, Interface refers to the set of buttons to control a media item (next/back pages, back/forward navigate, refresh, play/pause, scrub bar, volume control)
*/
class MediaInterface : public ds::ui::Sprite  {
public:
	MediaInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey = ci::vec2(400.0f, 50.0f), const ci::Color backgroundColor = ci::Color::black());


	virtual void						animateOn();
	virtual void						animateOff();

	virtual void						onUpdateServer(const ds::UpdateParams& updateParams) override;
	void								layout();

	void								setAnimateDuration(const float animDuration){ mAnimateDuration = animDuration; }

	/// allows the interface to timeout and hide itself after a period of time
	/// e.g. when a web interface has a keyboard displaying, the interface doesn't idle timeout
	void								setCanTimeout(const bool canTimeout){ mCanIdle = canTimeout; }

	/// allows the interface to be shown at all (rare edge case when you temporarily want to hide this)
	void								setAllowDisplay(const bool canDisplay){ mCanDisplay = canDisplay; }
	
	void								setBackgroundColorA(const ci::ColorA backgroundColor);

protected:

	float								mAnimateDuration;
	ds::ui::Sprite*						mBackground;
	float								mMinWidth;
	float								mMaxWidth;
	bool								mIdling;
	bool								mCanIdle;
	bool								mCanDisplay;
	float								mInterfaceIdleSettings;
	virtual void						onLayout(){};
	virtual void						onSizeChanged();
};

} // namespace ui
} // namespace ds

#endif
