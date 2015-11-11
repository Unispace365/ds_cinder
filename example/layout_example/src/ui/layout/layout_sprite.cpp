#include "layout_sprite.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/image.h>


namespace example {

LayoutSprite::LayoutSprite(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
{

}

void LayoutSprite::runLayout(){

	int numStretches = 0;
	float totalSize = 0.0f;
	const float layoutWidth = getWidth();

	std::vector<ds::ui::Sprite*>& chillins = getChildren();

	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* ls = (*it);

		if(ls->mLayoutUserType == kStretchSize){
			numStretches++;
		} else {
			if(ls->mLayoutUserType == kFixedSize){
				const float fixedW = layoutWidth - ls->mLayoutLPad - ls->mLayoutRPad;
				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(ls);
				if(mt){
					mt->setResizeLimit(fixedW, mt->getResizeLimitHeight());
				} else {
					ls->setSize(fixedW, ls->getHeight());
				}

			} else if(ls->mLayoutUserType == kFlexSize){
				const float fixedW = layoutWidth - ls->mLayoutLPad - ls->mLayoutRPad;
				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(ls);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(ls);
				if(mt){
					mt->setResizeLimit(fixedW);
				} else if(img){
					img->setScale(fixedW / img->getWidth());
				} else {
					ls->setSize(layoutWidth - ls->mLayoutLPad - ls->mLayoutRPad, ls->getHeight());
				}

				//lm.runLayout();
			}

			totalSize += ls->getScaleHeight() + ls->mLayoutTPad + ls->mLayoutBPad + mSpacing;
		}
	}

	float leftOver = 0.0f;
	float perStretch = 0.0f;
	if(numStretches > 0){
		leftOver = getHeight() - totalSize;
		perStretch = leftOver / numStretches;
	}

	float yp = 0.0f;
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* ls = (*it);

		yp += ls->mLayoutTPad;
		ls->setPosition(ls->mLayoutLPad, yp);

		if(ls->mLayoutUserType == kStretchSize){

			const float stretchW = layoutWidth - ls->mLayoutLPad - ls->mLayoutRPad;
			const float stretchH = perStretch - ls->mLayoutTPad - ls->mLayoutBPad;

			ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(ls);
			if(mt){
				mt->setResizeLimit(stretchW, stretchH);
			} else {
				ls->setSize(stretchW, stretchH);
			}

			//lm.runLayout();
		}

		yp += ls->getScaleHeight() + ls->mLayoutBPad + mSpacing;
	}

	if(mLayoutUserType == kFlexSize){
		setSize(layoutWidth, yp);
	}

	onLayoutUpdate();
}

void LayoutSprite::onLayoutUpdate(){
	if(mLayoutUpdatedFunction){
		mLayoutUpdatedFunction();
	}
}

void LayoutSprite::setLayoutUpdatedFunction(const std::function<void()> layoutUpdatedFunction){
	mLayoutUpdatedFunction = layoutUpdatedFunction;
}

} // namespace example
