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
	void								hideInterface();

	void								sendClick(const ci::vec3& globalClickPos);

	ds::ui::Web*						getWeb();
	WebInterface*						getWebInterface(); // may be nullptr if embedInterface is false

	/// UI params - replace with MediaViewerSettings?
	void								setWebViewSize(const ci::vec2 webSize);
	void								setKeyboardParams(const float keyboardKeyScale, const bool allowKeyboard, const bool keyboardAbove);
	void								setAllowTouchToggle(const bool allowTouchToggle);
	void								setShowInterfaceAtStart(bool showInterfaceAtStart);

protected:

	virtual void						onSizeChanged();
	ds::ui::Web*						mWeb;
	WebInterface*						mWebInterface;
	bool								mEmbedInterface;
	bool								mShowInterfaceAtStart;

	ci::vec2							mWebSize;
	float								mKeyboardKeyScale;
	bool								mKeyboardAllow;
	bool								mKeyboardAbove;
	bool								mAllowTouchToggle;

};

} // namespace ui
} // namespace ds

#endif
