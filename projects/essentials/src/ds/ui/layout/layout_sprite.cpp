#include "layout_sprite.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/image.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

LayoutSprite::LayoutSprite(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mLayoutType(kLayoutVFlow)
	, mSpacing(0.0f)
{

}

void LayoutSprite::runLayout(){
	if(mLayoutType == kLayoutNone){
		runNoneLayout();
	} else if(mLayoutType == kLayoutVFlow){
		runVLayout();
	} else if(mLayoutType == kLayoutHFlow){
		runHLayout();
	} else if(mLayoutType == kLayoutSize){
		runSizeLayout();
	}

	onLayoutUpdate();
}

void LayoutSprite::runNoneLayout(){
	std::vector<ds::ui::Sprite*>& chillins = getChildren();
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);
		auto layoutSprite = dynamic_cast<LayoutSprite*>(chillin);
		if(layoutSprite){
			layoutSprite->runLayout();
		}
	}
}

void LayoutSprite::runSizeLayout(){

	std::vector<ds::ui::Sprite*>& chillins = getChildren();
	const float layoutWidth = getWidth();
	const float layoutHeight = getHeight();

	// Size layout just adjusts the size of child elements to fit, if specified.
	// If children should ignore the size, don't add a layout size and set them to fixed.
	// Otherwise, stretch and flex both "gracefully" match sprites to the size of this layout
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kFixedSize){
			if(chillin->mLayoutSize.x > 0.0f && chillin->mLayoutSize.y > 0.0f){
				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
				if(mt){
					mt->setResizeLimit(chillin->mLayoutSize.x, chillin->mLayoutSize.y);
				} else if(img){
					// restore position after calculating the box size
					ci::Vec3f prePos = img->getPosition();
					fitInside(img, ci::Rectf(0.0f, 0.0f, chillin->mLayoutSize.x, chillin->mLayoutSize.y), true);
					img->setPosition(prePos);
				} else {
					chillin->setSize(mLayoutSize);
				}
			}
		} else if(chillin->mLayoutUserType == kStretchSize || chillin->mLayoutUserType == kFlexSize || chillin->mLayoutUserType == kFillSize){
			const float fixedW = layoutWidth - chillin->mLayoutLPad - chillin->mLayoutRPad;
			const float fixedH = layoutHeight - chillin->mLayoutTPad - chillin->mLayoutBPad;

			ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
			ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
			LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
			if(mt){
				mt->setResizeLimit(fixedW, fixedH);
			} else if(img){
				// restore position after calculating the box size
				ci::Vec3f prePos = img->getPosition();
				fitInside(img, ci::Rectf(0.0f, 0.0f, fixedW, fixedH), true);
				img->setPosition(prePos);
			} else if(ls){
				ls->setSize(fixedW, fixedH);
				ls->runLayout();
			} else {
				chillin->setSize(fixedW, fixedH);
			}
		}


		LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
		if(ls){
			ls->runLayout();
		}
	}

}

void LayoutSprite::runVLayout(){

	bool hasFills = false;
	int numStretches = 0;
	float totalSize = 0.0f;
	const float layoutWidth = getWidth();
	const float layoutHeight = getHeight();

	std::vector<ds::ui::Sprite*>& chillins = getChildren();

	// Look through all children to determine total size and how many items are set to stretch to fill the space
	// Also run recursive layouts on any non-stretch layouts and size any flexible items
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kFillSize){
			hasFills = true;
		} else if(chillin->mLayoutUserType == kStretchSize){
			numStretches++;
		} else {

			if(chillin->mLayoutUserType == kFixedSize){
				if(chillin->mLayoutSize.x > 0.0f && chillin->mLayoutSize.y > 0.0f){
					ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
					ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
					if(mt){
						mt->setResizeLimit(chillin->mLayoutSize.x, chillin->mLayoutSize.y);
					} else if(img){
						fitInside(img, ci::Rectf(0.0f, 0.0f, chillin->mLayoutSize.x, chillin->mLayoutSize.y), true);
					} else {
						chillin->setSize(mLayoutSize);
					}

				}

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
				fitInside(img, ci::Rectf(0.0f, 0.0f, stretchW, stretchH), true);
			} else if(ls){
				ls->setSize(stretchW, stretchH);
				ls->runLayout();
			} else {
				chillin->setSize(stretchW, stretchH);
			}
		}

		float xPos = 0.0f;
		if(chillin->mLayoutHAlign == kLeft){
			xPos = chillin->mLayoutLPad;
		} else if(chillin->mLayoutHAlign == kCenter){
			xPos = layoutWidth / 2.0f - chillin->getScaleWidth() / 2.0f;
		} else if(chillin->mLayoutHAlign == kRight){
			xPos = layoutWidth - chillin->getScaleWidth() - chillin->mLayoutRPad;
		}
		
		// Fill size only uses padding and fudge, but doesn't contribute to the flow
		if(chillin->mLayoutUserType == kFillSize){
			chillin->setPosition(xPos + chillin->mLayoutFudge.x, chillin->mLayoutFudge.y + chillin->mLayoutTPad);
			continue;
		}

		yp += chillin->mLayoutTPad;

		chillin->setPosition(xPos + chillin->mLayoutFudge.x, yp + chillin->mLayoutFudge.y);

		yp += chillin->getScaleHeight() + chillin->mLayoutBPad + mSpacing;
	}

	if(mLayoutUserType == kFlexSize){
		setSize(layoutWidth, yp);
	}

	if(hasFills){
		for(auto it = chillins.begin(); it < chillins.end(); ++it){
			ds::ui::Sprite* chillin = (*it);
			if(chillin->mLayoutUserType == kFillSize){
				const float fixedW = getWidth() - chillin->mLayoutLPad - chillin->mLayoutRPad;
				const float fixedH = getHeight() - chillin->mLayoutTPad - chillin->mLayoutBPad;

				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(mt){
					mt->setResizeLimit(fixedW, fixedH);
				} else if(img){
					fitInside(img, ci::Rectf(0.0f, 0.0f, fixedW, fixedH), true);
				} else if(ls){
					ls->setSize(fixedW, fixedH);
					ls->runLayout();
				} else {
					chillin->setSize(fixedW, fixedH);
				}
			}
		}
	}
}


