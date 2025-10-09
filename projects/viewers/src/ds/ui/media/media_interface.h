#pragma once

#include <ds/ui/sprite/sprite.h>

#include <utility>

namespace ds::ui {
class LayoutButton;

/**
 * \class MediaInterface
 *		Abstract base class for the other media interfaces (PDF, Web, Video)
 *		In this context, Interface refers to the set of buttons to control a media item (next/back pages, back/forward
 *navigate, refresh, play/pause, scrub bar, volume control)
 */
class MediaInterface : public Sprite {
  public:
	MediaInterface(SpriteEngine& eng, int type, const ci::vec2& sizey = ci::vec2(400.0f, 50.0f),
				   const ci::Color& backgroundColor = ci::Color::black());

	/// Returns the type of interface (PDF, Web, Video), see ds::Resource.
	int getType() const { return mType; }

	virtual void animateOn();
	virtual void animateOff();

	void onUpdateServer(const UpdateParams& updateParams) override;
	void layout();

	virtual void setAllowTouchToggle(const bool allowTouchToggling) {}
	virtual void toggleTouch() {}
	virtual void startTouch() {}
	virtual void stopTouch() {}

	void setAnimateDuration(const float animDuration) { mAnimateDuration = animDuration; }

	/// allows the interface to timeout and hide itself after a period of time
	/// e.g. when a web interface has a keyboard displaying, the interface doesn't idle timeout
	void setCanTimeout(const bool canTimeout) { mCanIdle = canTimeout; }

	/// allows the interface to be shown at all (rare edge case when you temporarily want to hide this)
	void setAllowDisplay(const bool canDisplay) { mCanDisplay = canDisplay; }

	/// Conveniences to set the background color
	void setBackgroundColorA(const ci::ColorA& backgroundColor) const;
	void setBackgroundColor(const ci::ColorA& newColor) const;
	void setBackgroundColor(const ci::Color& newColor) const;


	void show() override;

	Sprite*		 getBackground() const { return mBackground; }
	virtual void setMaxWidth(float width) {
		mMaxWidth = width;
		layout();
	}

	virtual void setMinWidth(float width) {
		mMinWidth = width;
		layout();
	}

	void setLocked(bool isLock) {
		if (!mCanLock) return;
		mLocked = isLock;
		if (mLockChangeCallback) mLockChangeCallback(isLock);
	}
	bool isLocked() const { return mCanLock && mLocked; }
	void setLockStateCallback(std::function<void(bool)> lockChangeCallback) {
		mLockChangeCallback = std::move(lockChangeCallback);
	}

	std::string		   composeIconPath(const std::string& iconId) const;
	static std::string composeIconPath(const SpriteEngine& engine, const std::string& iconId);

	static LayoutButton* createButton(SpriteEngine& engine, const ci::vec2& sizey, const std::string& iconIdNormal,
									  const std::string& iconIdHigh);

	static float getPlayButtonHeight() { return mPlayHeight; }
	static float getPauseButtonHeight() { return mPauseHeight; }
	static float getKeyboardButtonHeight() { return mKeyboardHeight; }
	static float getBackButtonHeight() { return mBackHeight; }
	static float getForwardButtonHeight() { return mForwardHeight; }
	static float getRefreshButtonHeight() { return mRefreshHeight; }
	static float getLockButtonHeight() { return mLockHeight; }
	static float getLoopButtonHeight() { return mLoopHeight; }
	static float getVolumeButtonHeight() { return mVolumeHeight; }
	static float getThumbnailButtonHeight() { return mThumbnailHeight; }
	static float getVolumeSliderHeight() { return mVolumeSliderHeight; }
	static float getScrubBarHeight() { return mScrubBarHeight; }

	virtual void setButtonColor(LayoutButton* button, const ci::Color& normalColor, const ci::Color& highColor);

	void setSizeAll(float width, float height, float depth) override;

  protected:
	virtual void		  onLayout() {};
	void				  onSizeChanged() override;
	virtual LayoutButton* createButton(const ci::vec2& sizey, const std::string& iconIdNormal,
									   const std::string& iconIdHigh);

	int mType = Resource::ERROR_TYPE;

	Sprite* mBackground;

	float mAnimateDuration;
	float mMinWidth;
	float mMaxWidth;

	bool					  mIdling;
	bool					  mCanIdle;
	bool					  mCanDisplay;
	bool					  mCanLock;
	bool					  mLocked = false;
	std::function<void(bool)> mLockChangeCallback;

	float		 mInterfaceIdleSettings;
	static float mPlayHeight;
	static float mPauseHeight;
	static float mKeyboardHeight;
	static float mBackHeight;
	static float mForwardHeight;
	static float mRefreshHeight;
	static float mLockHeight;
	static float mLoopHeight;
	static float mVolumeHeight;
	static float mThumbnailHeight;
	static float mVolumeSliderHeight;
	static float mScrubBarHeight;
};

class MediaInterfaceShownEvent : public RegisteredEvent<MediaInterfaceShownEvent> {
	MediaInterface* mMediaInterface = nullptr;

  public:
	MediaInterfaceShownEvent(MediaInterface* mp)
	  : mMediaInterface(mp) {}

	MediaInterface* getMediaInterface() const { return mMediaInterface; }
};

class MediaInterfaceHiddenEvent : public RegisteredEvent<MediaInterfaceHiddenEvent> {
	MediaInterface* mMediaInterface = nullptr;

  public:
	MediaInterfaceHiddenEvent(MediaInterface* mp)
	  : mMediaInterface(mp) {}

	MediaInterface* getMediaInterface() const { return mMediaInterface; }
};

} // namespace ds::ui
