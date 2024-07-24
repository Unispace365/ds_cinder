#include "stdafx.h"

#include "drawing_tools.h"

#include <CommDlg.h>
#include <Windows.h>

#include <cinder/ImageIo.h>
#include <cinder/ip/Flip.h>

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Timestamp.h>

#include <ds/app/environment.h>
#include <ds/content/content_events.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>
#include <waffles/util/waffles_helper.h>

#include "app/app_defs.h"
//#include "app/helpers.h"
#include "drawing_area.h"


namespace waffles {

DrawingTools::DrawingTools(ds::ui::SpriteEngine& g, DrawingArea* area)
	: ds::ui::SmartLayout(g, "waffles/drawing/drawing_control_panel.xml")
	, mDrawingArea(area)
	, mDrawingCanvas(area->getDrawingCanvas()) {

	if (!mDrawingCanvas) {
		DS_LOG_WARNING("Can't setup drawing tools without a drawing canvas!");
		return;
	}

	auto penny = configureToolButton("brush_pen", TOOL_TYPE_PEN);
	configureToolButton("brush_highlighter", TOOL_TYPE_HIGHLIGHTER);
	configureToolButton("brush_erase", TOOL_TYPE_ERASE);

	if (penny) {
		toolButtonTapped(penny, TOOL_TYPE_PEN);
	}

	for (int i = 1; i < 13; i++) {
		auto reddy = configureBrushColorButton("brush_" + std::to_string(i));
		if (i == 1) {
			if (reddy) {
				auto swatchy = reddy->getFirstDescendantWithName(L"brush_1.swatch");
				if (swatchy) {
					mDrawingCanvas->setBrushColor(swatchy->getColor());
					reddy->showDown();
					reddy->enable(false);
				}
			} else {
				DS_LOG_WARNING("whoops, didn't find a default color for drawing.");
			}
		}
	}


	/// The default button can be small or medium, depending on what's in the layout file
	float smSize  = mEngine.getAppSettings().getFloat("drawing:brush_size:small", 0, 10.0f);
	float medSize = mEngine.getAppSettings().getFloat("drawing:brush_size:med", 0, 40.0f);
	auto  smButt  = configureBrushSizeButton("small", smSize);
	auto  medButt = configureBrushSizeButton("med", medSize);
	configureBrushSizeButton("large", mEngine.getAppSettings().getFloat("drawing:brush_size:large", 0, 80.0f));
	configureBrushSizeButton("xlarge", mEngine.getAppSettings().getFloat("drawing:brush_size:xlarge", 0, 150.0f));
	configureBrushSizeButton("xxlarge", mEngine.getAppSettings().getFloat("drawing:brush_size:xxlarge", 0, 300.0f));

	ds::ui::LayoutButton* defaultSizeBut = smButt;
	mDrawingCanvas->setBrushSize(smSize);
	if (!smButt) {
		defaultSizeBut = medButt;
		mDrawingCanvas->setBrushSize(medSize);
	}

	if (defaultSizeBut) {
		defaultSizeBut->enable(false);
		defaultSizeBut->showDown();
	} else {
		DS_LOG_WARNING("Didn't find the default medium size brush size swatch");
	}

	auto clearButton = getSprite<ds::ui::LayoutButton>("clear_button.the_button");
	if (clearButton) {
		clearButton->setClickFn([this, penny] {
			if (mDrawingCanvas) mDrawingCanvas->clearCanvas();
			if (mDrawingArea) {
				mDrawingArea->clearAllDrawing();
			}

			if (penny) {
				toolButtonTapped(penny, TOOL_TYPE_PEN);
			}
		});
	}

	setSpriteClickFn("redo_button.the_button", [this] {
		if (mDrawingArea) mDrawingArea->redoMark();
	});
	setSpriteClickFn("undo_button.the_button", [this] {
		if (mDrawingArea) mDrawingArea->undoMark();
	});

	setSpriteClickFn("save_button.the_button", [this] { saveDrawing(""); });

	setSpriteClickFn("save_as_button.the_button", [this] {
		ci::fs::path fp = getSaveFilePath(ds::Environment::expand("%DOCUMENTS%"));
		if (!fp.empty()) {
			std::string fpString = fp.string();
			if (fpString.find(".png") != fpString.size() - 4) {
				fpString.append(".png");
			}
			callAfterDelay([this, fpString] { saveDrawing(fpString); }, 0.1f);
		}
	});


	updateSaveButton();
	listenToEvents<ds::ScheduleUpdatedEvent>([this](auto& e) { updateSaveButton(); });
}


void DrawingTools::updateSaveButton() {
	auto helper = WafflesHelperFactory::getDefault();
	if (auto saveBtn = getSprite("save_button.the_button")) {
		if (helper->getAnnotationFolder().empty()) {
			saveBtn->hide();
		} else {
			saveBtn->show();
		}
	}
	layout();
}


ci::fs::path DrawingTools::getSaveFilePath(const ci::fs::path& initialPath) {
	OPENFILENAMEW ofn;				// common dialog box structure
	wchar_t		  szFile[MAX_PATH]; // buffer for file name
	wchar_t		  initialPathStr[MAX_PATH];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	auto app		= ci::app::AppBase::get();
	if (app && app->getRenderer())
		ofn.hwndOwner = app->getRenderer()->getHwnd();
	else
		ofn.hwndOwner = 0;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetSaveFileName does not
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0]   = '\0';
	ofn.nMaxFile	   = sizeof(szFile);
	ofn.lpstrFilter	   = L"PNG Image\0*.png\0All files\0*.*\0\0";
	ofn.nFilterIndex   = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle  = 0;
	ofn.lpstrTitle	   = L"Save As...";
	ofn.lpstrDefExt	   = L"png";
	if (initialPath.empty()) {
		ofn.lpstrInitialDir = NULL;
	} else if (ci::fs::is_directory(initialPath)) {
		wcscpy_s(initialPathStr, initialPath.wstring().c_str());
		ofn.lpstrInitialDir = initialPathStr;
	} else {
		wcscpy_s(initialPathStr, initialPath.parent_path().wstring().c_str());
		ofn.lpstrInitialDir = initialPathStr;
		wcscpy_s(szFile, initialPath.filename().wstring().c_str());
	}
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_DONTADDTORECENT;

