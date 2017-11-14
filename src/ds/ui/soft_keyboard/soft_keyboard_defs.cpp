#include "stdafx.h"

#include "ds/ui/soft_keyboard/soft_keyboard_defs.h"

#include <sstream>

void ds::ui::SoftKeyboardDefs::handleKeyPressGeneric(const KeyType& inputKeyType, std::wstring& inOutCurrentKey, std::wstring& inOutFullString){
	if(inputKeyType == kDelete){
		if(inOutFullString.length() > 0){
			inOutFullString = inOutFullString.substr(0, inOutFullString.length() - 1);
		}
		inOutCurrentKey = L"";

	} else if(inputKeyType == kFwdDelete || inputKeyType == kEscape || inputKeyType == kSpecial || inputKeyType == kFunction || inputKeyType == kArrow){

	} else if(inputKeyType == kShift){
		inOutCurrentKey = L"";

	} else if(inputKeyType == kEnter){
		std::wstringstream ss;
		ss << std::endl;
		inOutCurrentKey = ss.str();
		ss.str(L"");
		ss << inOutFullString << inOutCurrentKey;
		inOutFullString = ss.str();

	} else if(inputKeyType == kSpace){
		inOutCurrentKey = L" ";
		std::wstringstream ss;
		ss << inOutFullString << inOutCurrentKey;
		inOutFullString = ss.str();

	} else if(inputKeyType == kTab){
		inOutCurrentKey = L"	";	
		std::wstringstream ss;
		ss << inOutFullString << inOutCurrentKey;
		inOutFullString = ss.str();

	} else {
		std::wstringstream ss;
		ss << inOutFullString << inOutCurrentKey;
		inOutFullString = ss.str();

	}
}

void ds::ui::SoftKeyboardDefs::handleKeyPressGeneric(const ci::app::KeyEvent& cinderKeyEvent, std::wstring& inOutCurrentKey, std::wstring& inOutFullString){
	using namespace ci::app;
	int keyCode = cinderKeyEvent.getCode();
	if(keyCode == KeyEvent::KEY_SPACE){
		handleKeyPressGeneric(kSpace, inOutCurrentKey, inOutFullString);
	} else if(keyCode == KeyEvent::KEY_DELETE || keyCode == KeyEvent::KEY_BACKSPACE){
		handleKeyPressGeneric(kDelete, inOutCurrentKey, inOutFullString);

	} else if(keyCode == KeyEvent::KEY_TAB){
		handleKeyPressGeneric(kTab, inOutCurrentKey, inOutFullString);

		// a bunch of keys effectively have no effect, so ignore then, otherwise they'll insert a null character into the string
		// check out the KeyEvent header for the values
	} else if(keyCode < KeyEvent::KEY_SPACE || keyCode > KeyEvent::KEY_KP_EQUALS){
		// nothin!
	} else {
		inOutCurrentKey = cinderKeyEvent.getChar();
		handleKeyPressGeneric(kLetter, inOutCurrentKey, inOutFullString);
	}
}

