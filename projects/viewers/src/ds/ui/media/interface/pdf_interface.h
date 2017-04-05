#pragma once
#ifndef DS_UI_MEDIA_VIEWER_PDF_INTERFACE
#define DS_UI_MEDIA_VIEWER_PDF_INTERFACE

#include <ds/data/resource.h>
#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class Pdf;
class ThumbnailBar;

/**
* \class ds::ui::PDFInterface
*			Implements page up/down, page count
*			Note: for PDF thumbnail viewer to show up, the PDF needs to be loaded via a Resource
*					that has a children vector of resources of the thumbnails set, and the children need to have the correct parentIndex (i.e. page number) set.
*/
class PDFInterface : public MediaInterface  {
public:
	PDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	void						linkPDF(ds::ui::Pdf* linkedPDF, const ds::Resource& sourceResource);
	void						updateWidgets();

protected:
	virtual void				onUpdateServer(const ds::UpdateParams& updateParams) override;
	virtual void				onLayout();

	ds::ui::Pdf*				mLinkedPDF;
	ds::Resource				mSourceResource;
	bool						mLinkedEnabled;

	ds::ui::ImageButton*		mUpButton;
	ds::ui::ImageButton*		mDownButton;
	ds::ui::Text*				mPageCounter;
	ds::ui::ImageButton*		mTouchToggle;
	ds::ui::ImageButton*		mThumbsButton;

	ds::ui::ThumbnailBar*		mThumbnailBar;
	bool						mShowingThumbs;

};

} // namespace ui
} // namespace ds

#endif