void LayoutSprite::runHLayout(){

	bool hasFills = false;
	int numStretches = 0;
	float totalSize = 0.0f;
	const float layoutWidth = getWidth();
	const float layoutHeight = getHeight();

	std::vector<ds::ui::Sprite*>& chillins = getChildren();

	// Look through all children to determine total size and how many items are set to stretch to fill the space
	// Also run recursive layouts on any non-stretch layouts and size any flexible items
	for(auto it = chillins.begin(); it < chillins.end(); ++it){
		ds::ui::Sprite* chillin = (*it);

		if(chillin->mLayoutUserType == kFillSize){
			hasFills = true;
		} else if(chillin->mLayoutUserType == kStretchSize){
			numStretches++;
		} else {

			if(chillin->mLayoutUserType == kFixedSize){
				if(chillin->mLayoutSize.x > 0.0f && chillin->mLayoutSize.y > 0.0f){
					ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
					ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
					if(mt){
						mt->setResizeLimit(chillin->mLayoutSize.x, chillin->mLayoutSize.y);
					} else if(img){
						fitInside(img, ci::Rectf(0.0f, 0.0f, chillin->mLayoutSize.x, chillin->mLayoutSize.y), true);
					} else {
						chillin->setSize(mLayoutSize);
					}

				}

				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(ls){
					ls->runLayout();
				}

			} else if(chillin->mLayoutUserType == kFlexSize){
				const float fixedH = layoutHeight - chillin->mLayoutTPad - chillin->mLayoutBPad;

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
				fitInside(img, ci::Rectf(0.0f, 0.0f, stretchW, stretchH), true);
			} else if(ls){
				ls->setSize(stretchW, stretchH);
				ls->runLayout();
			} else {
				chillin->setSize(stretchW, stretchH);
			}
		}

		float yPos = 0.0f;
		if(chillin->mLayoutVAlign == kTop){
			yPos = chillin->mLayoutTPad;
		} else if(chillin->mLayoutVAlign == kMiddle){
			yPos = layoutHeight / 2.0f - chillin->getScaleHeight() / 2.0f;
		} else if(chillin->mLayoutVAlign == kBottom){
			yPos = layoutHeight - chillin->getScaleHeight() - chillin->mLayoutBPad;
		}

		// Fill size only uses padding and fudge, but doesn't contribute to the flow
		if(chillin->mLayoutUserType == kFillSize){
			chillin->setPosition(chillin->mLayoutLPad + chillin->mLayoutFudge.x, chillin->mLayoutFudge.y + yPos);
			continue;
		}

		xp += chillin->mLayoutLPad;

		chillin->setPosition(xp + chillin->mLayoutFudge.x, yPos + chillin->mLayoutFudge.y);

		xp += chillin->getScaleWidth() + chillin->mLayoutRPad + mSpacing;
	}

	if(mLayoutUserType == kFlexSize){
		setSize(xp, layoutHeight);
	}

	if(hasFills){
		for(auto it = chillins.begin(); it < chillins.end(); ++it){
			ds::ui::Sprite* chillin = (*it);
			if(chillin->mLayoutUserType == kFillSize){
				const float fixedW = getWidth() - chillin->mLayoutLPad - chillin->mLayoutRPad;
				const float fixedH = getHeight() - chillin->mLayoutTPad - chillin->mLayoutBPad;

				ds::ui::MultilineText* mt = dynamic_cast<ds::ui::MultilineText*>(chillin);
				ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(chillin);
				LayoutSprite* ls = dynamic_cast<LayoutSprite*>(chillin);
				if(mt){
					mt->setResizeLimit(fixedW, fixedH);
				} else if(img){
					fitInside(img, ci::Rectf(0.0f, 0.0f, fixedW, fixedH), true);
				} else if(ls){
					ls->setSize(fixedW, fixedH);
					ls->runLayout();
				} else {
					chillin->setSize(fixedW, fixedH);
				}
			}
		}
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

void LayoutSprite::fitInside(ds::ui::Sprite* sp, const ci::Rectf area, const bool letterbox){
	if(!sp) return;
	// a = w / h;
	// h = w /a;
	// w = ah;
	const float spriteAspect = sp->getWidth() / sp->getHeight();
	const float areaAspect = area.getWidth() / area.getHeight();
	float destScale = sp->getScale().x;

	if(letterbox){
		// When letterboxing, if the sprite is narrower then the dest area, fill the height
		if(spriteAspect < areaAspect){
			destScale = area.getHeight() / sp->getHeight();
		} else {
			destScale = area.getWidth() / sp->getWidth();
		}
	} else {
		// When NOT letterboxing, if the sprite is wider then the dest area, fill the height
		if(spriteAspect > areaAspect){
			destScale = area.getHeight() / sp->getHeight();
		} else {
			destScale = area.getWidth() / sp->getWidth();
		}
	}

	sp->setScale(destScale, destScale, 1.0f);
	sp->setPosition(area.getX1() + area.getWidth() / 2.0f - sp->getScaleWidth() / 2.0f, area.getY1() + area.getHeight() / 2.0f - sp->getScaleHeight() / 2.0f);
}

} // namespace ui
} // namespace ds