	// Display the Open dialog box.
	if (::GetSaveFileName(&ofn) == TRUE)
		return ci::fs::path(ofn.lpstrFile);
	else
		return ci::fs::path();
}

const float DrawingTools::getControlHeight() {
	return getHeight();
}

void DrawingTools::saveDrawing(const std::string& localPath) {

	if (mDrawingArea) {
		mDrawingArea->saveDrawing(localPath);
	}

	auto saveButton = getSprite<ds::ui::LayoutButton>("save_button.the_button");
	auto saveTitle	= getSprite<ds::ui::Text>("save_button.label_high");
	if (saveButton) {
		saveButton->enable(false);
		saveButton->showDown();

		if (saveTitle) {
			saveTitle->setText("Saving...");
			saveButton->runLayout();
		}
	}
}
void DrawingTools::saveComplete(const bool success) {
	auto saveButton = getSprite<ds::ui::LayoutButton>("save_button.the_button");
	auto saveTitleH = getSprite<ds::ui::Text>("save_button.label_high");
	auto saveTitleN = getSprite<ds::ui::Text>("save_button.label_normal");
	if (saveButton && saveTitleH && saveTitleN) {
		saveButton->enable(true);
		saveButton->showUp();
		if (!success) {
			saveTitleH->setText("Error"); // should probably be more descriptive
			saveTitleN->setText("Error"); // should probably be more descriptive
		} else {
			saveTitleH->setText("Saved");
			saveTitleN->setText("Saved");
			saveTitleH->callAfterDelay(
				[saveButton, saveTitleH, saveTitleN] {
					saveTitleH->setText("Save");
					saveTitleN->setText("Save");
					saveButton->runLayout();
				},
				5.0f);
		}
		saveButton->runLayout();
	}
}

