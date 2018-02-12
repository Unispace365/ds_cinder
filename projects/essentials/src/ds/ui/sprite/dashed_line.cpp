#include "stdafx.h"

#include "dashed_line.h"

#include <ds/ui/sprite/sprite_engine.h>


namespace ds {
namespace ui {

DashedLine::DashedLine(ds::ui::SpriteEngine& eng, const float lineWidth, const float lineLength, const float dashInc, const float spaceInc)
	: ds::ui::Sprite(eng, lineWidth, lineLength)
	, mDashLength(dashInc)
	, mSpaceIncrement(spaceInc)
{

	setTransparent(false);

	rebuildLine();
}

void DashedLine::setDashLength(const float dashLength) {
	mDashLength = dashLength;
	rebuildLine();
}

void DashedLine::setSpaceIncrement(const float spaceIncrement) {
	mSpaceIncrement = spaceIncrement;
	rebuildLine();
}

void DashedLine::onSizeChanged() {
	rebuildLine();
}

void DashedLine::rebuildLine() {
	mSegments.clear();

	ci::vec2 point = ci::vec2(0.0f, 0.0f);

	while(point.y + mDashLength < getHeight()) {
		ci::Path2d dash;
		dash.moveTo(ci::vec2(point.x, point.y));
		dash.lineTo(ci::vec2(point.x, point.y + mDashLength));
		mSegments.push_back(dash);
		point += ci::vec2(0.0f, mDashLength + mSpaceIncrement);
	}
}

void DashedLine::drawLocalServer() {
	drawLocalClient();
}

void DashedLine::drawLocalClient() {
	ci::gl::ScopedLineWidth slw(mWidth);
	for(auto seg : mSegments) {
		ci::gl::draw(seg);
	}	
}

} // namespace ui
} // namespace ds

