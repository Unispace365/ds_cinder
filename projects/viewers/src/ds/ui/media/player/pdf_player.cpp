#include "pdf_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/pdf.h>

#include "ds/ui/media/interface/pdf_interface.h"

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

	if(!mPdfInterface){
		mPdfInterface = new PDFInterface(mEngine, ci::Vec2f(400.0f, 50.0f), 25.0f, ci::Color::white(), ci::Color::black());
	}

	if(mEmbedInterface){
		addChildPtr(mPdfInterface);
		mPdfInterface->sendToFront();
	}

	if(mPdfInterface && mPDF){
		mPdfInterface->linkPDF(mPDF);
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

	if(mPdfInterface && mEmbedInterface){
		mPdfInterface->setPosition(getWidth() / 2.0f - mPdfInterface->getWidth() / 2.0f, getHeight() - mPdfInterface->getHeight() - 50.0f);
	}
}

void PDFPlayer::showInterface(){
	if(mPdfInterface){
		mPdfInterface->animateOn();
	}
}

MediaInterface* PDFPlayer::getExternalInterface(){
	return (mEmbedInterface ? nullptr : mPdfInterface);
}
} // namespace ui
} // namespace ds
