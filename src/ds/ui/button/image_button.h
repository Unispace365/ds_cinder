#pragma once
#ifndef DS_UI_BUTTON_IMAGE_BUTTON
#define DS_UI_BUTTON_IMAGE_BUTTON

#include <ds/ui/button/button.h>
#include <ds/ui/sprite/image.h>

namespace ds { namespace ui {

	/**
	 * \class ImageButton
	 *	A convenience class to create a button from an image or two
	 */
	class ImageButton : public Sprite, public IButton {
	  public:
		static ImageButton& makeButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage,
									   float touchPad = 0.0f, Sprite* parent = nullptr);

		ImageButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage, float touchPad = 0.0f);

		float getPad() const override { return mPad; }
		void  setTouchPad(float touchPad) override;

		/// the amount of time the images take fading between themselves
		void setAnimationDuration(float dur) override { mAnimDuration = dur; }

		/// When the button has been clicked (touch released inside)
		void setClickFn(const std::function<void()>& func) override { mClickFn = func; }

		/// The visual state has been updated (down or up) pressed = down.
		void setStateChangeFn(const std::function<void(bool pressed)>& func) override { mStateChangeFunction = func; }

		Sprite& getNormalSprite() const override { return mUp; }
		Sprite& getHighSprite() const override { return mDown; }

		Image&		getNormalImage() const { return mUp; }
		void		setNormalImage(const std::string& imageFile, int flags = 0);
		std::string getNormalImagePath() { return mNormalFilePath; }

		Image&		getHighImage() const { return mDown; } // http://i.imgur.com/1qIw7AV.jpg
		void		setHighImage(const std::string& imageFile, int flags = 0);
		std::string getHighImagePath() { return mHighFilePath; }

		void	  setNormalImageColor(const ci::Color& upColor) const;
		void	  setNormalImageColor(const ci::ColorA& upColor) const;
		ci::Color getNormalImageColor() const { return mUp.getColor(); }

		/// Set the color of the image when pressed. Let's you use the same image for both and still have feedback
		void	  setHighImageColor(const ci::Color& downColor) const;
		void	  setHighImageColor(const ci::ColorA& downColor) const;
		ci::Color getHighImageColor() const { return mDown.getColor(); }

		void layout();

		void showDown() const override;
		void showUp() const override;

		ButtonBehaviour::State getButtonState() override { return mButtonBehaviour.getState(); }

	  private:
		void					  onClicked() const;
		std::function<void()>	  mClickFn;
		std::function<void(bool)> mStateChangeFunction;

		/// VIEW
		Image&		mDown;
		Image&		mUp;
		std::string mHighFilePath;
		std::string mNormalFilePath;

		/// TOUCH
		ButtonBehaviour mButtonBehaviour;

		/// SETTINGS
		float mPad;
		float mAnimDuration;
	};

}} // namespace ds::ui

#endif
