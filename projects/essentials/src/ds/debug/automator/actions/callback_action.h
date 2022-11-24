#pragma once
#ifndef ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_CALLBACK_ACTION_H_
#define ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_CALLBACK_ACTION_H_

#include "base_action.h"
#include <cinder/Vector.h>

namespace ds {
namespace ui {
	class SpriteEngine;
}
namespace debug {

	/**
	 * \class CallbackActionFactory
	 */
	class CallbackActionFactory : public BaseActionFactory {
	  public:
		CallbackActionFactory(std::function<void(void)> callback, const float minFrequency = 0.0f,
							  const float maxFrequency = 1.0f)
		  : mCallback(callback)
		  , mMinFrequency(minFrequency)
		  , mMaxFrequency(maxFrequency) {}

		virtual float getLimit() const;
		virtual int	  getNumberOfFingers() const;

		virtual BaseAction* build(std::vector<int>& freeList, ds::ui::SpriteEngine& engine,
								  const ci::Rectf& frame) const;

		std::function<void(void)> mCallback;
		float					  mMinFrequency;
		float					  mMaxFrequency;
	};

	/**
	 * \class CallbackAction
	 */
	class CallbackAction : public BaseAction {
	  public:
		CallbackAction(std::vector<int>& freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame,
					   const std::function<void(void)>& callback, const float minTime, const float floatMaxTime);

		void setup(float limit, int numberOfFingers);

		virtual bool update(float dt);

	  protected:
		std::function<void(void)> mCallback;
		float					  mMinTime;
		float					  mMaxTime;
	};

} // namespace debug
} // namespace ds

#endif // DRAG_ACTION_DS_H
