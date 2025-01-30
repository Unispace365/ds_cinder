#pragma once
#ifndef DS_UI_BUTTON_SPRITE_BUTTON
#define DS_UI_BUTTON_SPRITE_BUTTON

#include <ds/ui/button/button.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {

	// This allows us to efficiently resize whenever attached content changes.
	class AutoSizeSprite : public Sprite {
	  public:
		AutoSizeSprite(ds::ui::SpriteEngine& eng, float width = 0.0f, float height = 0.0f)
		  : Sprite(eng, width, height) {}

		void onChildAdded(ds::ui::Sprite& child) override {
			child.setDimensionsChangedCallback([this](Sprite*) { handleResize(); });
			handleResize();
		}

		void onChildRemoved(ds::ui::Sprite& child) override {
			child.setDimensionsChangedCallback(nullptr);
			handleResize();
		}

	  protected:
		virtual void handleResize() {
			float w = 0.0f;
			float h = 0.0f;
			for (auto child : mChildren) {
				w = std::max(w, child->getScale().x * child->getWidth() + child->getPosition().x);
				h = std::max(h, child->getScale().y * child->getHeight() + child->getPosition().y);
			}
			setSize(w, h);
		}
	};

	/**
	 * \class SpriteButton
	 *	A convenience class to create a button for some basic sprites
	 */
	class SpriteButton : public Sprite, public IButton {
	  public:
		SpriteButton(SpriteEngine& eng, float width = 0.0f, float height = 0.0f);

		float getPad() const override { return mPad; }
		void  setTouchPad(float touchPad) override;

		/// the amount of time the images take fading between themselves
		void setAnimationDuration(float dur) override { mAnimDuration = dur; }

		void setClickFn(const std::function<void()>& func) override { mClickFn = func; }

		/// The visual state has been updated (down or up) pressed = down.
		void setStateChangeFn(const std::function<void(bool pressed)>& func) override { mStateChangeFunction = func; }

		Sprite& getNormalSprite() const override { return mUp; }
		Sprite& getHighSprite() const override { return mDown; } // http://i.imgur.com/1qIw7AV.jpg

		void showDown() const override;
		void showUp() const override;

		ButtonBehaviour::State getButtonState() override { return mButtonBehaviour.getState(); }

		YGSize yogaMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height,
							   YGMeasureMode heightMode) override;

	  private:
		void handleResize();

		void					  onClicked() const;
		std::function<void()>	  mClickFn;
		std::function<void(bool)> mStateChangeFunction;

		/// VIEW
		Sprite& mDown;
		Sprite& mUp;

		/// TOUCH
		ButtonBehaviour mButtonBehaviour;

		/// SETTINGS
		float mPad;
		float mAnimDuration;
	};

}} // namespace ds::ui

#endif
