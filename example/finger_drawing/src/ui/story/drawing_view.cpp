#include "drawing_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

namespace example {

DrawingView::DrawingView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
	, mPrimaryLayout(nullptr)
	, mDrawingCanvas(nullptr)
	, mDrawingHolder(nullptr)
	, mBackground(nullptr)

{

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/drawing_view.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	mMessage = dynamic_cast<ds::ui::Text*>(spriteMap["message"]);
	mDrawingHolder = spriteMap["drawing_canvas_holder"];
	mBackground = spriteMap["background"];
	if(mDrawingHolder){
		mDrawingCanvas = new ds::ui::DrawingCanvas(mEngine, "%APP%/data/images/drawing/fuzzy.png");
		mDrawingHolder->addChildPtr(mDrawingCanvas);
	}

	// Set the default color
	auto reddy = configureBrushColorButton("brush_red", spriteMap, true);
	if(mDrawingCanvas && reddy){
		auto swatchy = reddy->getFirstDescendantWithName(L"brush_red.swatch");
		if(swatchy){
			mDrawingCanvas->setBrushColor(swatchy->getColor());
			reddy->showDown();
			reddy->enable(false);
		}
	}
	
	configureBrushColorButton("brush_pink", spriteMap, true);
	configureBrushColorButton("brush_orange", spriteMap, true);
	configureBrushColorButton("brush_yellow", spriteMap, true);
	configureBrushColorButton("brush_green", spriteMap, true);
	configureBrushColorButton("brush_blue", spriteMap, true);
	configureBrushColorButton("brush_black", spriteMap, true);
	configureBrushColorButton("brush_grey", spriteMap, true);
	configureBrushColorButton("brush_white", spriteMap, true);
	configureBrushColorButton("brush_erase", spriteMap, true, true);

	configureBrushSizeButton("small", spriteMap, 6);
	auto medButt = configureBrushSizeButton("med", spriteMap, 12);
	configureBrushSizeButton("large", spriteMap, 24);
	configureBrushSizeButton("xlarge", spriteMap, 50);
	configureBrushSizeButton("xxlarge", spriteMap, 120);

	if(medButt && mDrawingCanvas){
		medButt->enable(false);
		medButt->showDown();
		mDrawingCanvas->setBrushSize(12);
	}

	configureBrushColorButton("background_pink", spriteMap, false);
	configureBrushColorButton("background_red", spriteMap, false);
	configureBrushColorButton("background_orange", spriteMap, false);
	configureBrushColorButton("background_yellow", spriteMap, false);
	configureBrushColorButton("background_green", spriteMap, false);
	configureBrushColorButton("background_blue", spriteMap, false);
	configureBrushColorButton("background_grey", spriteMap, false);
	configureBrushColorButton("background_white", spriteMap, false);

	auto blackground = configureBrushColorButton("background_black", spriteMap, false);
	if(mBackground && blackground){
		auto swatchy = blackground->getFirstDescendantWithName(L"background_black.swatch");
		if(swatchy){
			mBackground->setColor(swatchy->getColor());
			blackground->showDown();
			blackground->enable(false);
		}
	}

	auto clearButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["clear_button.the_button"]);
	if(clearButton){
		clearButton->setClickFn([this]{
			if(mDrawingCanvas) mDrawingCanvas->clearCanvas();
		});
	}

	layout();
	animateOn();

}

ds::ui::LayoutButton* DrawingView::configureBrushColorButton(const std::string& buttonName, std::map<std::string, ds::ui::Sprite*> spriteMap, const bool isBrush, const bool isErase){

	std::string fullName = buttonName + ".the_button";
	std::string colorName = buttonName + ".swatch";
	auto swatchButton = dynamic_cast<ds::ui::LayoutButton*>(spriteMap[fullName]);
	if(swatchButton){
		auto theSwatch = dynamic_cast<ds::ui::Sprite*>(spriteMap[colorName]);
		swatchButton->setClickFn([this, swatchButton, theSwatch, isBrush, isErase]{
			if(mDrawingCanvas && theSwatch && mBackground){
				if(isBrush){
					for(auto it : mBrushColorButtons){
						it->showUp();
						it->enable(true);
					}
					if(isErase){
						mDrawingCanvas->setEraseMode(true);
					} else {
						mDrawingCanvas->setEraseMode(false);
						mDrawingCanvas->setBrushColor(theSwatch->getColor());
					}
				} else {
					for(auto it : mBackgroundColorButtons){
						it->showUp();
						it->enable(true);
					}
					mBackground->tweenColor(theSwatch->getColor());
				}

				swatchButton->enable(false);
				swatchButton->showDown();
			}
		});

		if(isBrush){
			mBrushColorButtons.push_back(swatchButton);
		} else {
			mBackgroundColorButtons.push_back(swatchButton);
		}

		return swatchButton;
		
	} else {
		DS_LOG_WARNING("Couldn't find a brush or background color button for " << buttonName);
	}

	return nullptr;
}
ds::ui::LayoutButton* DrawingView::configureBrushSizeButton(const std::string& buttonName, std::map<std::string, ds::ui::Sprite*> spriteMap, const float brushSize){

	std::string fullName = buttonName + ".the_button";
	std::string theSize = buttonName + ".sizer";
	auto swatchButton = dynamic_cast<ds::ui::LayoutButton*>(spriteMap[fullName]);
	if(swatchButton){
		auto theSwatch = dynamic_cast<ds::ui::Sprite*>(spriteMap[theSize]);
		swatchButton->setClickFn([this, swatchButton, theSwatch, brushSize]{
			if(mDrawingCanvas && theSwatch){
				for(auto it : mSizeButtons){
					it->showUp();
					it->enable(true);
				}

				mDrawingCanvas->setBrushSize(brushSize);

				swatchButton->enable(false);
				swatchButton->showDown();
			}
		});

		mSizeButtons.push_back(swatchButton);

		return swatchButton;

	} else {
		DS_LOG_WARNING("Couldn't find a brush or background color button for " << buttonName);
	}

	return nullptr;

}

void DrawingView::onAppEvent(const ds::Event& in_e){
}


void DrawingView::layout(){
	if(mPrimaryLayout){
		mPrimaryLayout->runLayout();
	}

	if(mDrawingCanvas && mDrawingHolder){
		mDrawingCanvas->setSizeAll(mDrawingHolder->getSize());
	}
}

void DrawingView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void DrawingView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void DrawingView::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	// any changes for this frame happen here
}


} // namespace example
