#include "soft_keyboard_builder.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "ds/ui/soft_keyboard/soft_keyboard.h"

namespace ds {
namespace ui {
namespace SoftKeyboardBuilder {

SoftKeyboard* buildLowercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings) {
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, "1", "!", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "2", "@", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "3", "#", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "4", "$", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "5", "%", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "6", "^", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "7", "&", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "8", "*", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "9", "(", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "0", ")", SoftKeyboardButton::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "q", "Q", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "w", "W", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "e", "E", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "r", "R", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "t", "T", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "y", "Y", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "u", "U", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "i", "I", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "o", "O", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "p", "P", SoftKeyboardButton::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "a", "A", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "s", "S", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "d", "D", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "f", "F", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "g", "G", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "h", "H", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "j", "J", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "k", "K", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "l", "L", SoftKeyboardButton::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth()* 3.0f / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "z", "Z", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "x", "X", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "c", "C", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "v", "V", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "b", "B", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "n", "N", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "m", "M", SoftKeyboardButton::kLetter);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "-", "_", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, ",", "<", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, ".", ">", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "/", "?", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "space", "SPACE", SoftKeyboardButton::kSpace);
	makeAButton(engine, newKeyboard, xp, yp, "", "", SoftKeyboardButton::kDelete);

	return newKeyboard;
}

SoftKeyboard* buildStandardKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings) {
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, "1", "!", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "2", "@", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "3", "#", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "4", "$", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "5", "%", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "6", "^", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "7", "&", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "8", "*", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "9", "(", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "0", ")", SoftKeyboardButton::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "q", "Q", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "w", "W", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "e", "E", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "r", "R", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "t", "T", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "y", "Y", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "u", "U", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "i", "I", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "o", "O", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "p", "P", SoftKeyboardButton::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "a", "A", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "s", "S", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "d", "D", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "f", "F", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "g", "G", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "h", "H", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "j", "J", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "k", "K", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "l", "L", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "'", "\"", SoftKeyboardButton::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "shift", "SHIFT", SoftKeyboardButton::kShift);
	makeAButton(engine, newKeyboard, xp, yp, "z", "Z", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "x", "X", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "c", "C", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "v", "V", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "b", "B", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "n", "N", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "m", "M", SoftKeyboardButton::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, "enter", "ENTER", SoftKeyboardButton::kEnter);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, "-", "_", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, ",", "<", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, ".", ">", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "/", "?", SoftKeyboardButton::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, "space", "SPACE", SoftKeyboardButton::kSpace);
	makeAButton(engine, newKeyboard, xp, yp, "", "", SoftKeyboardButton::kDelete);

	return newKeyboard;
}

void makeAButton(ds::ui::SpriteEngine& engine, SoftKeyboard* newKeyboard, float& xp,float& yp, const std::string& character, const std::string& characterHigh, const SoftKeyboardButton::KeyType keyType){
	SoftKeyboardButton* kb = new SoftKeyboardButton(engine, character, characterHigh, keyType, newKeyboard->getSoftKeyboardSettings());
	kb->setClickFn([newKeyboard, kb](){newKeyboard->handleKeyPress(kb); });
	kb->setPosition(xp + kb->getScaleWidth() / 2.0f, yp + kb->getScaleHeight() / 2.0f);
	xp += kb->getWidth();
	newKeyboard->addChild(*kb);
	newKeyboard->getButtonVector().push_back(kb);

}

}
}
}