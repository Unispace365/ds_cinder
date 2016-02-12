#include "pdf_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/pdf.h>

#include "ds/ui/media/interface/pdf_interface.h"
#include "ds/ui/media/media_interface_builder.h"

namespace ds {
namespace ui {

PDFPlayer::PDFPlayer(ds::ui::SpriteEngine& eng, bool embedInterface)
	: ds::ui::Sprite(eng)
	, mPDF(nullptr)
	, mPdfInterface(nullptr)
	, mPDFThumb(nullptr)
	, mPDFThumbHolder(nullptr)
	, mEmbedInterface(embedInterface)
{
//	setTransparent(false);
//	setColor(ci::Color(0.3f, 0.5f, 0.0f));

	mPDFThumbHolder = new ds::ui::Sprite(mEngine);
	addChildPtr(mPDFThumbHolder);
}

void PDFPlayer::setMedia(const std::string mediaPath){
	if(mPDF){
		mPDF->release();
		mPDF = nullptr;
		if(mPdfInterface){
			mPdfInterface->linkPDF(nullptr);
		}
	}


	mPDF = new ds::ui::Pdf(mEngine);
	mPDF->setResourceFilename(mediaPath);

	// When the page finishes loading, hide the low-res thumbnail version and advance it's page
	mPDF->setPageLoadedCallback([this]{
		if(mPDFThumb){
			mPDFThumb->setOpacity(0.0f);
			mPDFThumb->setPageNum(mPDF->getPageNum() + 1);
		}
		if(mGoodStatusCallback) mGoodStatusCallback();
	});

	mPDF->setLoadErrorCallback([this](const std::string& msg){
		if(mErrorMsgCallback) mErrorMsgCallback(msg);
	});
	addChildPtr(mPDF);

	if(mPDFThumb){
		mPDFThumb->release();
		mPDFThumb = nullptr;
	}
	if(mPDFThumbHolder){
		mPDFThumb = new ds::ui::Pdf(mEngine);
		mPDFThumb->setResourceFilename(mediaPath);
		mPDFThumb->setScale(0.1f);
		mPDFThumbHolder->addChildPtr(mPDFThumb);
		mPDFThumbHolder->sendToFront();
	}

	if(mPdfInterface){
		mPdfInterface->release();
		mPdfInterface = nullptr;
	}

	if(mEmbedInterface){
		mPdfInterface = dynamic_cast<PDFInterface*>(MediaInterfaceBuilder::buildMediaInterface(mEngine, this, this));

		if(mPdfInterface){
			mPdfInterface->sendToFront();
		}
	}  

	// This overrides the PDF interface page change callback
	if(mPDF){

		// Something requested the main PDF's page to change
		// Update the interface UI and show the thumbnail low-res size.
		// The thumbnail gets hidden when the main PDF finishes rendering (async)
		mPDF->setPageChangeCallback([this]{
			if(mPdfInterface){
				mPdfInterface->updateWidgets();
			}

			if(mPDFThumb){
				mPDFThumb->setPageNum(mPDF->getPageNum());
				mPDFThumb->setOpacity(1.0f);
			}
		});
	}

	setSize(mPDF->getWidth(), mPDF->getHeight());
}

void PDFPlayer::onSizeChanged(){
	layout();
}

void PDFPlayer::layout(){
	const float w = getWidth();
	const float h = getHeight();

	if(mPDFThumbHolder){
		float theScale = h / mPDFThumb->getHeight();
		mPDFThumbHolder->setScale(theScale * 10.0f);
	}

	if(mPDF){
		// make the PDF content fill the vertical space perfectly
		float theScale = h / mPDF->getHeight();
		mPDF->setScale(theScale);
	}

	if(mPdfInterface){
		mPdfInterface->setSize(w * 2.0f / 3.0f, mPdfInterface->getHeight());
		mPdfInterface->setPosition(w / 2.0f - mPdfInterface->getWidth() / 2.0f, h - mPdfInterface->getHeight() - 50.0f);
	}
}

ds::ui::Pdf* PDFPlayer::getPDF(){
	return mPDF;
}

void PDFPlayer::showInterface(){
	if(mPdfInterface){
		mPdfInterface->animateOn();
	}
}

void PDFPlayer::nextPage(){
	if(mPDF){
		mPDF->goToNextPage();
	}
}

void PDFPlayer::prevPage(){
	if(mPDF){
		mPDF->goToPreviousPage();
	}
}

} // namespace ui
} // namespace ds
