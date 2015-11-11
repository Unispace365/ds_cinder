#include "layout_sprite.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/image.h>


namespace ds {
namespace ui {

LayoutSprite::LayoutSprite(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mLayoutType(kLayoutVFlow)
	, mSpacing(0.0f)
{

}

void LayoutSprite::runLayout(){
	if(mLayoutType == kLayoutVFlow){
		runVLayout();
	} else if(mLayoutType == kLayoutHFlow){
		runHLayout();
	}

	onLayoutUpdate();
}

void LayoutSprite::runVLayout(){

	int numStretches = 0;
	float totalSize = 0.0f;
	const float layoutWidth = getWidth();

	std::vector<ds::ui::Sprite*>& chillins = getChildren();

	// Look through all children to determine total size and how many items are set to stretch to fill the space
	// Also run recursive layouts on any non-stretch layouts and size any flexible items
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kStretchSize){
			numStretches++;
		} else {

			if(chillin->mLayoutUserType == kFixedSize){
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(ls){
					ls->runLayout();
				}

			} else if(chillin->mLayoutUserType == kFlexSize){
				const float fixedW = layoutWidth - chillin->mLayoutLPad - chillin->mLayoutRPad;

				// TODO: more efficiently dynamic cast? (instead of all at once)
				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(mt){
					mt->setResizeLimit(fixedW);
				} else if(img){
					img->setScale(fixedW / img->getWidth());
				} else if(ls){
					ls->setSize(fixedW, ls->getHeight());
					ls->runLayout();
				} else {
					chillin->setSize(fixedW, chillin->getHeight());
				}
			}

			totalSize += chillin->getScaleHeight() + chillin->mLayoutTPad + chillin->mLayoutBPad + mSpacing;
		}
	}

	float leftOver = 0.0f;
	float perStretch = 0.0f;
	if(numStretches > 0){
		leftOver = getHeight() - totalSize;
		perStretch = leftOver / numStretches;
	}

	// Now that we know the size and leftover size, go through the children again, set position for all children 
	// and set the size of any stretch children
	float yp = 0.0f;
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kStretchSize){

			const float stretchW = layoutWidth - chillin->mLayoutLPad - chillin->mLayoutRPad;
			const float stretchH = perStretch - chillin->mLayoutTPad - chillin->mLayoutBPad;

			ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
			ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
			LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
			if(mt){
				mt->setResizeLimit(stretchW, stretchH);
			} else if(img){
				// TODO: fit inside?
				img->setScale(stretchH / img->getHeight());
			} else if(ls){
				ls->setSize(stretchW, stretchH);
				ls->runLayout();
			} else {
				chillin->setSize(stretchW, stretchH);
			}
		}

		yp += chillin->mLayoutTPad;

		float xPos = 0.0f;
		if(chillin->mLayoutHAlign == kLeft){
			xPos = chillin->mLayoutLPad;
		} else if(chillin->mLayoutHAlign == kCenter){
			xPos = layoutWidth / 2.0f - chillin->getScaleWidth() / 2.0f;
		} else if(chillin->mLayoutHAlign == kRight){
			xPos = layoutWidth - chillin->getScaleWidth() - chillin->mLayoutRPad;
		}

		chillin->setPosition(xPos, yp);

		yp += chillin->getScaleHeight() + chillin->mLayoutBPad + mSpacing;
	}

	if(mLayoutUserType == kFlexSize){
		setSize(layoutWidth, yp);
	}
}


void LayoutSprite::runHLayout(){

	int numStretches = 0;
	float totalSize = 0.0f;
	const float layoutHeight = getHeight();

	std::vector<ds::ui::Sprite*>& chillins = getChildren();

	// Look through all children to determine total size and how many items are set to stretch to fill the space
	// Also run recursive layouts on any non-stretch layouts and size any flexible items
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kStretchSize){
			numStretches++;
		} else {

			if(chillin->mLayoutUserType == kFixedSize){
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(ls){
					ls->runLayout();
				}

			} else if(chillin->mLayoutUserType == kFlexSize){
				const float fixedH = layoutHeight - chillin->mLayoutTPad - chillin->mLayoutBPad;

				// TODO: more efficiently dynamic cast? (instead of all at once)
				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(mt){
					mt->setResizeLimit(mt->getResizeLimitWidth(), fixedH);
				} else if(img){
					// TODO: fit inside?
					img->setScale(fixedH / img->getHeight());
				} else if(ls){
					ls->setSize(ls->getWidth(), fixedH);
					ls->runLayout();
				} else {
					chillin->setSize(chillin->getWidth(), fixedH);
				}
			}

			totalSize += chillin->getScaleWidth() + chillin->mLayoutRPad + chillin->mLayoutLPad + mSpacing;
		}
	}

	float leftOver = 0.0f;
	float perStretch = 0.0f;
	if(numStretches > 0){
		leftOver = getWidth() - totalSize;
		perStretch = leftOver / numStretches;
	}

	// Now that we know the size and leftover size, go through the children again, set position for all children 
	// and set the size of any stretch children
	float xp = 0.0f;
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kStretchSize){

			const float stretchH = layoutHeight - chillin->mLayoutTPad - chillin->mLayoutBPad;
			const float stretchW = perStretch - chillin->mLayoutLPad - chillin->mLayoutRPad;

			ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
			ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
			LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
			if(mt){
				mt->setResizeLimit(stretchW, stretchH);
			} else if(img){
				// TODO: fit inside?
				img->setScale(stretchW / img->getWidth());
			} else if(ls){
				ls->setSize(stretchW, stretchH);
				ls->runLayout();
			} else {
				chillin->setSize(stretchW, stretchH);
			}
		}

		xp += chillin->mLayoutLPad;

		float yPos = 0.0f;
		if(chillin->mLayoutVAlign == kTop){
			yPos = chillin->mLayoutTPad;
		} else if(chillin->mLayoutVAlign == kMiddle){
			yPos = layoutHeight / 2.0f - chillin->getScaleWidth() / 2.0f;
		} else if(chillin->mLayoutVAlign == kBottom){
			yPos = layoutHeight - chillin->getScaleHeight() - chillin->mLayoutBPad;
		}

		chillin->setPosition(xp, yPos);

		xp += chillin->getScaleWidth() + chillin->mLayoutRPad + mSpacing;
	}

	if(mLayoutUserType == kFlexSize){
		setSize(xp, layoutHeight);
	}
}

void LayoutSprite::onLayoutUpdate(){
	if(mLayoutUpdatedFunction){
		mLayoutUpdatedFunction();
	}
}

void LayoutSprite::setLayoutUpdatedFunction(const std::function<void()> layoutUpdatedFunction){
	mLayoutUpdatedFunction = layoutUpdatedFunction;
}

} // namespace ui
} // namespace ds
