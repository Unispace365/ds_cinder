#pragma once
#ifndef DS_DEBUG_KEY_MANAGER
#define DS_DEBUG_KEY_MANAGER

#include <functional>
#include <vector>
#include <cinder/app/KeyEvent.h>

namespace ds {
namespace keys {

/**
* \class KeyManager
* \brief Register keys with callbacks and store simple info about em
*/
class KeyManager {
public:
	class KeyRegister {
	public:
		KeyRegister(const std::string& name, std::function<void()> func, const int keyCode, const bool shiftDown, const bool ctrlDown, const bool altDown)
			: mName(name), mCallback(func), mKeyCode(keyCode), mShiftDown(shiftDown), mCtrlDown(ctrlDown), mAltDown(altDown) {}

		std::string mName;
		std::function<void()> mCallback;
		int mKeyCode;
		bool mShiftDown;
		bool mCtrlDown;
		bool mAltDown;
	};

	KeyManager();

	void registerKey(const std::string& name, std::function<void()> func, const int keyCode, const bool shiftDown = false, const bool ctrlDown = false, const bool altDown = false);
	void registerKey(KeyRegister);

	/// Handle key presses
	/// Returns true if they key was handled, false if nothing happened
	bool keyDown(ci::app::KeyEvent event);
	std::vector<KeyRegister>&	getKeyRegistry() { return mKeyRegisters; }

	std::string keyCodeToString(const int keyCode);

	/// Output all set keys into a string.
	std::string getAllKeysString();

	/// getAllKeysString() -> log info
	void printCurrentKeys();

private:
	std::vector<KeyRegister>	mKeyRegisters;
		
};

} // namespace time
} // namespace ds

#endif 
