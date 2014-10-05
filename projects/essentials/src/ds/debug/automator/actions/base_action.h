#pragma once
#ifndef ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_BASE_ACTION
#define ESSENTIALS_DS_DEBUG_AUTOMATOR_ACTIONS_BASE_ACTION

#include <vector>
#include <cinder/Rect.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds{
namespace debug{
class BaseAction;

/**
 * \class ds::debug::BaseActionFactory
 */
class BaseActionFactory {
public:
	BaseActionFactory();
	virtual ~BaseActionFactory();

	virtual float			getLimit() const = 0;
	virtual int				getNumberOfFingers() const = 0;
	virtual BaseAction*		build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const = 0;
};

/**
 * \class ds::debug::BaseAction
 */
class BaseAction {
public:
	BaseAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame);
	virtual ~BaseAction();

	virtual void			setup(float limit, int numberOfFingers);
	virtual bool			update(float dt);
	void					release();

protected:
	float					mLimit;
	float					mTotal;
	int						mNumberOfFingers;
	std::vector<int>		mInUseList;
	std::vector<int>&		mFreeList;
	ds::ui::SpriteEngine&	mEngine;
	const ci::Rectf			mFrame;
};

} // namespace debug
} // namespace ds

#endif//BASE_ACTION_DS_H
