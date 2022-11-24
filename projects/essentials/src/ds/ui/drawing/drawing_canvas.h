#pragma once
#ifndef DS_UI_DRAWING_DRAWING_CANVAS
#define DS_UI_DRAWING_DRAWING_CANVAS

#include <deque>
#include <utility>

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {

	/**
	 * \class DrawingCanvas
	 *			A view that you can touch to draw on
	 */
	class DrawingCanvas : public ds::ui::Sprite {

	  public:
		DrawingCanvas(ds::ui::SpriteEngine& eng, const std::string& brushImagePath = "");

		void setRenderLineCallback(std::function<void(std::pair<ci::vec2, ci::vec2>)> func) {
			mRenderLineCallback = func;
		}
		void setCompleteLineCallback(std::function<void(std::vector<std::pair<ci::vec2, ci::vec2>> line)> func) {
			mCompleteLineCallback = func;
		}
		void setTouchHoldCallback(std::function<void(const ci::vec3& startPosition)> func) {
			mTouchHoldCallback = func;
		}

		/// The color and opacity are mixed together, though these setters may overwrite others' settings
		void			  setBrushColor(const ci::ColorA& brushColor);
		void			  setBrushColor(const ci::Color& brushColor);
		void			  setBrushOpacity(const float brushOpacity);
		const ci::ColorA& getBrushColor();

		/// Resizes the brush image to this pixel width and scales the height proportionally
		void		setBrushSize(const float brushSize);
		const float getBrushSize();

		/// Draws a line from start to end with an instance of the brush texture at every pixel
		void renderLine(const ci::vec3& start, const ci::vec3& end);

		/// Loads an image file to use for the brush
		void		setBrushImage(const std::string& imagePath);
		std::string getBrushImagePath() { return mBrushImagePath; }

		/// Clears any drawings a person has made
		void clearCanvas();

		/// If true, will erase instead of drawing
		void setEraseMode(const bool eraseMode);
		bool getEraseMode();

		/// Saves the canvas drawing to a file
		void saveCanvasImage(const std::string& filePath);

		/// Loads the canvas drawing from a file
		void loadCanvasImage(const std::string& filePath);

		// Static Client/Server Blob registration
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);

		// For custom external drawing - use with caution
		ci::gl::Texture2dRef  getBrushImageTexture();
		ci::gl::FboRef		  getDrawingFbo();
		ds::ui::SpriteShader& getPointShader() { return mPointShader; }

	  protected:
		// Queue to store points as they're drawn.  This gets serialized to clients.
		typedef std::deque<std::pair<ci::vec2, ci::vec2>> PointsQueue;
		PointsQueue										  mSerializedPointsQueue;

		std::vector<std::pair<ci::vec2, ci::vec2>>							 mCurrentLine;
		std::function<void(std::pair<ci::vec2, ci::vec2> line)>				 mRenderLineCallback;
		std::function<void(std::vector<std::pair<ci::vec2, ci::vec2>> line)> mCompleteLineCallback;

		std::function<void(const ci::vec3& startPos)> mTouchHoldCallback;
		double										  mTouchHoldStartTime;
		ci::vec3									  mTouchHoldStartPos;
		bool										  mTouchHolding = false;
		virtual void								  onUpdateServer(const ds::UpdateParams& updateParams);

		virtual void drawLocalClient() override;
		void		 createFbo();
		virtual void writeAttributesTo(DataBuffer&) override;
		virtual void readAttributeFrom(const char, DataBuffer&) override;

		ds::ui::Image mCanvasFileLoaderClient;

	  private:
		// The shader that colorizes the brush image
		ds::ui::SpriteShader mPointShader;
		// The intermediate fbo that brushes are drawn to
		ci::gl::FboRef mFbo;

		/// Only for the getter, the actual brush image is loaded via the image loading API
		std::string	   mBrushImagePath;
		ds::ui::Image* mBrushImage;

		float	   mBrushSize;
		ci::ColorA mBrushColor;
		bool	   mEraseMode;
	};

}} // namespace ds::ui

#endif
