#pragma once
#ifndef DS_UI_MEDIA_VIEWER_PDF_INTERFACE
#define DS_UI_MEDIA_VIEWER_PDF_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class Pdf;

/**
* \class ds::ui::PDFInterface
*			Implements page up/down, page count
*/
class PDFInterface : public MediaInterface  {
public:
	PDFInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	void						linkPDF(ds::ui::Pdf* linkedPDF);
	void						updateWidgets();

protected:

	virtual void				onLayout();

	ds::ui::Pdf*				mLinkedPDF;

	ds::ui::ImageButton*		mUpButton;
	ds::ui::ImageButton*		mDownButton;
	ds::ui::Text*				mPageCounter;

};

} // namespace ui
} // namespace ds

#endif
