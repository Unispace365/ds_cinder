#pragma once
#ifndef DS_UI_BUTTON_BUTTON
#define DS_UI_BUTTON_BUTTON

#include <ds/ui/touch/button_behaviour.h>

namespace ds { namespace ui {

	/// Interface for a button. Derived classes should implement the actual button.
	class IButton {
	  public:
		virtual ~IButton() = default;

		/// Returns the additional padding around the button used for touch detection.
		virtual float getPad() const = 0;
		/// Sets the additional padding around the button used for touch detection.
		virtual void setTouchPad(float padding) = 0;

		/// Sets the duration of animations.
		virtual void setAnimationDuration(float duration) = 0;

		/// Function called when the button has been clicked (touch released inside).
		virtual void setClickFn(const std::function<void()>&) = 0;

		/// Function called when the visual state has been updated (down or up).
		virtual void setStateChangeFn(const std::function<void(bool pressed)>&) = 0;

		/// Returns the sprite used for the button's up state.
		virtual Sprite& getNormalSprite() const = 0;

		/// Returns the sprite used for the button's down state.
		virtual Sprite& getHighSprite() const = 0;

		/// Returns the current state of the button.
		virtual ButtonBehaviour::State getButtonState() = 0;

		/// Sets the current state of the button to down.
		virtual void showDown() const = 0;

		/// Sets the current state of the button to up.
		virtual void showUp() const = 0;
	};

}} // namespace ds::ui

#endif