#include "stdafx.h"

#include "pdf_link.h"

namespace ds { namespace pdf {

	PdfLink::PdfLink(ds::ui::SpriteEngine& eng, PdfLinkInfo info)
	  : ds::ui::Sprite(eng)
	  , mInfo(info) {
		enable(true);
		enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		setTransparent(false);
		setColor(ci::Color(6.0f / 255.0f, 69.0f / 255.0f, 173.0f / 255.0f));
		setOpacity(0.1f);
	}

}} // namespace ds::pdf
