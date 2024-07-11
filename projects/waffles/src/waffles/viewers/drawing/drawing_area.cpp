#include "stdafx.h"

#include "drawing_area.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Timestamp.h>
#include <cinder/ImageIo.h>
#include <cinder/ip/Flip.h>

#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "drawing_tools.h"
#include "waffles/waffles_events.h"

namespace waffles {
static int NEXT_REQUEST = 0;

DrawingArea::DrawingArea(ds::ui::SpriteEngine& g, const float widdy, const float hiddy)
	: ds::ui::Sprite(g)
	, mEventClient(g)
	, mDrawingCanvas(nullptr)
	, mDrawingTools(nullptr)
	, mToolsEmbedded(true)
	, mRequestId(-1) {

	enable(false);

	mDrawingCanvas = new ds::ui::DrawingCanvas(mEngine, "%APP%/data/images/ui/Circle_256.png");
	addChildPtr(mDrawingCanvas);
	mDrawingCanvas->setBrushColor(ci::Color::white());
	mDrawingCanvas->setBrushSize(16.0f);
	mDrawingCanvas->setSize(widdy, hiddy);

	mDrawingCanvas->setCompleteLineCallback([this](std::vector<std::pair<ci::vec2, ci::vec2>> line) {
		clearFutureHistory();

		mActions.emplace_back(Action("mark", Mark(line, mDrawingCanvas->getBrushColor(), mDrawingCanvas->getBrushSize(),
												  mDrawingCanvas->getEraseMode())));
		mCurrentAction = (int)mActions.size();
	});

	mDrawingTools = new DrawingTools(mEngine, this);
	addChildPtr(mDrawingTools);

	setSize(widdy, hiddy);

	mEventClient.listenToEvents<DrawingSaveComplete>([this](auto& e) {
		if (e.mRequestId == mRequestId) {
			saveComplete(!e.mErrored);
		}
	});
}

const float DrawingArea::getControlHeight() {
	if (mDrawingTools && mToolsEmbedded) {
		return mDrawingTools->getHeight();
	}

	return 0.0f;
}


void DrawingArea::onSizeChanged() {
	layout();
}

void DrawingArea::layout() {
	if (mDrawingCanvas) {
		mDrawingCanvas->setScale(getWidth() / mDrawingCanvas->getWidth());
	}

	if (mDrawingTools && mToolsEmbedded) {
		mDrawingTools->setSize(getWidth(), mDrawingTools->getHeight());
		mDrawingTools->setPosition(0.0f, getHeight());
	}
}

void DrawingArea::drawAll() {
	auto currentColor = mDrawingCanvas->getBrushColor();
	auto currentSize  = mDrawingCanvas->getBrushSize();
	auto erase		  = mDrawingCanvas->getEraseMode();

	std::vector<Mark> effectiveMarks;

	int item = 0;
	for (auto mark : mActions) {
		item++;
		if (item > mCurrentAction) break;
		if (mark.mType == "clear") {
			effectiveMarks.clear();
		} else if (!mark.mMark.mPoints.empty()) {
			effectiveMarks.emplace_back(mark.mMark);
		}
	}

	if (mDrawingCanvas) mDrawingCanvas->clearCanvas();

	renderHistory(effectiveMarks);

	mDrawingCanvas->setBrushColor(currentColor);
	mDrawingCanvas->setBrushSize(currentSize);
	mDrawingCanvas->setEraseMode(erase);
}

void DrawingArea::renderHistory(std::vector<Mark>& actions) {
	if (!mDrawingCanvas) return;

	ci::gl::Texture2dRef brushTexture = mDrawingCanvas->getBrushImageTexture();
	if (!brushTexture || brushTexture->getWidth() < 1) {
		DS_LOG_WARNING("DrawingArea::renderHistory() no brush texture!");
		return;
	}


	auto theFbo = mDrawingCanvas->getDrawingFbo();
	if (!theFbo) {
		DS_LOG_WARNING("DrawingArea::renderHistory() no fbo!");
		return;
	}

	// int	 w			 = theFbo->getWidth();
	// int	 h			 = theFbo->getHeight();
	auto pointShader = mDrawingCanvas->getPointShader();

	{
		ci::gl::pushMatrices();
		ci::gl::ScopedFramebuffer fbScp(theFbo);
		ci::gl::ScopedViewport	  scpVp(ci::ivec2(0), theFbo->getSize());

		ci::CameraOrtho camera = ci::CameraOrtho(0.0f, static_cast<float>(theFbo->getWidth()),
												 static_cast<float>(theFbo->getHeight()), 0.0f, -1000.0f, 1000.0f);
		ci::gl::setMatrices(camera);
		brushTexture->bind(0);
		pointShader.getShader()->uniform("tex0", 0);
		ci::gl::ScopedGlslProg shaderScp(pointShader.getShader());

		for (auto mark : actions) {
			auto& markToDraw = mark;
			// auto  numPoints	 = markToDraw.mPoints.size();

			int	 blendSfactor = 0;
			auto drawColor	  = markToDraw.mColor;
			if (drawColor.a < 1.0f && drawColor.a > 0.0f) drawColor += 0.015f;
			pointShader.getShader()->uniform("vertexColor", drawColor);

			if (markToDraw.mErase) {
				blendSfactor = GL_ZERO;
			} else {
				blendSfactor = GL_ONE;
			}

			ci::gl::ScopedBlend enableBlend(true);
			ci::gl::ScopedBlend enableFunc(blendSfactor, GL_ONE_MINUS_SRC_ALPHA);

			float widdy = markToDraw.mPenSize;
			float hiddy = markToDraw.mPenSize;

			if (brushTexture) {
				brushTexture->setTopDown(true);
				hiddy = markToDraw.mPenSize / ((float)brushTexture->getWidth() / (float)brushTexture->getHeight());
			}

			float brushPixelStep = 3.0f;
			// int	  vertexCount	 = 0;

			std::vector<ci::vec2> drawPoints;

			for (auto lines : markToDraw.mPoints) {
				auto start = lines.first;
				auto end   = lines.second;
				int	 count = std::max<int>(
					 (int)ceilf(sqrtf((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y)) /
								(float)brushPixelStep),
					 1);
				for (int i = 0; i < count; ++i) {
					drawPoints.push_back(ci::vec2(start.x + (end.x - start.x) * ((float)i / (float)count),
												  start.y + (end.y - start.y) * ((float)i / (float)count)));
				}
			}

			for (auto it : drawPoints) {
				ci::Rectf destRect =
					ci::Rectf(it.x - widdy / 2.0f, it.y - hiddy / 2.0f, it.x + widdy / 2.0f, it.y + hiddy / 2.0f);
				ci::gl::drawSolidRect(destRect);
			}
		}

		ci::gl::popMatrices();
	}

	DS_REPORT_GL_ERRORS();
}

void DrawingArea::saveComplete(const bool success) {
	if (mDrawingTools) {
		mDrawingTools->saveComplete(success);
	}
}

void DrawingArea::setToolsEmbedded(const bool embed) {
	if (mToolsEmbedded == embed) return;
	mToolsEmbedded = embed;
	if (mToolsEmbedded) {
		if (mDrawingTools) {
			if (mDrawingTools->getParent() != this) {
				addChildPtr(mDrawingTools);
			}
			mDrawingTools->show();
			mDrawingTools->tweenOpacity(1.0f, mEngine.getAnimDur(), 0.0f);
			layout();
		}
	} else {
		layout();
	}
}

void DrawingArea::clearFutureHistory() {
	if (mCurrentAction == 0) {
		mActions.clear();
		return;
	}
	if (mCurrentAction < mActions.size()) {
		mActions.erase(mActions.begin() + mCurrentAction, mActions.end());
	}
}

void DrawingArea::clearAllDrawing() {
	if (!mDrawingCanvas) return;

	clearFutureHistory();

	mDrawingCanvas->clearCanvas();

	std::vector<std::pair<ci::vec2, ci::vec2>> line;
	line.emplace_back(std::make_pair(ci::vec2(), ci::vec2()));

	mActions.emplace_back(
		Action("clear", Mark(line, mDrawingCanvas->getBrushColor(), mDrawingCanvas->getBrushSize(), true)));
	mCurrentAction = (int)mActions.size();
}

void DrawingArea::undoMark() {
	auto curAction = mCurrentAction;
	mCurrentAction--;
	if (mCurrentAction < 0) mCurrentAction = 0;
	if (mCurrentAction != curAction) {
		drawAll();
	}
}

void DrawingArea::redoMark() {
	auto curAction = mCurrentAction;
	mCurrentAction++;
	if (mCurrentAction > mActions.size()) mCurrentAction = (int)mActions.size();
	if (mCurrentAction != curAction) {
		drawAll();
	}
}

void DrawingArea::saveDrawing(const std::string& localSavePath) {
	if (!getParent()) return;

	/// Tell anything to hide itself
	mEngine.getNotifier().notify(RequestPreDrawingSave());

	if (mDrawingTools) mDrawingTools->hide();
	float delayey = 0.05f;
	callAfterDelay(
		[this, localSavePath] {
			const auto temp	   = getBoundingBox();
			auto	   leftTop = localToGlobal(ci::vec3(temp.x1, temp.y1, 0));
			// float	   toolsHeight = 0;
			auto	  rightBottom = localToGlobal(ci::vec3(temp.x2, temp.y2, 0));
			auto	  dst		  = mEngine.getDstRect();
			auto	  src		  = mEngine.getSrcRect();
			float	  scale		  = (dst.x2 - dst.x1) / (src.x2 - src.x1);
			ci::Rectf area		  = ci::Rectf(leftTop.x, leftTop.y, rightBottom.x, rightBottom.y);

			ci::Surface s((int32_t)(area.getWidth() * scale), (int32_t)(area.getHeight() * scale), false);
			glFlush(); // there is some disagreement about whether this is necessary, but ideally performance-conscious
					   // users will use FBOs anyway

			GLint oldPackAlignment;
			glGetIntegerv(GL_PACK_ALIGNMENT, &oldPackAlignment);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels((GLint)area.x1 * scale, (GLint)(mEngine.getWorldHeight() - area.y2) * scale,
						 (GLint)area.getWidth() * scale, (GLint)area.getHeight() * scale, GL_RGB, GL_UNSIGNED_BYTE,
						 s.getData());
			glPixelStorei(GL_PACK_ALIGNMENT, oldPackAlignment);
			ci::ip::flipVertical(&s);


			mRequestId = NEXT_REQUEST++;

			mEngine.getNotifier().notify(RequestDrawingSave(s, mRequestId, localSavePath));
			if (mDrawingTools) mDrawingTools->show();
		},
		delayey);
}

} // namespace waffles
