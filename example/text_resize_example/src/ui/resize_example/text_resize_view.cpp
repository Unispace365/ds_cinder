#include "stdafx.h"

#include "text_resize_view.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/control/control_check_box.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"


namespace text_resize {

TextResizeView::TextResizeView(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "text_resize_view.xml") {
	/// link a button to an action in the app
	setSpriteClickFn("idle_button.the_button", [this] {
		if (mEngine.isIdling()) {
			userInputReceived();
		} else {
			mEngine.startIdling();
		}
	});


	mTextSprite	  = getSprite<ds::ui::Text>("text_box");
	mTextBoxBack  = getSprite("text_box_size");
	mResizeHandle = getSprite("dragbox");
	mLimitBack	  = getSprite("text_box_limit");


	auto use_fit_chkbox		  = getSprite<ds::ui::ControlCheckBox>("do_fit_limit");
	auto use_fit_fonts_chkbox = getSprite<ds::ui::ControlCheckBox>("do_fit_font_sizes");
	auto use_wrap_chxbox	  = getSprite<ds::ui::ControlCheckBox>("do_wrap");
	auto use_maxmin			  = getSprite<ds::ui::ControlCheckBox>("do_fit_font_maxmin");


	auto fonts_input  = getSprite<ds::ui::EntryField>("sizes_entry");
	auto wrap_input	  = getSprite<ds::ui::EntryField>("text_wrap_entry");
	auto maxmin_input = getSprite<ds::ui::EntryField>("maxmin_entry");

	auto textInput = getSprite<ds::ui::EntryField>("text_entry");
	if (mTextSprite && textInput && fonts_input && use_fit_chkbox && use_fit_fonts_chkbox && use_wrap_chxbox) {

		setTapCallback([=](Sprite* sp, const ci::vec3& ti) {
			fonts_input->unfocus();
			textInput->unfocus();
			wrap_input->unfocus();
			maxmin_input->unfocus();
			fonts_input->enable(true);
			textInput->enable(true);
			wrap_input->enable(true);
			maxmin_input->enable(true);
		});

		textInput->setCurrentText(mTextSprite->getText());

		use_fit_chkbox->setCheckBoxValue(true);
		maxmin_input->unfocus();
		maxmin_input->enable(true);
		maxmin_input->setTapCallback([=](Sprite* sp, const ci::vec3& ti) {
			fonts_input->unfocus();
			textInput->unfocus();
			wrap_input->unfocus();
			maxmin_input->focus();
		});
		fonts_input->unfocus();
		fonts_input->enable(true);
		fonts_input->setTapCallback([=](Sprite* sp, const ci::vec3& ti) {
			maxmin_input->unfocus();
			textInput->unfocus();
			wrap_input->unfocus();
			fonts_input->focus();
		});
		wrap_input->unfocus();
		wrap_input->enable(true);
		wrap_input->setTapCallback([=](Sprite* sp, const ci::vec3& ti) {
			maxmin_input->unfocus();
			textInput->unfocus();
			fonts_input->unfocus();
			wrap_input->focus();
		});
		textInput->unfocus();
		textInput->enable(true);
		textInput->getTextSprite()->setResizeLimit(textInput->getWidth(), textInput->getHeight());
		textInput->setTapCallback([=](Sprite* sp, const ci::vec3& ti) {
			maxmin_input->unfocus();
			fonts_input->unfocus();
			wrap_input->unfocus();
			textInput->focus();
		});

		setSpriteClickFn("view_button", [=]() {
			mTextSprite->setFitToResizeLimit(use_fit_chkbox->getCheckBoxValue());
			if (use_fit_fonts_chkbox->getCheckBoxValue()) {
				std::regex			e3(",+");
				auto				value = fonts_input->getCurrentTextString();
				auto				itr	  = std::sregex_token_iterator(value.begin(), value.end(), e3, -1);
				std::vector<double> size_values;
				double				font_value;

				for (; itr != std::sregex_token_iterator(); ++itr) {
					if (ds::string_to_value<double>(itr->str(), font_value)) {
						size_values.push_back(font_value);
					}
				}


				mTextSprite->setFitFontSizes(size_values);
			} else {
				mTextSprite->setFitFontSizes({});
			}


			if (use_maxmin->getCheckBoxValue()) {
				std::regex			e3(",+");
				auto				value = maxmin_input->getCurrentTextString();
				auto				itr	  = std::sregex_token_iterator(value.begin(), value.end(), e3, -1);
				std::vector<double> size_values;
				double				font_value;

				for (; itr != std::sregex_token_iterator(); ++itr) {
					if (ds::string_to_value<double>(itr->str(), font_value)) {
						size_values.push_back(font_value);
					}
				}
				if (size_values.size() >= 2) {
					mTextSprite->setFitMaxFontSize(size_values[1]);
					mTextSprite->setFitMinFontSize(size_values[0]);
				}
			} else {
				mTextSprite->setFitMaxFontSize(0);
				mTextSprite->setFitMinFontSize(0);
			}

			if (use_wrap_chxbox->getCheckBoxValue()) {
				auto			 value	 = wrap_input->getCurrentTextString();
				ds::ui::WrapMode theMode = ds::ui::WrapMode::kWrapModeWordChar;
				if (value == "false" || value == "off") {
					theMode = ds::ui::WrapMode::kWrapModeOff;
				} else if (value == "word") {
					theMode = ds::ui::WrapMode::kWrapModeWord;
				} else if (value == "char") {
					theMode = ds::ui::WrapMode::kWrapModeChar;
				} else if (value == "word_char" || value == "wordchar") {
					theMode = ds::ui::WrapMode::kWrapModeWordChar;
				}


				mTextSprite->setWrapMode(theMode);
			} else {
				ds::ui::WrapMode theMode = ds::ui::WrapMode::kWrapModeWordChar;
				mTextSprite->setWrapMode(theMode);
			}


			mTextSprite->setText(textInput->getCurrentTextString());
			mIsDirty = true;
		});


		textInput->setNativeKeyboardCallback([this, textInput](ci::app::KeyEvent& event) -> bool {
			if (event.getCode() == ci::app::KeyEvent::KEY_v && event.isControlDown()) {
				if (OpenClipboard((HWND)(ci::app::App::get()->getWindow()->getNative()))) {
					auto hglb = ::GetClipboardData(CF_TEXT);
					if (hglb != NULL) {
						char* lptstr = (char*)GlobalLock(hglb);
						if (lptstr != nullptr) {
							// Call the application-defined ReplaceSelection
							// function to insert the text and repaint the
							// window.
							std::string x(lptstr);

							auto str = ds::wstr_from_utf8(x);

							textInput->setCurrentText(str);


							GlobalUnlock(hglb);
							CloseClipboard();
							return true;
						}
					}
				}
			}
			return false;
		});
	}
	if (mResizeHandle) {
		mResizeHandle->enable(true);
		mResizeHandle->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
		mResizeHandle->setProcessTouchCallback([this](Sprite* sp, const ds::ui::TouchInfo& ti) {
			if (ti.mPhase == ds::ui::TouchInfo::Moved) {
				mUserMoved = true;
				mIsDirty   = true;
			}
			if (ti.mPhase == ds::ui::TouchInfo::Removed) {
				mIsDirty = true;
			}
		});
	}
}

void TextResizeView::onUpdateServer(const ds::UpdateParams& p) {
	if (mTextBoxBack && mResizeHandle && mTextSprite && mLimitBack) {
		if (mIsDirty) {
			if (mUserMoved) {
				mTextSprite->showDebug(false);
				auto pos = mResizeHandle->getPosition();
				auto off = pos - mTextSprite->getPosition();
				mTextSprite->setResizeLimit(off.x, off.y);
				mUserMoved = false;
			} else {
				auto pos = mTextSprite->getPosition() +
						   ci::vec3(mTextSprite->getResizeLimitWidth(), mTextSprite->getResizeLimitHeight(), 0);
				mResizeHandle->setPosition(pos);
			}
			mTextBoxBack->setPosition(mTextSprite->getPosition());
			mLimitBack->setPosition(mTextSprite->getPosition());
			mTextBoxBack->setSizeAll(mTextSprite->getSize());
			mLimitBack->setSize(mTextSprite->getResizeLimitWidth(), mTextSprite->getResizeLimitHeight());
			mIsDirty = false;
		}
	}
}
} // namespace text_resize
