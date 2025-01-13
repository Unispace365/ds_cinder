#pragma once
#ifndef DS_UI_MEDIA_VIEWER_ONELINE_PDF_INTERFACE
#define DS_UI_MEDIA_VIEWER_ONELINE_PDF_INTERFACE

#include <ds/data/resource.h>

#include "ds/ui/media/interface/pdf_interface.h"

namespace ds::ui {

class ImageButton;
class Text;
class IPdf;
class ThumbnailBar;
class VideoScrubBar;

/**
 * \class OnelinePDFInterface
 *			Implements page up/down, page count
 *			Note: for PDF thumbnail viewer to show up, the PDF needs to be loaded via a Resource
 *					that has a children vector of resources of the thumbnails set, and the children need to have the
 *correct parentIndex (i.e. page number) set.
 */
class OnelinePDFInterface : public PDFInterface {
  public:
	OnelinePDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
				 const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void updateWidgets() override;
	virtual void onLayout() override;

	
};

} // namespace ds::ui

#endif
