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

protected:

	virtual void						onSizeChanged();
	ds::ui::Pdf*						mPDF;
	PDFInterface*						mPdfInterface;
	bool								mEmbedInterface;

};

} // namespace ui
} // namespace ds

#endif
