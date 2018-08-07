#include "stdafx.h"

#include "base_action.h"

namespace ds {
namespace debug {

/**
 * \class BaseActionFactory
 */
BaseActionFactory::BaseActionFactory(){
}

BaseActionFactory::~BaseActionFactory(){
}

/**
 * \class BaseAction
 */
BaseAction::BaseAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame)
	: mFreeList(freeList)
	, mEngine(engine)
	, mFrame(frame)
	, mTotal(0.0f)
{

}

BaseAction::~BaseAction(){
	release();
}

bool BaseAction::update(float dt){
	mTotal += dt;

	return false;
}

void BaseAction::setup(float limit, int numberOfFingers){
	mLimit = limit;
	mNumberOfFingers = numberOfFingers;

	release();

	if((int)mFreeList.size() < mNumberOfFingers)
		mNumberOfFingers = static_cast<int>(mFreeList.size());

	for(int i = 0; i < mNumberOfFingers; ++i)
	{
		mInUseList.push_back(mFreeList.back());
		mFreeList.pop_back();
	}
}

void BaseAction::release(){

	if(!mInUseList.empty())	{
		for(auto it = mInUseList.begin(), it2 = mInUseList.end(); it != it2; ++it){
			mFreeList.push_back(*it);
		}
		mInUseList.clear();
	}
}

} // namespace debug
} // namespace ds 
