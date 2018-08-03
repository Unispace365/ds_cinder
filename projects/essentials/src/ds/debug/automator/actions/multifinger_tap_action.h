#pragma once
#ifndef MULTI_TAP_ACTION_DS_H
#define MULTI_TAP_ACTION_DS_H

#include "base_action.h"
#include <cinder/Vector.h>

namespace ds{
namespace ui{
class SpriteEngine;
}
namespace debug{

/**
 * \class MultiTapActionFactory
 */
class MultiTapActionFactory : public BaseActionFactory {
public:
	MultiTapActionFactory()	{ }

	virtual float			getLimit() const;
	virtual int				getNumberOfFingers() const;

	virtual BaseAction*		build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const;
};

/**
 * \class MultiTapAction
 */
class MultiTapAction : public BaseAction
{
public:
	MultiTapAction(std::vector<int> &freeList, ds::ui::SpriteEngine&engine, const ci::Rectf& frame);
	virtual ~MultiTapAction();

	void setup(float limit, int numberOfFingers);

	virtual bool update(float dt);
protected:
	std::vector<ci::vec2> mTouchPos;
};
}
}

#endif//MULTI_TAP_ACTION_DS_H
