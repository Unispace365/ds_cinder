#pragma once
#ifndef ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_DRAGONTION_H_
#define ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_DRAGONTION_H_

#include "base_action.h"
#include <cinder/Vector.h>

namespace ds{
namespace ui{
class SpriteEngine;
}
namespace debug{

/**
 * \class ds::debug::DragActionFactory
 */
class DragActionFactory : public BaseActionFactory {
public:
	DragActionFactory()		{ }

	virtual float			getLimit() const;
	virtual int				getNumberOfFingers() const;

	virtual BaseAction*		build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const;
};

/**
 * \class ds::DragAction
 */
class DragAction : public BaseAction
{
public:
	DragAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame);
	virtual ~DragAction();

	void setup(float limit, int numberOfFingers);

	virtual bool update(float dt);
protected:
	std::vector<ci::Vec2f>	mPreviousTouch;
	std::vector<ci::Vec2f>	mTouchPos;
	float					mMagnitude;
	ci::Vec2f				mDirection;
	float					mUpdateTime;
	float					mUpdateTimeTotal;
};

} // namespace debug
} // namespace ds

#endif//DRAG_ACTION_DS_H
