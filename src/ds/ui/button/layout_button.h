#pragma once
#ifndef DS_UI_BUTTON_LAYOUT_BUTTON
#define DS_UI_BUTTON_LAYOUT_BUTTON

#include <ds/ui/button/button.h>
#include <ds/ui/layout/layout_sprite.h>

namespace ds { namespace ui {

	/**
	 * \class LayoutButton
	 *	A convenience class that's basically a SpriteButton that also can Layout like a LayoutSprite.
	 *	NOTE: changing the layout type of this sprite may make up/down weird
	 *	This sprite is set to layoutNone, which passes the runLayout() recursive call down, but doesn't affect the size
	 *or position of it's children The up/down sprites are set to fillsize and kLayoutSize, so changing the size of this
	 *(and running the layout) will make the up/down the same size and affect the size of it's children
	 */
	class LayoutButton : public LayoutSprite, public IButton {
	  public:
		LayoutButton(SpriteEngine& eng, float width = 0.0f, float height = 0.0f);

		float getPad() const override { return 0; } /* TODO */
		void  setTouchPad(float) override {}		/* TODO */

		/// the amount of time the images take fading between themselves
		void setAnimationDuration(float dur) override { mAnimDuration = dur; }

		void setClickFn(const std::function<void()>& func) override { mClickFn = func; }

		/// The visual state has been updated (down or up) pressed = down.
		void setStateChangeFn(const std::function<void(bool pressed)>& func) override { mStateChangeFunction = func; }

		LayoutSprite& getNormalSprite() const override { return mUp; }
		LayoutSprite& getHighSprite() const override { return mDown; }

		void setNormalSpriteColor(const ci::ColorA& color) const { mUp.setColor(color); }
		void setHighSpriteColor(const ci::ColorA& color) const { mDown.setColor(color); }

		/// Provides backward compatibility with the old image button
		#pragma deprecated(setNormalImage)
		void setNormalImage(const std::string& url, int flags = 0) const {
			// Not implemented, but this needs to exist for compatibility!
		}
		/// Provides backward compatibility with the old image button
		#pragma deprecated(setHighImage)
		void setHighImage(const std::string& url, int flags = 0) const {
			// Not implemented, but this needs to exist for compatibility!
		}
		/// Provides backward compatibility with the old image button
		void setNormalImageColor(const ci::ColorA& color) const { setNormalSpriteColor(color); }
		/// Provides backward compatibility with the old image button
		void setHighImageColor(const ci::ColorA& color) const { setHighSpriteColor(color); }

		void showDown() const override;
		void showUp() const override;

		ButtonBehaviour::State getButtonState() override { return mButtonBehaviour.getState(); }

	  private:
		void					  onClicked() const;
		std::function<void()>	  mClickFn;
		std::function<void(bool)> mStateChangeFunction;

		/// VIEW
		LayoutSprite& mDown;
		LayoutSprite& mUp;

		/// TOUCH
		ButtonBehaviour mButtonBehaviour;

		/// SETTINGS
		float mAnimDuration;
	};

}} // namespace ds::ui

#endif
