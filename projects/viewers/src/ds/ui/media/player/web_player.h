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
	struct Params {
		Params(){
			viewSize.zero();
			keyboardPanelSize.set(900.0f, 450.0f);
			keyboardScale = 1.0f;
		}

		ci::Vec2f viewSize;
		ci::Vec2f keyboardPanelSize;
		float keyboardScale;
	};

	WebPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface, const Params& params = Params());

	void								setMedia(const std::string mediaPath);

	virtual void						userInputReceived();
	void								layout();

	void								showInterface();

	void								sendClick(const ci::Vec3f& globalClickPos);

	ds::ui::Web*						getWeb();

protected:

	virtual void						onSizeChanged();
	Params								mParams;
	ds::ui::Web*						mWeb;
	WebInterface*						mWebInterface;
	bool								mEmbedInterface;

};

} // namespace ui
} // namespace ds

#endif
