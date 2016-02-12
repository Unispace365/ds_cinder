#pragma once
#ifndef DS_UI_MEDIA_VIEWER_PDF_PLAYER
#define DS_UI_MEDIA_VIEWER_PDF_PLAYER

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {
class Pdf;
class MediaInterface;
class PDFInterface;

/**
* \class ds::ui::PDFPlayer
*			Shows a scrollable PDF and puts an interface on top of it.
*/
class PDFPlayer : public ds::ui::Sprite  {
public:
	PDFPlayer(ds::ui::SpriteEngine& eng, bool embedInterface = true);

	void								setMedia(const std::string mediaPath);

	void								layout();

	void								showInterface();

	ds::ui::Pdf*						getPDF();

	void								nextPage();
	void								prevPage();

	void								setGoodStatusCallback(std::function<void()> func){ mGoodStatusCallback = func; }
	void								setErrorCallback(std::function<void(const std::string&)> func){ mErrorMsgCallback = func; }

protected:

	virtual void						onSizeChanged();
	ds::ui::Pdf*						mPDF;
	ds::ui::Sprite*						mPDFThumbHolder;
	ds::ui::Pdf*						mPDFThumb;
	PDFInterface*						mPdfInterface;
	bool								mEmbedInterface;
	std::function<void(void)>			mGoodStatusCallback;
	std::function<void(const std::string&)>			mErrorMsgCallback;

};

} // namespace ui
} // namespace ds

#endif
