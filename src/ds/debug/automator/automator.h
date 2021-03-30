#pragma once
#ifndef DS_DEBUG_AUTOMATOR
#define DS_DEBUG_AUTOMATOR

#include <vector>
#include <cinder/Vector.h>
#include <cinder/Rect.h>
#include <Poco/Timestamp.h>
#include <ds/app/auto_update.h>
#include <ds/ui/sprite/text.h>
#include "ds/debug/automator/actions/base_action.h"

namespace ds {
namespace ui {
class Sprite;
} // namespace ui

namespace debug {
/**
* \class Automator
*  Creates a bunch of fake touch points to help find bugs in your app.
*  Can also call arbitrary lambdas to load special hard-to-reach parts of your app via the callback action
*
* Use:
*  - Add one automator on your base app class
*  - Call activate when you want it to kick in (perhaps in response to a keypress?)
*  - Register lambda functions to randomly be called via the callback action
*/

class Automator : public ds::AutoUpdate {
public:
	Automator(ds::ui::SpriteEngine& engine, const std::string& textConfigForOverlayText = "");

	// Call activate to start automating
	void						activate();
	// Deactivate turns automation off
	void						deactivate();
	// Toggles activate / deactivate
	void						toggleActive();

	void						setFrame(const ci::Rectf&);
	void						setPeriod(const float period);

	// Supply factories for any actions you would like this automator to perform.
	void						addFactory(const std::shared_ptr<BaseActionFactory>&);
	void						clearFactories();

	// Supply a factory for a single-fire action. That action will repeat endlessly using the range supplied to it
	void						addSingletonFactory(const std::shared_ptr<BaseActionFactory>&);
protected:
	virtual void				update(const ds::UpdateParams&);

private:

	class Factory {
	public:
		Factory();

		// Range used by the randomizer to determine which action to take
		float										mMin, mMax;
		std::shared_ptr<BaseActionFactory>			mFactory;
		std::vector<std::shared_ptr<BaseAction>>	mAction;

		std::shared_ptr<BaseAction>					addAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame);
	};


	class Actioner {
	public:
		Actioner();
		~Actioner();

		void add(std::shared_ptr<BaseAction> action);
		void clear();
		void update(float deltaTime);

	private:
		std::vector<std::shared_ptr<BaseAction>> mActions;
	};
	void						clear();

	typedef ds::AutoUpdate		inherited;

	ci::Rectf					mFrame;
	float						mPeriod;
	float						mTotal;

	ds::ui::Text*				mWatermark;
	std::string					mWatermarkConfig;
	bool						mActive;
	
	// A collection of factory objects for all of my actions.
	std::vector<Factory>		mFactory;

	// A collection of factories that only get called one at a time
	std::vector<std::shared_ptr<BaseAction>>		mSingletonList;

	Actioner					mActioner;
	std::vector<int>			mFreeList;
	int							mFingerMax;
};

} // namespace debug
} // namespace ds

#endif // DS_DEBUG_AUTOMATOR
