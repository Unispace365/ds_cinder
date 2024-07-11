#pragma once

#include <ds/ui/button/layout_button.h>
#include <ds/ui/drawing/drawing_canvas.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {
class DrawingArea;

/**
 * \class waffles::DrawingTools
 *			 The tools part of the drawing
 */
class DrawingTools : public ds::ui::SmartLayout {
  public:
	DrawingTools(ds::ui::SpriteEngine& g, DrawingArea*);

	const float getControlHeight();

	void saveComplete(const bool success);


  protected:
	void updateSaveButton();

	virtual void onSizeChanged();
	void		 layout();

	ci::fs::path getSaveFilePath(const ci::fs::path& initialPath);
	void		 saveDrawing(const std::string& localPath);


	static const int TOOL_TYPE_PEN		   = 0;
	static const int TOOL_TYPE_HIGHLIGHTER = 1;
	static const int TOOL_TYPE_ERASE	   = 2;

	ds::ui::LayoutButton* configureToolButton(const std::string& buttonName, const int toolType);
	void				  toolButtonTapped(ds::ui::LayoutButton* toolButton, const int toolType);
	ds::ui::LayoutButton* configureBrushColorButton(const std::string& buttonName);
	void				  colorButtonTapped(ds::ui::LayoutButton* swatchButton, ci::Color theSwatch);
	ds::ui::LayoutButton* configureBrushSizeButton(const std::string& buttonName, const float brushSize);
	void				  sizeButtonTapped(ds::ui::LayoutButton* swatchButton, const float brushSize);


	std::vector<ds::ui::LayoutButton*> mToolButtons;
	std::vector<ds::ui::LayoutButton*> mBrushColorButtons;
	std::vector<ds::ui::LayoutButton*> mSizeButtons;

	DrawingArea*		   mDrawingArea;
	ds::ui::DrawingCanvas* mDrawingCanvas;
};

} // namespace waffles
