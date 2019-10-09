#pragma once
#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace pdf {

struct PdfLinkInfo {
	std::string mRawUri;
	std::string mUrl;
	int			mPageDest = -1;
	ci::Rectf	mRect;
};

/**
* \class ds::pdf::PdfLink
*			A sprite that represents a link inside a PDF. When tapped, changes the page of the pdf or opens a url
*/
class PdfLink : public ds::ui::Sprite {
public:
	PdfLink(ds::ui::SpriteEngine& eng, PdfLinkInfo info);

	PdfLinkInfo mInfo;
};

} // namespace pdf
} // namespace ds


