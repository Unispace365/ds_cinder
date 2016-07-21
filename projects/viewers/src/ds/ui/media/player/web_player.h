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

	virtual void						userInputReceived();
	void								layout();

	void								showInterface();

	void								sendClick(const ci::Vec3f& globalClickPos);

	ds::ui::Web*						getWeb();
	WebInterface*						getWebInterface(); // may be nullptr if embedInterface is false

	/// UI params - replace with MediaViewerSettings?
	void								setWebViewSize(const ci::Vec2f webSize);
	void								setKeyboardParams(const ci::Vec2f keyboardPanelSize, const float keyboardKeyScale, const bool allowKeyboard);
	void								setAllowTouchToggle(const bool allowTouchToggle);

protected:

	virtual void						onSizeChanged();
	ds::ui::Web*						mWeb;
	WebInterface*						mWebInterface;
	bool								mEmbedInterface;

	ci::Vec2f							mWebSize;
	ci::Vec2f							mKeyboardPanelSize;
	float								mKeyboardKeyScale;
	bool								mKeyboardAllow;
	bool								mAllowTouchToggle;

};

} // namespace ui
} // namespace ds

#endif
