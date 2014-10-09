#include "Automator.h"

#include <cinder/Rand.h>

#include <ds/ui/sprite/sprite_engine.h>

#include "ds/debug/automator/actions/base_action.h"
#include "ds/debug/automator/actions/drag_action.h"
#include "ds/debug/automator/actions/multifinger_tap_action.h"
#include "ds/debug/automator/actions/tap_action.h"

namespace ds {
namespace debug {

Automator::Automator(ds::ui::SpriteEngine& engine, const std::string& watermarkTextConfig)
	: inherited(engine)
	, mActive(false)
	, mWatermark(nullptr)
	, mWatermarkConfig(watermarkTextConfig)
	, mFrame(0.0f, 0.0f, 1.0f, 1.0f)
	, mPeriod(0.016f)
	, mTotal(0.0f)
	, mFingerMax(128)
{		
	setFrame(ci::Rectf(0.0f, 0.0f, mEngine.getWorldWidth(), mEngine.getWorldHeight()));

	addFactory(std::shared_ptr<BaseActionFactory>(new DragActionFactory()));
	addFactory(std::shared_ptr<BaseActionFactory>(new MultiTapActionFactory()));
	addFactory(std::shared_ptr<BaseActionFactory>(new TapActionFactory()));

	clear();
}

void Automator::setFrame(const ci::Rectf& f){
	mFrame = f;
}

void Automator::setPeriod(const float period){
	mPeriod = period;
}

void Automator::addFactory(const std::shared_ptr<BaseActionFactory>& fac){
	if(fac.get() == nullptr) return;

	try {
		mFactory.push_back(Factory());
		mFactory.back().mFactory = fac;
		// Assign everyone to a bucket for random selection.
		const int			size = mFactory.size();
		float				min = 0;
		for(int k = 0; k < size; ++k) {
			Factory&		f = mFactory[k];
			f.mMin = min;
			f.mMax = float(k + 1) / float(size);
			min = f.mMax;
		}
	} catch(std::exception&) {
	}
}

void Automator::addSingletonFactory(const std::shared_ptr<BaseActionFactory>& fact){
	if(fact.get() == nullptr) return;

	Factory factory;
	factory.mFactory = fact;
	std::shared_ptr<BaseAction>		a = factory.addAction(mFreeList, mEngine, mFrame);
	if(a != nullptr) mSingletonList.push_back(a);
}

void Automator::clearFactories(){
	mFactory.clear();
}

void Automator::update(const ds::UpdateParams& up){

	if(!mActive) return;

	// If the client has installed actions, use those, otherwise use the defaults.
	// Probably, the defaults should be actions that are installed at construction,
	// and then removed if the client installs their own.
	if(!mFactory.empty()){
		mActioner.update(up.getDeltaTime());

		mTotal += up.getDeltaTime();

		if(mTotal >= mPeriod){
			mTotal = 0.0f;

			if(!mFreeList.empty()){
				float percent = ci::randFloat(0.0f, 1.0f);
				for(auto it = mFactory.begin(), end = mFactory.end(); it != end; ++it) {
					Factory&	f = *it;
					if(percent >= f.mMin && percent <= f.mMax && f.mFactory.get() != nullptr) {
						std::shared_ptr<BaseAction>		a = f.addAction(mFreeList, mEngine, mFrame);
						if(a != nullptr) mActioner.add(a);
						break;
					}
				}
			}
		}

		for(auto it = mSingletonList.begin(); it < mSingletonList.end(); ++it){
			(*it).get()->update(up.getDeltaTime());
		}
	}
}

void Automator::toggleActive(){
	if(mActive){
		deactivate();
	} else {
		activate();
	}
}

void Automator::activate(){
	if(mActive) return;
	mActive = true;
	std::cout << "Automated mode is on." << std::endl;
	if(mWatermark){
		mWatermark->show();
	} else {
		// TODO
// 		const std::wstring automatedText = L"Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode | Automated Mode";
// 		mWatermark = mEngine.addTextSprite(automatedText, mWatermarkFontResourceId, 72);
// 		mWatermark->disable();
// 		mWatermark->setPosition(72, 0);
// 		mWatermark->setRotation(90.0f);
// 		mWatermark->setColor(Color(0xffffff));
// 		mWatermark->setOpacity(0.25f);
	}

}

void Automator::deactivate(){
	if(!mActive) return;
	mActive = false;

	std::cout << "Automated mode is off." << std::endl;
	if(mWatermark){
		mWatermark->hide();
	}

	clear();
}

void Automator::clear(){	
	mActive = false;
	std::vector<ci::app::TouchEvent::Touch> touches;
	for(int i = 0; i < mFingerMax; ++i){
		touches.push_back(ci::app::TouchEvent::Touch(ci::Vec2f::zero(), ci::Vec2f::zero(), i, 0.0, nullptr));
	}
	mEngine.injectTouchesEnded(ci::app::TouchEvent(mEngine.getWindow(), touches));

	mActioner.clear();
	mFreeList.clear();
	for(int i = 0; i < mFingerMax; ++i)	{
		mFreeList.push_back(i);
	}
}

/**
* \class ds::Automator::Factory
*/
Automator::Factory::Factory()
	: mMin(0)
	, mMax(1)
{
}

std::shared_ptr<BaseAction> Automator::Factory::addAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame)
{
	BaseActionFactory*			factory = mFactory.get();
	if(factory == nullptr) return nullptr;

	for(int i = 0; i < mAction.size(); ++i)	{
		auto ptr = mAction[i];
		if(ptr.unique())		{
			ptr.get()->setup(factory->getLimit(), factory->getNumberOfFingers());
			return ptr;
		}
	}

	mAction.push_back(std::shared_ptr<BaseAction>(factory->build(freeList, engine, frame)));
	mAction.back().get()->setup(factory->getLimit(), factory->getNumberOfFingers());
	return mAction.back();
}

// AUTOMATOR::ACTIONER
Automator::Actioner::Actioner(){
}

Automator::Actioner::~Actioner(){
	clear();
}

void Automator::Actioner::clear(){
	mActions.clear();
}

void Automator::Actioner::add(std::shared_ptr<BaseAction> action){
	mActions.push_back(action);
}

void Automator::Actioner::update(float deltaTime){
	std::list<std::vector<std::unique_ptr<BaseAction>>::iterator> freeList;

	for(int i = 0; i < mActions.size();){
		if(mActions[i].get()){
			if(mActions[i].get()->update(deltaTime)){
				std::swap(mActions[i], mActions[mActions.size() - 1]);
				mActions.back().get()->release();
				mActions.pop_back();
				continue;
			}
		} else {
			std::swap(mActions[i], mActions[mActions.size() - 1]);
			mActions.pop_back();
			continue;
		}

		++i;
	}
}

} // namespace debug
} // namespace ds
