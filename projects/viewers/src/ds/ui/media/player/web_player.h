#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_PLAYER
#define DS_UI_MEDIA_VIEWER_WEB_PLAYER


#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {
class Web;
class WebInterface;
class WebYoutubeInterface;
class MediaInterface;
struct MediaViewerSettings;

/**
 * \class WebPlayer
 *			Creates a web sprite and puts an interface on top of it.
 */
class WebPlayer : public ds::ui::Sprite {
  public:
	WebPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface);

	void setMedia(const std::string mediaPath);

	virtual void userInputReceived();
	void		 layout();

	void showInterface();
	void hideInterface();

	void sendClick(const ci::vec3& globalClickPos);

	ds::ui::Web*  getWeb();
	WebInterface* getWebInterface();  // may be nullptr if embedInterface is false
	WebYoutubeInterface* getWebYoutubeInterface();  // may be nullptr if embedInterface is false
	bool		  getIsYoutube() { return mIsYoutube; }

	/// Sets all applicable settings from a MediaViewerSettings
	void setMediaViewerSettings(const MediaViewerSettings& settings);

	void setWebViewSize(const ci::vec2 webSize);
	void setKeyboardParams(const float keyboardKeyScale, const bool allowKeyboard, const bool keyboardAbove);
	void setKeyboardStateCallback(std::function<void(const bool onscreen)> func);
	void setAllowTouchToggle(const bool allowTouchToggle);
	void setShowInterfaceAtStart(const bool showInterfaceAtStart);
	void setStartInteractable(const bool startInteractable);
	void setLetterbox(const bool doLetterbox);
	void setIsYoutube(const bool isYoutube);


  protected:
	virtual void onSizeChanged();

	ds::ui::Web*  mWeb;
	WebInterface* mWebInterface;
	WebYoutubeInterface* mWebYoutubeInterface;
	ci::vec2	  mWebSize;
	float		  mKeyboardKeyScale;
	std::function<void(bool)>	mKeyboardStatusCallback = nullptr;

	bool mEmbedInterface;
	bool mShowInterfaceAtStart;
	bool mLetterbox;
	bool mKeyboardAllow;
	bool mKeyboardAbove;
	bool mAllowTouchToggle;
	bool mStartInteractable;
	bool mInterfaceBelowMedia;
	bool mIsYoutube;
};

}  // namespace ui
}  // namespace ds

#endif
