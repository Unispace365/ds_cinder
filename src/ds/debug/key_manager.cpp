#include "stdafx.h"

#include "key_manager.h"

#include <ds/debug/logger.h>

namespace ds {
namespace keys {


KeyManager::KeyManager() {}


void KeyManager::registerKey(KeyRegister reg) {
	for (auto it : mKeyRegisters){
		if(it.mKeyCode == reg.mKeyCode
		   && it.mAltDown == reg.mAltDown
		   && it.mCtrlDown == reg.mCtrlDown
		   && it.mShiftDown == reg.mShiftDown) {
			DS_LOG_INFO("Multiple functions registered for key: " << keyCodeToString(it.mKeyCode) << " functions: " << it.mName << " and " << reg.mName);
		}
	}

	mKeyRegisters.emplace_back(reg);
}

void KeyManager::registerKey(const std::string& name, std::function<void()> func, const int keyCode, const bool shiftDown /*= false*/, const bool ctrlDown /*= false*/, const bool altDown /*= false*/) {
	registerKey(KeyRegister(name, func, keyCode, shiftDown, ctrlDown, altDown));
}

bool KeyManager::keyDown(ci::app::KeyEvent event) {
	bool handled = false;
	for (auto it : mKeyRegisters){
		if(it.mKeyCode == event.getCode()
		   && it.mAltDown == event.isAltDown()
		   && it.mCtrlDown == event.isControlDown()
		   && it.mShiftDown == event.isShiftDown()
		   && it.mCallback
		   ) {
			it.mCallback();
			handled = true;
		}
	}

	return handled;
}

std::string KeyManager::getAllKeysString() {
	std::stringstream ss;
	ss << "Available keys:" << std::endl;
	for(auto it : mKeyRegisters) {

		if(it.mAltDown) 	ss << "alt  ";
		ss << "\t";

		if(it.mShiftDown)	ss << "shift ";
		ss << "\t";

		if(it.mCtrlDown) 	ss << "ctrl ";
		ss << "\t";

		ss << keyCodeToString(it.mKeyCode) << "\t" << it.mName << std::endl;
	}
	return ss.str();
}


void KeyManager::printCurrentKeys() {
	DS_LOG_INFO(getAllKeysString());
}

std::string KeyManager::keyCodeToString(const int keyCode) {
	using ci::app::KeyEvent;

	switch(keyCode) {
		case KeyEvent::KEY_UNKNOWN: return "unknown"; 
		case KeyEvent::KEY_BACKSPACE: return "backspace";
		case KeyEvent::KEY_TAB: return "tab";
		case KeyEvent::KEY_CLEAR: return "clear";
		case KeyEvent::KEY_RETURN: return "return";
		case KeyEvent::KEY_PAUSE: return "pause";
		case KeyEvent::KEY_ESCAPE: return "esc";
		case KeyEvent::KEY_SPACE: return "space";
		case KeyEvent::KEY_EXCLAIM: return "!";
		case KeyEvent::KEY_QUOTEDBL: return "\"";
		case KeyEvent::KEY_HASH: return "#";
		case KeyEvent::KEY_DOLLAR: return "$";
		case KeyEvent::KEY_AMPERSAND: return "&";
		case KeyEvent::KEY_QUOTE: return "'";
		case KeyEvent::KEY_LEFTPAREN: return "(";
		case KeyEvent::KEY_RIGHTPAREN: return ")";
		case KeyEvent::KEY_ASTERISK: return "*";
		case KeyEvent::KEY_PLUS: return "+";
		case KeyEvent::KEY_COMMA: return ",";
		case KeyEvent::KEY_MINUS: return "-";
		case KeyEvent::KEY_PERIOD: return ".";
		case KeyEvent::KEY_SLASH: return "/";
		case KeyEvent::KEY_0: return "0";
		case KeyEvent::KEY_1: return "1";
		case KeyEvent::KEY_2: return "2";
		case KeyEvent::KEY_3: return "3";
		case KeyEvent::KEY_4: return "4";
		case KeyEvent::KEY_5: return "5";
		case KeyEvent::KEY_6: return "6";
		case KeyEvent::KEY_7: return "7";
		case KeyEvent::KEY_8: return "8";
		case KeyEvent::KEY_9: return "9";
		case KeyEvent::KEY_COLON: return ":";
		case KeyEvent::KEY_SEMICOLON: return ";";
		case KeyEvent::KEY_LESS: return "<";
		case KeyEvent::KEY_EQUALS: return "=";
		case KeyEvent::KEY_GREATER: return ">";
		case KeyEvent::KEY_QUESTION: return "?";
		case KeyEvent::KEY_AT: return "@";

		case KeyEvent::KEY_LEFTBRACKET: return "[";
		case KeyEvent::KEY_BACKSLASH: return "\\";
		case KeyEvent::KEY_RIGHTBRACKET: return "]";
		case KeyEvent::KEY_CARET: return "|";
		case KeyEvent::KEY_UNDERSCORE: return "_";
		case KeyEvent::KEY_BACKQUOTE: return "`";
		case KeyEvent::KEY_a: return "a";
		case KeyEvent::KEY_b: return "b";
		case KeyEvent::KEY_c: return "c";
		case KeyEvent::KEY_d: return "d";
		case KeyEvent::KEY_e: return "e";
		case KeyEvent::KEY_f: return "f";
		case KeyEvent::KEY_g: return "g";
		case KeyEvent::KEY_h: return "h";
		case KeyEvent::KEY_i: return "i";
		case KeyEvent::KEY_j: return "j";
		case KeyEvent::KEY_k: return "k";
		case KeyEvent::KEY_l: return "l";
		case KeyEvent::KEY_m: return "m";
		case KeyEvent::KEY_n: return "n";
		case KeyEvent::KEY_o: return "o";
		case KeyEvent::KEY_p: return "p";
		case KeyEvent::KEY_q: return "q";
		case KeyEvent::KEY_r: return "r";
		case KeyEvent::KEY_s: return "s";
		case KeyEvent::KEY_t: return "t";
		case KeyEvent::KEY_u: return "u";
		case KeyEvent::KEY_v: return "v";
		case KeyEvent::KEY_w: return "w";
		case KeyEvent::KEY_x: return "x";
		case KeyEvent::KEY_y: return "y";
		case KeyEvent::KEY_z: return "z";
		case KeyEvent::KEY_DELETE: return "delete";

		case KeyEvent::KEY_KP0: return "keypad 0";
		case KeyEvent::KEY_KP1: return "keypad 1";
		case KeyEvent::KEY_KP2: return "keypad 2";
		case KeyEvent::KEY_KP3: return "keypad 3";
		case KeyEvent::KEY_KP4: return "keypad 4";
		case KeyEvent::KEY_KP5: return "keypad 5";
		case KeyEvent::KEY_KP6: return "keypad 6";
		case KeyEvent::KEY_KP7: return "keypad 7";
		case KeyEvent::KEY_KP8: return "keypad 8";
		case KeyEvent::KEY_KP9: return "keypad 9";
		case KeyEvent::KEY_KP_PERIOD: return "keypad .";
		case KeyEvent::KEY_KP_DIVIDE: return "keypad /";
		case KeyEvent::KEY_KP_MULTIPLY: return "keypad *";
		case KeyEvent::KEY_KP_MINUS: return "keypad -";
		case KeyEvent::KEY_KP_PLUS: return "keypad +";
		case KeyEvent::KEY_KP_ENTER: return "keypad enter";
		case KeyEvent::KEY_KP_EQUALS: return "keypad ";

		case KeyEvent::KEY_UP: return "up";
		case KeyEvent::KEY_DOWN: return "down";
		case KeyEvent::KEY_RIGHT: return "right";
		case KeyEvent::KEY_LEFT: return "left";
		case KeyEvent::KEY_INSERT: return "ins";
		case KeyEvent::KEY_HOME: return "home";
		case KeyEvent::KEY_END: return "end";
		case KeyEvent::KEY_PAGEUP: return "pgup";
		case KeyEvent::KEY_PAGEDOWN: return "pgdwn";

		case KeyEvent::KEY_F1: return "F1";
		case KeyEvent::KEY_F2: return "F2";
		case KeyEvent::KEY_F3: return "F3";
		case KeyEvent::KEY_F4: return "F4";
		case KeyEvent::KEY_F5: return "F5";
		case KeyEvent::KEY_F6: return "F6";
		case KeyEvent::KEY_F7: return "F7";
		case KeyEvent::KEY_F8: return "F8";
		case KeyEvent::KEY_F9: return "F9";
		case KeyEvent::KEY_F10: return "F10";
		case KeyEvent::KEY_F11: return "F11";
		case KeyEvent::KEY_F12: return "F12";
		case KeyEvent::KEY_F13: return "F13";
		case KeyEvent::KEY_F14: return "F14";
		case KeyEvent::KEY_F15: return "F15";

		case KeyEvent::KEY_NUMLOCK: return "numlock";
		case KeyEvent::KEY_CAPSLOCK: return "capslock";
		case KeyEvent::KEY_SCROLLOCK: return "scrllock";
		case KeyEvent::KEY_RSHIFT: return "rshift";
		case KeyEvent::KEY_LSHIFT: return "lshift";
		case KeyEvent::KEY_RCTRL: return "rctrl";
		case KeyEvent::KEY_LCTRL: return "lctrl";
		case KeyEvent::KEY_RALT: return "ralt";
		case KeyEvent::KEY_LALT: return "lalt";
		case KeyEvent::KEY_RMETA: return "rmeta";
		case KeyEvent::KEY_LMETA: return "lmeta";
		case KeyEvent::KEY_LSUPER: return "lsuper windows";		/* Left "Windows" case KeyEvent::KEY */
		case KeyEvent::KEY_RSUPER: return "rsuper windows";		/* Right "Windows" case KeyEvent::KEY */
		case KeyEvent::KEY_MODE: return "mode";		/* "Alt Gr" case KeyEvent::KEY */
		case KeyEvent::KEY_COMPOSE: return "compose";		/* Multi-case KeyEvent::KEY compose case KeyEvent::KEY */

		case KeyEvent::KEY_HELP: return "help";
		case KeyEvent::KEY_PRINT: return "print";
		case KeyEvent::KEY_SYSREQ: return "sysreq";
		case KeyEvent::KEY_BREAK: return "break";
		case KeyEvent::KEY_MENU: return "menu";
		case KeyEvent::KEY_POWER: return "power";		/* Power Macintosh power case KeyEvent::KEY */
		case KeyEvent::KEY_EURO: return "euro";		/* Some european case KeyEvent::KEYboards */
		case KeyEvent::KEY_UNDO: return "undo";		/* Atari case KeyEvent::KEYboard has Undo */
	default:
		break;

	}
	return "Unknown";
}

int KeyManager::stringToKeyCode(const std::string& keyname)
{
	if (mKeyCodeMap.find(keyname) == mKeyCodeMap.end()) {
		return 0;
	}
	return mKeyCodeMap[keyname];
}

std::unordered_map<std::string, int> KeyManager::mKeyCodeMap = { {"unknown",ci::app::KeyEvent::KEY_UNKNOWN},
        {"backspace",ci::app::KeyEvent::KEY_BACKSPACE},
        {"tab",ci::app::KeyEvent::KEY_TAB},
        {"clear",ci::app::KeyEvent::KEY_CLEAR},
        {"return",ci::app::KeyEvent::KEY_RETURN},
        {"pause",ci::app::KeyEvent::KEY_PAUSE},
        {"esc",ci::app::KeyEvent::KEY_ESCAPE},
        {"space",ci::app::KeyEvent::KEY_SPACE},
        {"!",ci::app::KeyEvent::KEY_EXCLAIM},
        {"\"",ci::app::KeyEvent::KEY_QUOTEDBL},
        {"#",ci::app::KeyEvent::KEY_HASH},
        {"$",ci::app::KeyEvent::KEY_DOLLAR},
        {"&",ci::app::KeyEvent::KEY_AMPERSAND},
        {"'",ci::app::KeyEvent::KEY_QUOTE},
        {"(",ci::app::KeyEvent::KEY_LEFTPAREN},
        {")",ci::app::KeyEvent::KEY_RIGHTPAREN},
        {"*",ci::app::KeyEvent::KEY_ASTERISK},
        {"+",ci::app::KeyEvent::KEY_PLUS},
        {",",ci::app::KeyEvent::KEY_COMMA},
        {"-",ci::app::KeyEvent::KEY_MINUS},
        {".",ci::app::KeyEvent::KEY_PERIOD},
        {"/",ci::app::KeyEvent::KEY_SLASH},
        {"0",ci::app::KeyEvent::KEY_0},
        {"1",ci::app::KeyEvent::KEY_1},
        {"2",ci::app::KeyEvent::KEY_2},
        {"3",ci::app::KeyEvent::KEY_3},
        {"4",ci::app::KeyEvent::KEY_4},
        {"5",ci::app::KeyEvent::KEY_5},
        {"6",ci::app::KeyEvent::KEY_6},
        {"7",ci::app::KeyEvent::KEY_7},
        {"8",ci::app::KeyEvent::KEY_8},
        {"9",ci::app::KeyEvent::KEY_9},
        {":",ci::app::KeyEvent::KEY_COLON},
        {";",ci::app::KeyEvent::KEY_SEMICOLON},
        {"<",ci::app::KeyEvent::KEY_LESS},
        {"=",ci::app::KeyEvent::KEY_EQUALS},
        {">",ci::app::KeyEvent::KEY_GREATER},
        {"?",ci::app::KeyEvent::KEY_QUESTION},
        {"@",ci::app::KeyEvent::KEY_AT},
        {"[",ci::app::KeyEvent::KEY_LEFTBRACKET},
        {"\\",ci::app::KeyEvent::KEY_BACKSLASH},
        {"]",ci::app::KeyEvent::KEY_RIGHTBRACKET},
        {"|",ci::app::KeyEvent::KEY_CARET},
        {"_",ci::app::KeyEvent::KEY_UNDERSCORE},
        {"`",ci::app::KeyEvent::KEY_BACKQUOTE},
        {"a",ci::app::KeyEvent::KEY_a},
        {"b",ci::app::KeyEvent::KEY_b},
        {"c",ci::app::KeyEvent::KEY_c},
        {"d",ci::app::KeyEvent::KEY_d},
        {"e",ci::app::KeyEvent::KEY_e},
        {"f",ci::app::KeyEvent::KEY_f},
        {"g",ci::app::KeyEvent::KEY_g},
        {"h",ci::app::KeyEvent::KEY_h},
        {"i",ci::app::KeyEvent::KEY_i},
        {"j",ci::app::KeyEvent::KEY_j},
        {"k",ci::app::KeyEvent::KEY_k},
        {"l",ci::app::KeyEvent::KEY_l},
        {"m",ci::app::KeyEvent::KEY_m},
        {"n",ci::app::KeyEvent::KEY_n},
        {"o",ci::app::KeyEvent::KEY_o},
        {"p",ci::app::KeyEvent::KEY_p},
        {"q",ci::app::KeyEvent::KEY_q},
        {"r",ci::app::KeyEvent::KEY_r},
        {"s",ci::app::KeyEvent::KEY_s},
        {"t",ci::app::KeyEvent::KEY_t},
        {"u",ci::app::KeyEvent::KEY_u},
        {"v",ci::app::KeyEvent::KEY_v},
        {"w",ci::app::KeyEvent::KEY_w},
        {"x",ci::app::KeyEvent::KEY_x},
        {"y",ci::app::KeyEvent::KEY_y},
        {"z",ci::app::KeyEvent::KEY_z},
        {"delete",ci::app::KeyEvent::KEY_DELETE},
        {"keypad 0",ci::app::KeyEvent::KEY_KP0},
        {"keypad 1",ci::app::KeyEvent::KEY_KP1},
        {"keypad 2",ci::app::KeyEvent::KEY_KP2},
        {"keypad 3",ci::app::KeyEvent::KEY_KP3},
        {"keypad 4",ci::app::KeyEvent::KEY_KP4},
        {"keypad 5",ci::app::KeyEvent::KEY_KP5},
        {"keypad 6",ci::app::KeyEvent::KEY_KP6},
        {"keypad 7",ci::app::KeyEvent::KEY_KP7},
        {"keypad 8",ci::app::KeyEvent::KEY_KP8},
        {"keypad 9",ci::app::KeyEvent::KEY_KP9},
        {"keypad .",ci::app::KeyEvent::KEY_KP_PERIOD},
        {"keypad /",ci::app::KeyEvent::KEY_KP_DIVIDE},
        {"keypad *",ci::app::KeyEvent::KEY_KP_MULTIPLY},
        {"keypad -",ci::app::KeyEvent::KEY_KP_MINUS},
        {"keypad +",ci::app::KeyEvent::KEY_KP_PLUS},
        {"keypad enter",ci::app::KeyEvent::KEY_KP_ENTER},
        {"keypad ",ci::app::KeyEvent::KEY_KP_EQUALS},
        {"up",ci::app::KeyEvent::KEY_UP},
        {"down",ci::app::KeyEvent::KEY_DOWN},
        {"right",ci::app::KeyEvent::KEY_RIGHT},
        {"left",ci::app::KeyEvent::KEY_LEFT},
        {"ins",ci::app::KeyEvent::KEY_INSERT},
        {"home",ci::app::KeyEvent::KEY_HOME},
        {"end",ci::app::KeyEvent::KEY_END},
        {"pgup",ci::app::KeyEvent::KEY_PAGEUP},
        {"pgdwn",ci::app::KeyEvent::KEY_PAGEDOWN},
        {"F1",ci::app::KeyEvent::KEY_F1},
        {"F2",ci::app::KeyEvent::KEY_F2},
        {"F3",ci::app::KeyEvent::KEY_F3},
        {"F4",ci::app::KeyEvent::KEY_F4},
        {"F5",ci::app::KeyEvent::KEY_F5},
        {"F6",ci::app::KeyEvent::KEY_F6},
        {"F7",ci::app::KeyEvent::KEY_F7},
        {"F8",ci::app::KeyEvent::KEY_F8},
        {"F9",ci::app::KeyEvent::KEY_F9},
        {"F10",ci::app::KeyEvent::KEY_F10},
        {"F11",ci::app::KeyEvent::KEY_F11},
        {"F12",ci::app::KeyEvent::KEY_F12},
        {"F13",ci::app::KeyEvent::KEY_F13},
        {"F14",ci::app::KeyEvent::KEY_F14},
        {"F15",ci::app::KeyEvent::KEY_F15},
        {"numlock",ci::app::KeyEvent::KEY_NUMLOCK},
        {"capslock",ci::app::KeyEvent::KEY_CAPSLOCK},
        {"scrllock",ci::app::KeyEvent::KEY_SCROLLOCK},
        {"rshift",ci::app::KeyEvent::KEY_RSHIFT},
        {"lshift",ci::app::KeyEvent::KEY_LSHIFT},
        {"rctrl",ci::app::KeyEvent::KEY_RCTRL},
        {"lctrl",ci::app::KeyEvent::KEY_LCTRL},
        {"ralt",ci::app::KeyEvent::KEY_RALT},
        {"lalt",ci::app::KeyEvent::KEY_LALT},
        {"rmeta",ci::app::KeyEvent::KEY_RMETA},
        {"lmeta",ci::app::KeyEvent::KEY_LMETA},
        {"lsuper windows",ci::app::KeyEvent::KEY_LSUPER},
        {"rsuper windows",ci::app::KeyEvent::KEY_RSUPER},
        {"mode",ci::app::KeyEvent::KEY_MODE},
        {"compose",ci::app::KeyEvent::KEY_COMPOSE},
        {"help",ci::app::KeyEvent::KEY_HELP},
        {"print",ci::app::KeyEvent::KEY_PRINT},
        {"sysreq",ci::app::KeyEvent::KEY_SYSREQ},
        {"break",ci::app::KeyEvent::KEY_BREAK},
        {"menu",ci::app::KeyEvent::KEY_MENU},
        {"power",ci::app::KeyEvent::KEY_POWER},
        {"euro",ci::app::KeyEvent::KEY_EURO},
        { "undo",ci::app::KeyEvent::KEY_UNDO } 
};

}
}