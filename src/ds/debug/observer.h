#ifndef _DS_OBSERVER_H_
#define _DS_OBSERVER_H_

#include "AntTweakBar.h"
#include "ds\util\bit_mask.h"

namespace ds {

	extern const ds::BitMask	OBSERVER_LOG;

	class Observer {
	public:
		Observer();
		Observer(std::string settingsFileName, bool liveMode = false);
		~Observer();
	private:
		void				mIntialize(std::string aSettingsFileName, bool aLiveMode);
	public:
		// Initializes the Observer.
		// Sprites (or classes) can override this
		// to add more variables to the Tweaker GUI
		// via Observer::observe API
		virtual void		installObserver();

		// Should I be writing to settings files on variable change?
		void				observeLive(bool mode = true);
		// Are we in live mode?
		bool				observingLive() const;

	private:
		// Pointer to an AntTweakBar Object
		TwBar*				mTweaker;
		std::string			mSettingsFileName;
		bool				mLiveMode;

	public:
		// You can use this to live-route changes of a Tweaker GUI to a settings file
		// NOTE: make sure you add your observers with the exact name associated to them in the
		//       settings file. For example if the position is being tweaked and in the settings
		//       file you have it like "ci:lm:name:pos:x", name of your observer must be that
		//       as well:
		//               observerSetSettings("layout.xml");
		//               Observer::observe("ci:lm:name:pos:x", &mNamePosX);
		void				observerSetSettings(std::string name);
		std::string			observerGetSettings() const;

	public:
		// Retrieves the original Ant Tweak Bar pointer.
		// You can use this to work with AntTweakBar API inside your sprite
		// **WARNING** If you add variables through the pointer, they
		// won't be enabled in live mode. use Observer::observe API instead.
		TwBar*				getObserver();
	
	protected:
		// Supplies observer with a unique hash as their name
		// Classes should override this to supply unique names
		// for their Observers. For example Sprites override
		// this and return their mId.
		virtual std::string	observerHashGenerator() const;

	public:
		// This can be extended to support more observe-able types.
		template<typename T>
		void				observe(std::string name, T* var, const std::string def = "") {
			DS_LOG_WARNING_M("The typename you are trying to observe does not have a predefined observer."
				<< std::endl
				<< "passed name is: " << name
				<< std::endl
				<< "C++ type id is: " << typeid(T).name()
				<< std::endl
				<< "You can use Sprite::getPointer() and AntTweakBar's API to register observers for this type."
				<< std::endl,
				ds::OBSERVER_LOG);
		};

	}; //!class Observer

} //!namespace ds

#endif //!_DS_OBSERVER_H_