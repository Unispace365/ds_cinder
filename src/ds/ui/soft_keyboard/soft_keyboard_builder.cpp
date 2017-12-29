#include "stdafx.h"

#include "soft_keyboard_builder.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "ds/ui/soft_keyboard/soft_keyboard.h"

namespace ds {
namespace ui {
namespace SoftKeyboardBuilder {

SoftKeyboard* buildLowercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent) {
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, L"1", L"!", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"@", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"#", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"4", L"$", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"%", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"^", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"7", L"&", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"*", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"(", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"0", L")", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"q", L"Q", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"w", L"W", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"e", L"E", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"r", L"R", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"t", L"T", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"y", L"Y", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"u", L"U", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"i", L"I", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"o", L"O", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"p", L"P", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"a", L"A", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"s", L"S", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"d", L"D", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"f", L"F", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"g", L"G", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"h", L"H", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"j", L"J", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"k", L"K", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"l", L"L", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth()* 3.0f / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"z", L"Z", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"x", L"X", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"c", L"C", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"v", L"V", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"b", L"B", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"n", L"N", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"m", L"M", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"-", L"_", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L",", L"<", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L".", L">", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"/", L"?", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"space", L"SPACE", SoftKeyboardDefs::kSpace);
	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	newKeyboard->setSize(xp, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

SoftKeyboard* buildUppercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent, bool numbers) {
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	if (numbers){
		makeAButton(engine, newKeyboard, xp, yp, L"1", L"!", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"2", L"@", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"3", L"#", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"4", L"$", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"5", L"%", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"6", L"^", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"7", L"&", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"8", L"*", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"9", L"(", SoftKeyboardDefs::kNumber);
		makeAButton(engine, newKeyboard, xp, yp, L"0", L")", SoftKeyboardDefs::kNumber);

		xp = settings.mKeyInitialPosition.x;
		yp += newKeyboard->getButtonVector().back()->getHeight();
	}


	makeAButton(engine, newKeyboard, xp, yp, L"Q", L"Q", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"W", L"W", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"E", L"E", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"R", L"R", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"T", L"T", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"Y", L"Y", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"U", L"U", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"I", L"I", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"O", L"O", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"P", L"P", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"A", L"A", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"S", L"S", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"D", L"D", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"F", L"F", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"G", L"G", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"H", L"H", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"J", L"J", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"K", L"K", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"L", L"L", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth()* 3.0f / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"Z", L"Z", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"X", L"X", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"C", L"C", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"V", L"V", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"B", L"B", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"N", L"N", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"M", L"M", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth()* 3.0f / 2.0f + 10.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"space", L"SPACE", SoftKeyboardDefs::kSpace);
	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	newKeyboard->setSize(xp, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if (parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

SoftKeyboard* buildStandardKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent) {
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, L"1", L"!", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"@", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"#", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"4", L"$", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"%", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"^", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"7", L"&", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"*", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"(", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"0", L")", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"q", L"Q", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"w", L"W", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"e", L"E", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"r", L"R", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"t", L"T", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"y", L"Y", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"u", L"U", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"i", L"I", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"o", L"O", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"p", L"P", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"a", L"A", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"s", L"S", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"d", L"D", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"f", L"F", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"g", L"G", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"h", L"H", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"j", L"J", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"k", L"K", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"l", L"L", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"'", L"\"", SoftKeyboardDefs::kLetter);

	xp = settings.mKeyInitialPosition.x - newKeyboard->getButtonVector().back()->getWidth() / 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"shift", L"SHIFT", SoftKeyboardDefs::kShift);
	makeAButton(engine, newKeyboard, xp, yp, L"z", L"Z", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"x", L"X", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"c", L"C", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"v", L"V", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"b", L"B", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"n", L"N", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"m", L"M", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"-", L"_", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L",", L"<", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L".", L">", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"/", L"?", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"space", L"SPACE", SoftKeyboardDefs::kSpace);
	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	newKeyboard->setSize(xp, yp);
	newKeyboard->setScale(settings.mKeyScale);
	
	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

ds::ui::SoftKeyboard* buildExtendedKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent /*= nullptr*/){

	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	float maxW = xp;

	makeAButton(engine, newKeyboard, xp, yp, L"`", L"~", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"1", L"!", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"@", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"#", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"4", L"$", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"%", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"^", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"7", L"&", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"*", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"(", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"0", L")", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"-", L"_", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"=", L"+", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"tab", L"TAB", SoftKeyboardDefs::kTab);
	makeAButton(engine, newKeyboard, xp, yp, L"q", L"Q", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"w", L"W", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"e", L"E", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"r", L"R", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"t", L"T", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"y", L"Y", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"u", L"U", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"i", L"I", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"o", L"O", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"p", L"P", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"[", L"{", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"]", L"}", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"\\", L"|", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"@", L"@", SoftKeyboardDefs::kLetter);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;// +newKeyboard->getButtonVector().back()->getWidth() * 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);
	makeAButton(engine, newKeyboard, xp, yp, L"a", L"A", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"s", L"S", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"d", L"D", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"f", L"F", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"g", L"G", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"h", L"H", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"j", L"J", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"k", L"K", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"l", L"L", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L";", L":", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"'", L"\"", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"shift", L"SHIFT", SoftKeyboardDefs::kShift);
	makeAButton(engine, newKeyboard, xp, yp, L"z", L"Z", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"x", L"X", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"c", L"C", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"v", L"V", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"b", L"B", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"n", L"N", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"m", L"M", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L",", L"<", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L".", L">", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"/", L"?", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"shift", L"SHIFT", SoftKeyboardDefs::kShift);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	auto spaceButton = makeAButton(engine, newKeyboard, xp, yp, L"space", L"SPACE", SoftKeyboardDefs::kSpace);
	if(spaceButton){
		spaceButton->setPosition(maxW / 2.0f, spaceButton->getPosition().y);
		xp = maxW /2.0f + spaceButton->getWidth() /2.0f;
	}

	if(settings.mEmailMode){
		makeAButton(engine, newKeyboard, xp, yp, L"@", L"@", SoftKeyboardDefs::kLetter);
		makeAButton(engine, newKeyboard, xp, yp, L".com", L".net", SoftKeyboardDefs::kDotCom);
	}

	// TODO: need a way to make the .com key size bigger than a letter
	//xp = maxW / 2.0f + spaceButton->getWidth()/2.0f;
	//makeAButton(engine, newKeyboard, xp, yp, L".com", L".org", SoftKeyboardDefs::kLetter);

	yp += newKeyboard->getButtonVector().back()->getHeight();

	newKeyboard->setSize(maxW, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

ds::ui::SoftKeyboard* buildFullKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent /*= nullptr*/){


	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	float maxW = xp;

	makeAButton(engine, newKeyboard, xp, yp, L"esc", L"ESC", SoftKeyboardDefs::kEscape);
	makeAButton(engine, newKeyboard, xp, yp, L"F1", L"F1", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F2", L"F2", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F3", L"F3", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F4", L"F4", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F5", L"F5", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F6", L"F6", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F7", L"F7", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F8", L"F8", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F9", L"F9", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F10", L"F10", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F11", L"F11", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"F12", L"F12", SoftKeyboardDefs::kFunction);
	makeAButton(engine, newKeyboard, xp, yp, L"del", L"del", SoftKeyboardDefs::kFwdDelete);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();


	makeAButton(engine, newKeyboard, xp, yp, L"`", L"~", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"1", L"!", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"@", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"#", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"4", L"$", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"%", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"^", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"7", L"&", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"*", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"(", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"0", L")", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"-", L"_", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"=", L"+", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"tab", L"TAB", SoftKeyboardDefs::kTab);
	makeAButton(engine, newKeyboard, xp, yp, L"q", L"Q", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"w", L"W", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"e", L"E", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"r", L"R", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"t", L"T", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"y", L"Y", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"u", L"U", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"i", L"I", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"o", L"O", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"p", L"P", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"[", L"{", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"]", L"}", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"\\", L"|", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"@", L"@", SoftKeyboardDefs::kLetter);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;// +newKeyboard->getButtonVector().back()->getWidth() * 2.0f;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);
	makeAButton(engine, newKeyboard, xp, yp, L"a", L"A", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"s", L"S", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"d", L"D", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"f", L"F", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"g", L"G", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"h", L"H", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"j", L"J", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"k", L"K", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"l", L"L", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L";", L":", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"'", L"\"", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"shift", L"SHIFT", SoftKeyboardDefs::kShift);
	makeAButton(engine, newKeyboard, xp, yp, L"z", L"Z", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"x", L"X", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"c", L"C", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"v", L"V", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"b", L"B", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"n", L"N", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"m", L"M", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L",", L"<", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L".", L">", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"/", L"?", SoftKeyboardDefs::kLetter);
	makeAButton(engine, newKeyboard, xp, yp, L"shift", L"SHIFT", SoftKeyboardDefs::kShift);

	if(maxW < xp) maxW = xp;

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"Home", L"Home", SoftKeyboardDefs::kSpecial);
	makeAButton(engine, newKeyboard, xp, yp, L"End", L"End", SoftKeyboardDefs::kSpecial);
	makeAButton(engine, newKeyboard, xp, yp, L"PgUp", L"PgUp", SoftKeyboardDefs::kSpecial);
	makeAButton(engine, newKeyboard, xp, yp, L"PgDn", L"PgDn", SoftKeyboardDefs::kSpecial);

	auto spaceButton = makeAButton(engine, newKeyboard, xp, yp, L"space", L"SPACE", SoftKeyboardDefs::kSpace);

	makeAButton(engine, newKeyboard, xp, yp, L"<", L"<", SoftKeyboardDefs::kArrow);
	makeAButton(engine, newKeyboard, xp, yp, L"^", L"^", SoftKeyboardDefs::kArrow);
	makeAButton(engine, newKeyboard, xp, yp, L">", L">", SoftKeyboardDefs::kArrow);
	makeAButton(engine, newKeyboard, xp, yp, L"v", L"v", SoftKeyboardDefs::kArrow);
	//makeAButton(engine, newKeyboard, xp, yp, L"clear", L"clear", SoftKeyboardDefs::kSpecial);


	yp += newKeyboard->getButtonVector().back()->getHeight();

	newKeyboard->setSize(maxW, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

SoftKeyboard* buildPinPadKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent){
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, L"1", L"1", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"2", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"3", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"4", L"4", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"5", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"6", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"7", L"7", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"8", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"9", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth();
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"0", L"0", SoftKeyboardDefs::kNumber);

	yp += newKeyboard->getButtonVector().back()->getHeight();
	xp = settings.mKeyInitialPosition.x;
	float numberButtonWidht = newKeyboard->getButtonVector().back()->getWidth();
	
	makeAButton(engine, newKeyboard, xp, yp, L"enter", L"ENTER", SoftKeyboardDefs::kEnter);

	auto enterKey = newKeyboard->getButtonVector().back();
	enterKey->setPosition(settings.mKeyInitialPosition.x + numberButtonWidht * 1.5f, enterKey->getPosition().y);
	xp = numberButtonWidht * 3.0f;
	newKeyboard->setSize(xp, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

ds::ui::SoftKeyboard* buildPinCodeKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent /*= nullptr*/){
	SoftKeyboard* newKeyboard = new SoftKeyboard(engine, settings);

	float xp = settings.mKeyInitialPosition.x;
	float yp = settings.mKeyInitialPosition.y;

	makeAButton(engine, newKeyboard, xp, yp, L"1", L"1", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"2", L"2", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"3", L"3", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"4", L"4", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"5", L"5", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"6", L"6", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x;
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"7", L"7", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"8", L"8", SoftKeyboardDefs::kNumber);
	makeAButton(engine, newKeyboard, xp, yp, L"9", L"9", SoftKeyboardDefs::kNumber);

	xp = settings.mKeyInitialPosition.x + newKeyboard->getButtonVector().back()->getWidth();
	yp += newKeyboard->getButtonVector().back()->getHeight();

	makeAButton(engine, newKeyboard, xp, yp, L"0", L"0", SoftKeyboardDefs::kNumber);

	yp += newKeyboard->getButtonVector().back()->getHeight();
	xp = settings.mKeyInitialPosition.x;
	float numberButtonWidht = newKeyboard->getButtonVector().back()->getWidth();

	makeAButton(engine, newKeyboard, xp, yp, L"", L"", SoftKeyboardDefs::kDelete);

	auto enterKey = newKeyboard->getButtonVector().back();
	enterKey->setPosition(settings.mKeyInitialPosition.x + numberButtonWidht * 1.5f, enterKey->getPosition().y);
	xp = numberButtonWidht * 3.0f;
	newKeyboard->setSize(xp, yp);
	newKeyboard->setScale(settings.mKeyScale);

	if(parent){
		parent->addChildPtr(newKeyboard);
	}

	return newKeyboard;
}

SoftKeyboardButton* makeAButton(ds::ui::SpriteEngine& engine, SoftKeyboard* newKeyboard, float& xp, float& yp, const std::wstring& character, const std::wstring& characterHigh, const SoftKeyboardDefs::KeyType keyType){
	SoftKeyboardButton* kb = new SoftKeyboardButton(engine, character, characterHigh, keyType, newKeyboard->getSoftKeyboardSettings());
	kb->setClickFn([newKeyboard, kb](){newKeyboard->handleKeyPress(kb); });
	kb->setPosition(xp + kb->getScaleWidth() / 2.0f, yp + kb->getScaleHeight() / 2.0f);
	xp += kb->getWidth();
	newKeyboard->addChild(*kb);
	newKeyboard->getButtonVector().push_back(kb);

	return kb;

}

}
}
}