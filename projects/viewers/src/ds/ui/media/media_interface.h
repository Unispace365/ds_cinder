#pragma once
#ifndef DS_UI_MEDIA_MEDIA_INTERFACE
#define DS_UI_MEDIA_MEDIA_INTERFACE


#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::MediaInterface
*			Abstract base class for the other media interfaces (PDF, Web, Video)
*			In this context, Interface refers to the set of buttons to control a media item (next/back pages, back/forward navigate, refresh, play/pause, scrub bar, volume control)
*/
class MediaInterface : public ds::ui::Sprite  {
public:
	MediaInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& sizey = ci::Vec2f(400.0f, 50.0f), const ci::Color backgroundColor = ci::Color::black());


	void								animateOn();
	void								animateOff();
	void								turnOn();
	void								turnOff();

	virtual void						updateServer(const ds::UpdateParams& updateParams);
	void								layout();

	void								setAnimateDuration(const float animDuration){ mAnimateDuration = animDuration; }

protected:

	float								mAnimateDuration;
	ds::ui::Sprite*						mBackground;
	bool								mIdling;
	virtual void						onLayout(){};
	virtual void						onSizeChanged();
};

} // namespace ui
} // namespace ds

#endif
