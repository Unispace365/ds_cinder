#include "ds/ui/soft_keyboard/soft_keyboard_defs.h"

#include <sstream>

void ds::ui::SoftKeyboardDefs::handleKeyPressGeneric(const KeyType& inputKeyType, std::wstring& inOutCurrentKey, std::wstring& inOutFullString){
	if(inputKeyType == kDelete){
		if(inOutFullString.length() > 0){
			inOutFullString = inOutFullString.substr(0, inOutFullString.length() - 1);
		}
		inOutCurrentKey = L"";

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

	} else {
		std::wstringstream ss;
		ss << inOutFullString << inOutCurrentKey;
		inOutFullString = ss.str();

	}
}