void DrawingTools::onSizeChanged() {
	layout();
}

void DrawingTools::layout() {
	runLayout();
}

ds::ui::LayoutButton* DrawingTools::configureToolButton(const std::string& buttonName, const int toolType) {
	std::string fullName   = buttonName + ".the_button";
	auto		toolButton = getSprite<ds::ui::LayoutButton>(fullName);
	if (toolButton) {
		toolButton->setClickFn([this, toolButton, toolType] { toolButtonTapped(toolButton, toolType); });

		mToolButtons.push_back(toolButton);

		return toolButton;

	} else {
		DS_LOG_WARNING("Couldn't find a tool button for " << buttonName);
	}

	return nullptr;
}

void DrawingTools::toolButtonTapped(ds::ui::LayoutButton* swatchButton, const int toolType) {
	if (mDrawingCanvas && swatchButton) {
		for (auto it : mToolButtons) {
			it->showUp();
			it->enable(true);
		}
		if (toolType == TOOL_TYPE_PEN) {
			mDrawingCanvas->setBrushOpacity(1.0f);
			mDrawingCanvas->setEraseMode(false);

		} else if (toolType == TOOL_TYPE_HIGHLIGHTER) {
			mDrawingCanvas->setBrushOpacity(0.015f);
			mDrawingCanvas->setEraseMode(false);

		} else if (toolType == TOOL_TYPE_ERASE) {
			mDrawingCanvas->setBrushOpacity(1.0f);
			mDrawingCanvas->setEraseMode(true);
		}

		swatchButton->enable(false);
		swatchButton->showDown();
	}
}

ds::ui::LayoutButton* DrawingTools::configureBrushColorButton(const std::string& buttonName) {

	std::string fullName	 = buttonName + ".the_button";
	std::string colorName	 = buttonName + ".swatch";
	auto		swatchButton = getSprite<ds::ui::LayoutButton>(fullName);
	if (swatchButton) {
		auto theSwatch = getSprite<ds::ui::Sprite>(colorName);
		swatchButton->setClickFn(
			[this, swatchButton, theSwatch] { colorButtonTapped(swatchButton, theSwatch->getColor()); });

		mBrushColorButtons.push_back(swatchButton);

		return swatchButton;

	} else {
		DS_LOG_WARNING("Couldn't find a brush or background color button for " << buttonName);
	}

	return nullptr;
}

void DrawingTools::colorButtonTapped(ds::ui::LayoutButton* swatchButton, ci::Color theSwatch) {
	if (mDrawingCanvas) {
		for (auto it : mBrushColorButtons) {
			it->showUp();
			it->enable(true);
		}

		mDrawingCanvas->setBrushColor(theSwatch);

		swatchButton->enable(false);
		swatchButton->showDown();
	}
}

ds::ui::LayoutButton* DrawingTools::configureBrushSizeButton(const std::string& buttonName, const float brushSize) {

	std::string fullName	 = buttonName + ".the_button";
	std::string theSize		 = buttonName + ".sizer";
	auto		swatchButton = getSprite<ds::ui::LayoutButton>(fullName);
	if (swatchButton) {
		swatchButton->setClickFn([this, swatchButton, brushSize] { sizeButtonTapped(swatchButton, brushSize); });

		mSizeButtons.push_back(swatchButton);

		return swatchButton;

	} else {
		DS_LOG_WARNING("Couldn't find a brush or background color button for " << buttonName);
	}

	return nullptr;
}

void DrawingTools::sizeButtonTapped(ds::ui::LayoutButton* swatchButton, const float brushSize) {
	if (mDrawingCanvas) {
		for (auto it : mSizeButtons) {
			it->showUp();
			it->enable(true);
		}

		mDrawingCanvas->setBrushSize(brushSize);

		swatchButton->enable(false);
		swatchButton->showDown();
	}
}

} // namespace waffles
