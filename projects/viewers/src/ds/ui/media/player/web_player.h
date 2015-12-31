#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_PLAYER
#define DS_UI_MEDIA_VIEWER_WEB_PLAYER


#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {
class Web;
class WebInterface;
class MediaInterface;

/**
* \class ds::ui::WebPlayer
*			Creates a web sprite and puts an interface on top of it.
*/
class WebPlayer : public ds::ui::Sprite  {
public:
	WebPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface);

	void								setMedia(const std::string mediaPath);

	virtual void						userInputReceived(Sprite* child = nullptr);
	void								layout();

	void								showInterface();

	void								sendClick(const ci::Vec3f& globalClickPos);

	ds::ui::Web*						getWeb();

protected:

	virtual void						onSizeChanged();
	ds::ui::Web*						mWeb;
	WebInterface*						mWebInterface;
	bool								mEmbedInterface;

};

} // namespace ui
} // namespace ds

#endif
