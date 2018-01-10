#pragma once
#ifndef DS_UI_SPRITE_DASHED_LINE_H_
#define DS_UI_SPRITE_DASHED_LINE_H_


#include <functional>

#include <cinder/Path2d.h>

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

class Globals;

/**
* \class bragg::DashedLine
*			Sprite to draw lines for map path!
*/
class DashedLine : public ds::ui::Sprite {
public:
	DashedLine(ds::ui::SpriteEngine& eng, const float lineWidth = 1.0f, const float lineLength = 20.0f, const float dashLength = 5.0f, const float spaceIncrement = 10.0f);

	void					setDashLength(const float dashLength);
	const float				getDashLength() { return mDashLength; }

	void					setSpaceIncrement(const float spaceIncrement);
	const float				setSpaceIncrement() { return mSpaceIncrement; }

protected:
	virtual void			onSizeChanged();
	void					rebuildLine();
	bool					mClosed;

	std::vector<ci::Path2d> mSegments;

	bool					mPointsAreDirty;
	float					mSpaceIncrement;
	float					mDashLength;


	virtual void			drawLocalClient();
	virtual void			drawLocalServer();
};

} // namespace ui
} // namespace ds

#endif
