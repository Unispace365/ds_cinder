#pragma once

#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/drawing/drawing_canvas.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/text.h>

#include <ds/app/event_client.h>

namespace waffles {
class DrawingTools;

struct Mark {
	Mark(std::vector<std::pair<ci::vec2, ci::vec2>> points, ci::ColorA color, float penSize, bool erase)
		: mPoints(points)
		, mColor(color)
		, mPenSize(penSize)
		, mErase(erase) {}

	std::vector<std::pair<ci::vec2, ci::vec2>> mPoints;

	ci::ColorA mColor;
	float	   mPenSize;
	bool	   mErase;
};

struct Action {
	Action(std::string type, Mark thing)
		: mType(type)
		, mMark(thing) {}

	std::string mType;
	Mark		mMark;
};

/**
 * \class waffles::DrawingArea
 *			 Load the drawing tools and link it to a canvas
 */
class DrawingArea : public ds::ui::Sprite {
  public:
	DrawingArea(ds::ui::SpriteEngine& g, const float widdy, const float hiddy);

	const float getControlHeight();

	void saveDrawing(const std::string& localSavePath);

	ds::ui::DrawingCanvas* getDrawingCanvas() { return mDrawingCanvas; }

	void saveComplete(const bool success);

	DrawingTools* getDrawingTools() { return mDrawingTools; }
	void		  setToolsEmbedded(const bool embed);

	void undoMark();
	void redoMark();
	void clearFutureHistory();
	void clearAllDrawing();
	virtual void onParentSet() override;

  protected:
	virtual void onSizeChanged() override;
	void		 layout();
	void		 drawAll();
	void		 renderHistory(std::vector<Mark>& actions);

	ds::EventClient mEventClient;

	// the place where the drawing happens
	ds::ui::DrawingCanvas* mDrawingCanvas;
	DrawingTools*		   mDrawingTools;
	bool				   mToolsEmbedded;

	std::vector<Action> mActions;
	int					mCurrentAction = 0;

	int mRequestId;
};

} // namespace waffles
