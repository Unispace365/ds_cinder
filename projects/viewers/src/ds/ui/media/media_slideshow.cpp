#include "media_slideshow.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "ds/ui/media/media_viewer.h"
#include "ds/ui/media/media_interface.h"


namespace ds {
namespace ui {

MediaSlideshow::MediaSlideshow(ds::ui::SpriteEngine& eng)
	: ds::ui::Sprite(eng)
	, mHolder(nullptr)
	, mCurItemIndex(0)
	, mAnimateDuration(0.35f)
	, mCurrentInterface(nullptr)
{

	mHolder = new ds::ui::Sprite(mEngine);
	addChildPtr(mHolder);
}

void MediaSlideshow::clearSlideshow(){
	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		(*it)->release();
	}

	mViewers.clear();

	mCurItemIndex = 0;
}

void MediaSlideshow::setMediaSlideshow(const std::vector<ds::Resource>& resources){
	clearSlideshow();

	mCurItemIndex = 0;

	for(auto it = resources.begin(); it < resources.end(); ++it){
		MediaViewer* mv = new MediaViewer(mEngine, (*it));
		mv->setDefaultBounds(getWidth(), getHeight());
		mv->setDefaultSize(ci::Vec2f(getWidth(), getHeight()));
		mHolder->addChildPtr(mv);
		mv->setSwipeCallback([this](ds::ui::Sprite* spr, const ci::Vec3f& amount){
			// don't advance if we're zoomed in
			if(spr->getWidth() <= getWidth()){
				if(amount.x > 20.0f){
					gotoPrev(false);
				} else if(amount.x < -20.0f){
					gotoNext(false);
				}
			}
		});
		mv->setAnimateDuration(mAnimateDuration);
		mv->setDoubleTapCallback([this, mv](ds::ui::Sprite* bs, const ci::Vec3f& pos){
			callAfterDelay([this]{recenterSlides(); }, 0.01f);
		});
		mViewers.push_back(mv);
	}

	loadCurrentAndAdjacent();
	layout();
	recenterSlides();
}

void MediaSlideshow::onSizeChanged(){
	layout();
}

// don't change the size of this thing in layout
void MediaSlideshow::layout(){
	float mssWidth = getWidth();
	float mssHeight = getHeight();
	float xp = 0.0f;

	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		ci::Vec2f size = (*it)->getDefaultSize();
		float viewerWidth = size.x;
		float viewerHeight = size.y;

		(*it)->setViewerSize(viewerWidth, viewerHeight);
		(*it)->setPosition(xp, 0.0f);
		(*it)->setOrigin((*it)->getPosition());
		(*it)->setBoundingArea(ci::Rectf(xp, 0.0f, xp + mssWidth, 0.0f + mssHeight));
		xp += mssWidth;
	}

	setCurrentInterface();
}

// This is basically layout, but animates
void MediaSlideshow::recenterSlides(){
	float xp = 0.0f;
	const float w = getWidth();
	const float h = getHeight();
	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		(*it)->animateToDefaultSize();
		(*it)->tweenPosition(ci::Vec3f((*it)->getOrigin().x + (w - (*it)->getDefaultSize().x) * 0.5f, (h - (*it)->getDefaultSize().y) * 0.5f, 0.0f), mAnimateDuration, 0.0f, ci::EaseInOutQuad());
		xp += w;
	}
}


void MediaSlideshow::stopAllContent(){
	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		(*it)->stopContent();
	}
}

void MediaSlideshow::gotoItemIndex(const int newIndex){
	if(!mHolder || mViewers.empty()) return;

	if(mCurItemIndex > -1 && mCurItemIndex < mViewers.size() && mCurItemIndex != newIndex){
		// if the current view is in a drag, shut that down
		MediaViewer &currentView = *mViewers[mCurItemIndex];
		currentView.enable(false);

		// fix the position if the center is oddly set
		ci::Vec3f positionDelta = currentView.getCenter();
		positionDelta.x *= currentView.getWidth() * currentView.getScale().x;
		positionDelta.y *= currentView.getHeight() * currentView.getScale().y;
		currentView.setPosition(currentView.getPosition() - positionDelta);
		currentView.setCenter(0.0f, 0.0f);

		// tell the current view that we're leaving it
		currentView.exit();
	}

	if(newIndex < 0 || newIndex > mViewers.size() - 1){
		mCurItemIndex = 0;
	} else {
		mCurItemIndex = newIndex;
	}

	const float destX = -(float)(mCurItemIndex)* getWidth();
	mHolder->tweenPosition(ci::Vec3f(destX, 0.0f, 0.0f), mAnimateDuration, 0.0f, ci::EaseInOutQuad());

	// tell the new view that we're entering it
	mViewers[mCurItemIndex]->enter();

	setCurrentInterface();

	loadCurrentAndAdjacent();
	recenterSlides();
}

void MediaSlideshow::setCurrentInterface(){
	if(mViewers.empty() || mCurItemIndex < 0 || mCurItemIndex > mViewers.size() - 1) return;

	// if there was an interface, get rid of it
	if(mCurrentInterface){
		mCurrentInterface->turnOff();
		mViewers[mCurItemIndex]->addChildPtr(mCurrentInterface);
		mCurrentInterface = nullptr;
	}

	// see if there's an interface to display
	mCurrentInterface = mViewers[mCurItemIndex]->getInterface();
	if(mCurrentInterface){
		mCurrentInterface->setPosition(getWidth() / 2.0f - mCurrentInterface->getWidth() / 2.0f, getHeight() - mCurrentInterface->getHeight() - 50.0f);
		addChildPtr(mCurrentInterface);
		mCurrentInterface->turnOff();
		mCurrentInterface->animateOn();
	}

}

void MediaSlideshow::gotoNext(const bool wrap){
	if(mViewers.empty()) return;
	int newIndex = mCurItemIndex + 1;
	if(newIndex > mViewers.size() - 1){
		if(!wrap){
			// don't do anything if we're not supposed to wrap around
			return;
		}
		newIndex = 0;
	}

	gotoItemIndex(newIndex);
}

void MediaSlideshow::gotoPrev(const bool wrap){
	if(mViewers.empty()) return;
	int newIndex = mCurItemIndex - 1;
	if(newIndex < 0){
		if(!wrap){
			// don't do anything if we're not supposed to wrap around
			return;
		}
		newIndex = mViewers.size() - 1;
	}

	gotoItemIndex(newIndex);
}

void MediaSlideshow::loadCurrentAndAdjacent(){
	// Simple sanity checks
	if(mViewers.empty() || mCurItemIndex < 0 || mCurItemIndex > mViewers.size() - 1) return;

	// load the current
	if(mCurItemIndex > -1){

		mViewers[mCurItemIndex]->initializeIfNeeded();

		int length = mViewers.size();

		int next = (mCurItemIndex + 1) % length;
		mViewers[next]->initializeIfNeeded();

		int prev = (mCurItemIndex + length - 1) % length;
		mViewers[prev]->initializeIfNeeded();
	}
}
} // namespace ui
} // namespace ds
