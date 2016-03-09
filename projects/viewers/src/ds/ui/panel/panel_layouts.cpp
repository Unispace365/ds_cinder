#include "panel_layouts.h"

#include <ds/debug/logger.h>

#include "ds/util/pixel_packer/util/myVector2.h"
#include "ds/util/pixel_packer/util/myBox.h"
#include "ds/util/pixel_packer/pak/algoMaxRects.h"
#include "ds/util/pixel_packer/pak/algoGil.h"
#include <map>

namespace ds {
namespace ui {

struct PanelPackage {
	int mPanelIndex;
	float mAsepectRatio;
	t_myVector2	mPackSize;
	ci::Rectf	mOutputRect;
};

bool sortByBigness(PanelPackage& a, PanelPackage& b){
	return a.mPackSize.x * a.mPackSize.y > b.mPackSize.x * b.mPackSize.y;
}

float adjustSizes(std::vector<PanelPackage>& packages, const float fractionalAmount, const float padding){
	float piecemealArea = 0.0f;
	for(auto it = packages.begin(); it < packages.end(); ++it){
		float tw = (float)(*it).mPackSize.x * fractionalAmount;
		float th = tw / (*it).mAsepectRatio;
		tw += padding;
		th += padding;

		piecemealArea += tw * th;
		(*it).mPackSize.x = (int)(tw);
		(*it).mPackSize.y = (int)(th);
	}

	return piecemealArea;
}

bool PanelLayouts::binPack(std::vector<ds::ui::BasePanel*> panels, const ci::Vec2f totalArea, const float padding, const float animDur){
	if(panels.empty()) return false;

	std::vector<PanelPackage> thePackages;
	int ind = 0;


	float totalAreaAmount = totalArea.x * totalArea.y;
	float piecemealArea = 0.0f;
	for(auto it = panels.begin(); it < panels.end(); ++it){
		float tw = (*it)->getWidth();
		float th = (*it)->getHeight();
		if(tw < 1.0f || th < 1.0f) continue;

		PanelPackage pp;
		pp.mAsepectRatio = (*it)->getContentAspectRatio();

		if(tw > totalArea.x){
			tw = totalArea.x;
			th = tw / pp.mAsepectRatio;
		}

		if(th > totalArea.y){
			th = totalArea.y;
			tw = th * pp.mAsepectRatio;
		}

		// do the width again in case the height calculation made it too wide
		if(tw > totalArea.x){
			tw = totalArea.x;
			th = tw / pp.mAsepectRatio;
		}
		piecemealArea += tw * th;
		pp.mPackSize = t_myVector2((int)tw, (int)th);
		pp.mPanelIndex = ind;
		thePackages.push_back(pp);
		ind++;
	}

	// be sure padding gets added
	adjustSizes(thePackages, 1.0f, padding);

	while(piecemealArea < totalAreaAmount * 0.75){
		piecemealArea = adjustSizes(thePackages, 1.05f, padding);
	}

	while(piecemealArea > totalAreaAmount * 0.9f){
		piecemealArea = adjustSizes(thePackages, 0.95f, padding);
	}


	std::vector<PanelPackage> outputPackages;
	bool isFine = false;
	t_algoMaxRects packs;

	for(int i = 0; i < 100; i++){
		isFine = false;
		std::vector<t_myVector2> sizes;
		for(auto it = thePackages.begin(); it < thePackages.end(); ++it){
			sizes.push_back((*it).mPackSize);
		}

		auto returny = packs.pack(sizes, t_myVector2((int)totalArea.x, (int)totalArea.y), isFine);
		if(isFine){

			for(auto it = returny.begin(); it != returny.end(); ++it){
				int xx = (*it).second.x;
				int yy = (*it).second.y;
				int wid = (*it).first.x;
				int hei = (*it).first.y;
				int right = xx + wid;
				int bottom = yy + hei;

				for(auto it = thePackages.begin(); it < thePackages.end(); ++it){
					if((*it).mPackSize.x == wid && (*it).mPackSize.y == hei){
						(*it).mOutputRect = ci::Rectf((float)xx, (float)yy, (float)right, (float)bottom);
						outputPackages.push_back((*it));
						thePackages.erase(it);
						break;
					}
				}
			}
			break;
		} else {
			piecemealArea = adjustSizes(thePackages, 0.95f, padding);
		}
	}

	if(outputPackages.empty()){
		DS_LOG_WARNING("Panel Layouts couldn't determine a layout!");
		return false;
	}

	float farthestRight = 0.0f;
	float farthestBotto = 0.0f;
	for(auto it = outputPackages.begin(); it < outputPackages.end(); ++it){
		ci::Vec2f br = (*it).mOutputRect.getLowerRight();
		if(br.x > farthestRight) farthestRight = br.x;
		if(br.y > farthestBotto) farthestBotto = br.y;
	}

	float offsetX = (totalArea.x - farthestRight) / 2.0f;
	float offsetY = (totalArea.y - farthestBotto) / 2.0f;
	float delayey = 0.0f;
	const float deltaDelay = animDur / (float)(outputPackages.size());
	for(auto it = outputPackages.begin(); it < outputPackages.end(); ++it){
		ci::Rectf recty = (*it).mOutputRect;
		auto tmv = panels[(*it).mPanelIndex];
		ci::Vec3f destination = ci::Vec3f(recty.getUpperLeft().x + offsetX, recty.getUpperLeft().y + offsetY, 0.0f);
		float destWidth = recty.getWidth() - padding;
		if(animDur > 0.0f){
			tmv->tweenStarted();
			tmv->tweenPosition(destination, animDur, delayey, ci::EaseInOutQuad(), [tmv]{ tmv->tweenEnded(); });
			tmv->animateWidthTo(destWidth);
		} else {
			tmv->setPosition(destination);
			tmv->setViewerWidth(destWidth);
		}
	}

	return true;
}
}
}