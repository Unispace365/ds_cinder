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
	, mEmbedInterface(embedInterface)
{
	setTransparent(false);
	setColor(ci::Color(0.3f, 0.5f, 0.0f));
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
	addChildPtr(mPDF);

	if(mPdfInterface){
		mPdfInterface->release();
		mPdfInterface = nullptr;
	}

	if(mEmbedInterface){
		mPdfInterface = dynamic_cast<PDFInterface*>(MediaInterfaceBuilder::buildMediaInterface(mEngine, this, this));

		if(mPdfInterface){
			mPdfInterface->sendToFront();
			mPdfInterface->hide();
		}

	}  

	setSize(mPDF->getWidth(), mPDF->getHeight());
}

void PDFPlayer::onSizeChanged(){
	layout();
}

void PDFPlayer::layout(){
	if(mPDF){
		// make the PDF content fill the vertical space perfectly
		float scale = this->getHeight() / mPDF->getHeight();
		mPDF->setScale(scale);
	}

	if(mPdfInterface){
		mPdfInterface->setSize(getWidth() * 2.0f / 3.0f, mPdfInterface->getHeight());
		mPdfInterface->setPosition(getWidth() / 2.0f - mPdfInterface->getWidth() / 2.0f, getHeight() - mPdfInterface->getHeight() - 50.0f);
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

} // namespace ui
} // namespace ds
