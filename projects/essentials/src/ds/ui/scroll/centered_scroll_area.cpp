#include "centered_scroll_area.h"

#include <ds/ui/layout/layout_sprite.h>

namespace ds{
namespace ui{

CenteredScrollArea::CenteredScrollArea(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool vertical)
	: ScrollArea(engine, startWidth, startHeight, vertical)
	, mCenterBy(1)
	, mCenterIndex(0)
	, mMinimumSwipeDistance(50.0f)
{
	// add the swipe behavior
	if(mScroller){
		mScroller->setSwipeCallback([this](ds::ui::Sprite* sprite, const ci::Vec3f& delta){
			bool shouldCenter = false;
			int index = 0;

			int itemCount = getItemCount();
			if(itemCount > 1) {
				if(mVertical){
					if(delta.y > mMinimumSwipeDistance){
						shouldCenter = true;
						index = mCenterIndex - mCenterBy;
					} else if(delta.y < -mMinimumSwipeDistance){
						shouldCenter = true;
						index = mCenterIndex + mCenterBy;
					}
				} else {
					if(delta.x > mMinimumSwipeDistance){
						shouldCenter = true;
						index = mCenterIndex - mCenterBy;
					} else if(delta.x < -mMinimumSwipeDistance){
						shouldCenter = true;
						index = mCenterIndex + mCenterBy;
					}
				}
			}

			if(shouldCenter){
				if(mWillSnapAfterDelay){
					mWillSnapAfterDelay = false;
					cancelDelayedCall();
				}
				balanceOnIndex(index, mReturnAnimateTime, 0.0f, ci::EaseOutQuint(), [this](){ scrollerTweenUpdated(); });
			}
		});
	}
}

void CenteredScrollArea::setCenterBy(int centerBy){
	mCenterBy = centerBy;
}

int CenteredScrollArea::getItemCount(){
	int output = 0;

	Sprite* firstChild = getFirstChild();
	if(firstChild){
		auto grandChildren = firstChild->getChildren();
		output = grandChildren.size();
	}

	return output;
}

Sprite* CenteredScrollArea::getFirstChild(){
	Sprite* output = nullptr;

	if(mScroller){
		auto children = mScroller->getChildren();
		if(children.size() > 0){
			output = children.front();
		}
	}

	return output;
}

int CenteredScrollArea::getCenterIndex(){
	return mCenterIndex;
}

void CenteredScrollArea::centerOnIndex(int index, float duration, float delay, const ci::EaseFn& ease, const std::function<void()>& postFunc){
	bool callPostFuncNow = true;
	if(mScroller){
		float p = trueCenterOfItem(index);
		
		ci::Vec3f position;
		if(mVertical){
			position.y = p;
		} else {
			position.x = p;
		}

		// kill any existing tween and/or momentum
		mSpriteMomentum.deactivate();
		mScroller->animStop();

		if(duration > 0.0f){
			callPostFuncNow = false;
			mScroller->tweenPosition(position, duration, delay, ease, postFunc);
		} else {
			mScroller->setPosition(position);
		}
	}

	mCenterIndex = index;

	if(callPostFuncNow && postFunc){
		postFunc();
	}
}

void CenteredScrollArea::balanceOnIndex(int index, float duration, float delay, const ci::EaseFn& ease, const std::function<void()>& postFunc){
	bool callPostFuncNow = true;
	if(mScroller){
		float p = 0.0f;
		
		int itemCount = getItemCount();
		if(itemCount < mCenterBy){
			// just center it
			float extent;
			float scrollerExtent;
			if(mVertical){
				extent = getHeight();
				scrollerExtent = mScroller->getHeight();
			} else {
				extent = getWidth();
				scrollerExtent = mScroller->getWidth();
			}
			p = ((extent - scrollerExtent) * 0.5f);
		} else {
			p = relaxedCenterOfItem(index);
		}

		ci::Vec3f position;
		if(mVertical){
			position.y = p;
		} else {
			position.x = p;
		}

		// kill any existing tween and/or momentum
		mSpriteMomentum.deactivate();
		mScroller->animStop();

		if(duration > 0.0f){
			callPostFuncNow = false;
			mScroller->tweenPosition(position, duration, delay, ease, postFunc);
		} else {
			mScroller->setPosition(position);
		}
	}

	mCenterIndex = index;

	if(callPostFuncNow && postFunc){
		postFunc();
	}
}

bool CenteredScrollArea::callSnapToPositionCallback(bool& doTween, ci::Vec3f& tweenDestination){
	// accept an externally-imposed snap function, if provided, but otherwise use our own logic
	bool output = ScrollArea::callSnapToPositionCallback(doTween, tweenDestination);

	if(!output){
		// do our own snapping
		if(mScroller){
			int itemCount = getItemCount();
			if(itemCount > 0){
				float extent;
				if(mVertical){
					extent = getHeight();
				} else {
					extent = getWidth();
				}

				float centerInLayoutSpace = (extent * 0.5f);
				if(mVertical){
					centerInLayoutSpace -= tweenDestination.y;
				} else {
					centerInLayoutSpace -= tweenDestination.x;
				}
		
				float itemSize = 0.0f;
				float itemSpacing = 0.0f;
					
				Sprite* firstChild = getFirstChild();
				if(firstChild){
					auto children = firstChild->getChildren();
					if(children.size() > 0){
						Sprite* representativeChild = children.front();
						if(representativeChild){
							if(mVertical){
								itemSize = representativeChild->getHeight();
							} else {
								itemSize = representativeChild->getWidth();
							}
						}
					}

					LayoutSprite* layout = dynamic_cast<LayoutSprite*>(firstChild);
					if(layout){
						itemSpacing = layout->getSpacing();
					}
				}

				float itemTotal = (itemSize + itemSpacing);

				int index = 0;
				if(isOdd()){
					index = (int)floorf((centerInLayoutSpace + (itemSpacing * 0.5f)) / itemTotal);
				} else {
					index = (int)floorf((centerInLayoutSpace - (itemTotal * 0.5f)) / itemTotal);
				}

				float p = relaxedCenterOfItem(index);

				mCenterIndex = index;
			
				// we're ready to tween
				doTween = true;
				if(mVertical){
					tweenDestination.y = p;
				} else {
					tweenDestination.x = p;
				}

				output = true;
			}
		}
	}

	return output;
}

float CenteredScrollArea::trueCenterOfItem(int index){
	float output = 0.0;

	if(mScroller){
		int itemCount = getItemCount();
		if((index >= 0) && (index < itemCount)){
			float extent;
			if(mVertical){
				extent = getHeight();
			} else {
				extent = getWidth();
			}

			float itemSize = 0.0f;
			float itemSpacing = 0.0f;
					
			Sprite* firstChild = getFirstChild();
			if(firstChild){
				auto children = firstChild->getChildren();
				if(children.size() > 0){
					Sprite* representativeChild = children.front();
					if(representativeChild){
						if(mVertical){
							itemSize = representativeChild->getHeight();
						} else {
							itemSize = representativeChild->getWidth();
						}
					}
				}

				LayoutSprite* layout = dynamic_cast<LayoutSprite*>(firstChild);
				if(layout){
					itemSpacing = layout->getSpacing();
				}
			}

			float itemTotal = (itemSize + itemSpacing);

			output = (extent * 0.5f) - ((index * itemTotal) + (itemSize * 0.5f));
		}
	}

	return output;
}

float CenteredScrollArea::trueCenterAfterItem(int index){
	float output = 0.0;

	if(mScroller){
		int itemCount = getItemCount();

		if((index >= 0) && (index < itemCount)){
			float extent;
			if(mVertical){
				extent = getHeight();
			} else {
				extent = getWidth();
			}

			float itemSize = 0.0f;
			float itemSpacing = 0.0f;
					
			Sprite* firstChild = getFirstChild();
			if(firstChild){
				auto children = firstChild->getChildren();
				if(children.size() > 0){
					Sprite* representativeChild = children.front();
					if(representativeChild){
						if(mVertical){
							itemSize = representativeChild->getHeight();
						} else {
							itemSize = representativeChild->getWidth();
						}
					}
				}

				LayoutSprite* layout = dynamic_cast<LayoutSprite*>(firstChild);
				if(layout){
					itemSpacing = layout->getSpacing();
				}
			}

			float itemTotal = (itemSize + itemSpacing);

			output = (extent * 0.5f) - (((index + 1) * itemTotal) - (itemSpacing * 0.5f));
		}
	}

	return output;
}

float CenteredScrollArea::relaxedCenterOfItem(int& index){
	float output = 0.0f;

	if(mScroller){
		int itemCount = getItemCount();
		if(itemCount < mCenterBy){
			// just center it
			float extent;
			float scrollerExtent;
			if(mVertical){
				extent = getHeight();
				scrollerExtent = mScroller->getHeight();
			} else {
				extent = getWidth();
				scrollerExtent = mScroller->getWidth();
			}
			output = ((extent - scrollerExtent) * 0.5f);
		} else {
			int middleIndex = (mCenterBy / 2);
			if(isOdd()){
				// deal with edge cases so we always show the right amount at a time
				index = (index < middleIndex) ? middleIndex : ((index > itemCount - (middleIndex + 1)) ? itemCount - (middleIndex + 1) : index);
				output = trueCenterOfItem(index);
			} else {
				// always going to be halfway between two thumbnails
				index = (index < middleIndex - 1) ? middleIndex - 1 : ((index > itemCount - (middleIndex + 1)) ? itemCount - (middleIndex + 1) : index);
				output = trueCenterAfterItem(index); 
			}
		}
	}

	return output;
}

} // namespace ui

} // namespace ds