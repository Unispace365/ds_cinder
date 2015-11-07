#include "pdf_interface.h"

#include <ds/app/environment.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/pdf.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/text.h>

namespace ds {
namespace ui {

PDFInterface::PDFInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& sizey, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor)
	: MediaInterface(eng, sizey, backgroundColor)
	, mLinkedPDF(nullptr)
	, mUpButton(nullptr)
	, mDownButton(nullptr)
	, mPageCounter(nullptr)
{
	mUpButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/prev.png", "%APP%/data/images/media_interface/prev.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mUpButton);
	mUpButton->setClickFn([this](){
		if(mLinkedPDF){
			if(mLinkedPDF->getPageNum() > 1){
				mLinkedPDF->goToPreviousPage();
				updateWidgets();
			}
		}
	});

	mUpButton->getNormalImage().setColor(buttonColor);
	mUpButton->getHighImage().setColor(buttonColor / 2.0f);
	mUpButton->setScale(sizey.y / mUpButton->getHeight());

	mDownButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/next.png", "%APP%/data/images/media_interface/next.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mDownButton);
	mDownButton->setClickFn([this](){
		if(mLinkedPDF){
			if(mLinkedPDF->getPageNum() < mLinkedPDF->getPageCount()){
				mLinkedPDF->goToNextPage();
				updateWidgets();
			}
		}
	});

	mDownButton->getNormalImage().setColor(buttonColor);
	mDownButton->getHighImage().setColor(buttonColor / 2.0f);
	mDownButton->setScale(sizey.y / mDownButton->getHeight());

	mPageCounter = mEngine.getEngineCfg().getText("viewer:widget").create(mEngine, this);
	if(mPageCounter){
		mPageCounter->setResizeToText(true);
		mPageCounter->enable(false);
	}

	updateWidgets();
}

void PDFInterface::linkPDF(ds::ui::Pdf* linkedPDF){
	mLinkedPDF = linkedPDF;
	updateWidgets();
}


// Layout is called when the size is changed, so don't change the size in the layout
void PDFInterface::onLayout(){
	if(mUpButton && mDownButton && mPageCounter){
		const float w = getWidth();
		const float h = getHeight();
		const float padding = h / 4.0f;

		float componentsWidth = (
			mUpButton->getScaleWidth() + padding +
			mPageCounter->getScaleWidth() + padding +
			mDownButton->getScaleWidth()
			);

		float margin = ((w - componentsWidth) * 0.5f);
		float xp = margin;

		mUpButton->setPosition(xp, (h * 0.5f) - (mUpButton->getScaleHeight() * 0.5f));
		xp += mUpButton->getScaleWidth() + padding;

		mPageCounter->setPosition(xp, (h * 0.5f) - (mPageCounter->getHeight() * 0.6f));
		xp += mPageCounter->getScaleWidth() + padding;

		mDownButton->setPosition(xp, (h * 0.5f) - (mDownButton->getScaleHeight() * 0.5f));
		xp += mDownButton->getScaleWidth() + padding;
	}
}

void PDFInterface::updateWidgets(){
	if(mPageCounter){
		std::wstringstream wss;
		if(mLinkedPDF){
			wss << mLinkedPDF->getPageNum() << " / " << mLinkedPDF->getPageCount();
		}
		mPageCounter->setText(wss.str());
	}
	layout();
}
} // namespace ui
} // namespace ds
