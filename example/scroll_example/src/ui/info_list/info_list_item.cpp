#include "info_list_item.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

namespace example{
InfoListItem::InfoListItem(Globals& g, const float widthy, const float heighty)
	: ds::ui::Sprite(g.mEngine, widthy, heighty)
	, mGlobals(g)
	, mLabel(nullptr)
	, mBackground(nullptr)
	, mThumbnail(nullptr)
{

	mBackground = new ds::ui::Sprite(mEngine, widthy, heighty);
	mLabel = new ds::ui::Text(mEngine);
	mThumbnail = new ds::ui::Image(mEngine);

	if(!mThumbnail || !mLabel || !mBackground){
		DS_LOG_WARNING("Error creating related product item! Check XML! Or Somethingngngng!");
		return;
	}

	mBackground->setTransparent(false);
	mBackground->setColor(ci::Color(0.3f, 0.3f, 0.3f));
	addChildPtr(mBackground);

	mLabel->setFont("noto-sans", 24.0f);
	mLabel->setColor(ci::Color::white());
	addChildPtr(mLabel);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
}

void InfoListItem::setInfo(const ds::model::StoryRef info){
	mInfoModel = info;

	if(mLabel){
		std::wstring uppername = mInfoModel.getName();
		std::transform(uppername.begin(), uppername.end(), uppername.begin(), ::toupper);
		mLabel->setText(uppername);
	}

	if(mThumbnail){
		// TODO: support thumbnails
		//mThumbnail->setResourceID(product.getSquareThumb().getDbId());
	}

	setState(0);
	layout();
}

ds::model::StoryRef InfoListItem::getInfo(){
	return mInfoModel;
}


void InfoListItem::animateOn(const float delay){
	setOpacity(0.0f);

	tweenOpacity(1.0f, 0.35f, delay);
}

void InfoListItem::layout(){
	if(mBackground){
		mBackground->setSize(getWidth(), getHeight());
	}

	if(mLabel && mThumbnail){
		const float thumbPad = mGlobals.getSettingsLayout().getFloat("info_list:item:thumb_pad", 0, 40.0f);
		const float xPad = mGlobals.getSettingsLayout().getFloat("info_list:item:text_x_pad", 0, 40.0f);
		const float yPad = mGlobals.getSettingsLayout().getFloat("info_list:item:text_y_pad", 0, 40.0f);
// 		mThumbnail->setHeight(getHeight());
// 		mThumbnail->setPosition(thumbPad, 0.0f);

		mLabel->setPosition(thumbPad + mThumbnail->getScaleWidth() + xPad, getHeight() / 2.0f - mLabel->getHeight() / 2.0f + yPad);
	}

}


void InfoListItem::setState(const int buttonState){
	if(mBackground){
		ci::Color normalColor = mGlobals.getSettingsLayout().getColor("info_list:item:normal_color", 0, ci::Color::white());
		ci::Color highliColor = mGlobals.getSettingsLayout().getColor("info_list:item:highli_color", 0, ci::Color::white());
		if(buttonState == 1){
			mBackground->setColor(highliColor);
		} else {
			mBackground->setColor(normalColor);
		}
	}
}

}
