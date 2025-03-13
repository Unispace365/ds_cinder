#pragma once
#ifndef DS_DEBUG_KEY_MANAGER
#define DS_DEBUG_KEY_MANAGER

#include <functional>
#include <utility>
#include <vector>

#include <cinder/app/KeyEvent.h>

namespace ds { namespace keys {

	/**
	 * \class KeyManager
	 * \brief Register keys with callbacks and store simple info about em
	 */
	class KeyManager {
	  public:
		class KeyRegister {
		  public:
			KeyRegister(const std::string& name, std::function<void()> func, int keyCode, bool shiftDown, bool ctrlDown,
						bool altDown)
			  : mName(name)
			  , mCallback(std::move(func))
			  , mKeyCode(keyCode)
			  , mShiftDown(shiftDown)
			  , mCtrlDown(ctrlDown)
			  , mAltDown(altDown) {}

			std::string			  mName;
			std::function<void()> mCallback;
			int					  mKeyCode;
			bool				  mShiftDown;
			bool				  mCtrlDown;
			bool				  mAltDown;
		};

		KeyManager() = default;

		void registerKey(const std::string& name, std::function<void()> func, int keyCode, bool shiftDown = false,
						 bool ctrlDown = false, bool altDown = false);
		void registerKey(const KeyRegister&);

		/// Handle key presses
		/// Returns true if they key was handled, false if nothing happened
		bool					  keyDown(const ci::app::KeyEvent& event) const;
		std::vector<KeyRegister>& getKeyRegistry() { return mKeyRegisters; }

		static std::string keyCodeToString(int keyCode);
		static int		   stringToKeyCode(const std::string& keyName);

		/// Output all set keys into a string.
		std::string getAllKeysString() const;

		/// getAllKeysString() -> log info
		void printCurrentKeys();

	  private:
		std::vector<KeyRegister>					mKeyRegisters;
		static std::unordered_map<std::string, int> mKeyCodeMap;
	};

}} // namespace ds::keys

#endif
