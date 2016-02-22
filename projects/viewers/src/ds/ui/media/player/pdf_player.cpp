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
	, mPDFNext(nullptr)
	, mPDFPrev(nullptr)
	, mPDFThumbHolder(nullptr)
	, mEmbedInterface(embedInterface)
	, mCurrentPage(-1)
	, mNextReady(false)
	, mPrevReady(false)
	, mFirstPageLoaded(false)
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

	// When the page finishes loading, hide the low-res thumbnail version and advance its page
	mPDF->setPageLoadedCallback([this, mediaPath]{
		if(!mFirstPageLoaded){
			if(mPDFNext){
				mPDFNext->setResourceFilename(mediaPath);
			}
			if(mPDFPrev){
				mPDFPrev->setResourceFilename(mediaPath);
			}
			layout();
		}
		mFirstPageLoaded = true;
		loadNextAndPrevPages();
		if(mGoodStatusCallback) mGoodStatusCallback();
	});

	mPDF->setLoadErrorCallback([this](const std::string& msg){
		if(mErrorMsgCallback) mErrorMsgCallback(msg);
	});
	addChildPtr(mPDF);

	if(mPDFNext){
		mPDFNext->release();
		mPDFNext = nullptr;
	}
	if(mPDFPrev){
		mPDFPrev->release();
		mPDFPrev = nullptr;
	}

	mFirstPageLoaded = false;

	// This loads 2 additional PDF sprites, one for the next page, and one for the previous page
	// The scale of the PDF stays at 0.5 (because of the thumb holder), so the quality is less than the proper PDF
	// When the page gets changed, and the next or previous PDFs are loaded, they're shown until the primary PDF finishes rendering
	if(mPDFThumbHolder && mPDF->getPageCount() > 1){
		mPDFNext = new ds::ui::Pdf(mEngine);

		//Next and prev pdf starts loading after the first proper page has finished, for speed
		//mPDFNext->setResourceFilename(mediaPath);
		//mPDFNext->setPageNum(2);
		mPDFNext->setScale(0.5f);
		mNextReady = false;
		mPDFNext->setPageChangeCallback([this]{
			mNextReady = false;
		});
		mPDFNext->setPageLoadedCallback([this]{
			mNextReady = true;
		});
		mPDFThumbHolder->addChildPtr(mPDFNext);

		mPDFPrev = new ds::ui::Pdf(mEngine);
		//mPDFPrev->setResourceFilename(mediaPath);
		//mPDFPrev->setPageNum(mPDF->getPageCount());
		mPDFPrev->setScale(0.5f);
		mPrevReady = false;
		mPDFPrev->setPageChangeCallback([this]{
			mPrevReady = false;
		});
		mPDFPrev->setPageLoadedCallback([this]{
			mPrevReady = true;
		});
		mPDFThumbHolder->addChildPtr(mPDFPrev);

		mPDFThumbHolder->sendToFront();
	}

	mCurrentPage = 0;

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

			// If we're going to a page we've already tried to load (the next or prev)
			if(mPDF->getPageNum() != mCurrentPage){
				if(mPDFNext && mPDF->getPageNum() == mPDFNext->getPageNum()){
					// We should be ready to go, if not, then start loading the next page and forget about this one
					if(mNextReady){
						mPDFNext->setOpacity(1.0f);
					} else {
						//loadNextAndPrevPages();
					}
				}
				if(mPDFPrev && mPDF->getPageNum() == mPDFPrev->getPageNum()){
					// We should be ready to go, if not, then start loading the prev page and forget about this one
					if(mPrevReady){
						mPDFPrev->setOpacity(1.0f);
					} else {
					//	loadNextAndPrevPages();
					}
				}

			
			}

			mCurrentPage = mPDF->getPageNum();

			if(mPdfInterface){
				mPdfInterface->updateWidgets();
			}

		});
	}

	setSize(mPDF->getWidth(), mPDF->getHeight());
}

void PDFPlayer::loadNextAndPrevPages(){
	if(!mPDF) return;
	const int curPageNum = mPDF->getPageNum();
	const int curCount = mPDF->getPageCount();
	if(mPDFNext){
		mPDFNext->setOpacity(0.0f);
		if(curPageNum == curCount){
			mPDFNext->setPageNum(0);
		} else {
			mPDFNext->setPageNum(curPageNum + 1);
		}
	}
	if(mPDFPrev){
		mPDFPrev->setOpacity(0.0f);
		if(curPageNum == 1){
			mPDFPrev->setPageNum(curCount);
		} else {
			mPDFPrev->setPageNum(curPageNum - 1);
		}
	}
}

void PDFPlayer::onSizeChanged(){
	layout();
}

void PDFPlayer::layout(){
	const float w = getWidth();
	const float h = getHeight();

	if(mPDFThumbHolder && mPDFNext){
		float theScale = h / mPDFNext->getHeight();
		mPDFThumbHolder->setScale(theScale * 2.0f);
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
