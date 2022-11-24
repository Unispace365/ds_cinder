#pragma once
#ifndef ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_DRAGONTION_H_
#define ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_DRAGONTION_H_

#include "base_action.h"
#include <cinder/Vector.h>

namespace ds {
namespace ui {
	class SpriteEngine;
}
namespace debug {

	/**
	 * \class DragActionFactory
	 */
	class DragActionFactory : public BaseActionFactory {
	  public:
		DragActionFactory() {}

		virtual float getLimit() const;
		virtual int	  getNumberOfFingers() const;

		virtual BaseAction* build(std::vector<int>& freeList, ds::ui::SpriteEngine& engine,
								  const ci::Rectf& frame) const;
	};

	/**
	 * \class DragAction
	 */
	class DragAction : public BaseAction {
	  public:
		DragAction(std::vector<int>& freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame);
		virtual ~DragAction();

		void setup(float limit, int numberOfFingers);

		virtual bool update(float dt);

	  protected:
		std::vector<ci::vec2> mPreviousTouch;
		std::vector<ci::vec2> mTouchPos;
		float				  mMagnitude;
		ci::vec2			  mDirection;
		float				  mUpdateTime;
		float				  mUpdateTimeTotal;
	};

} // namespace debug
} // namespace ds

#endif // DRAG_ACTION_DS_H
