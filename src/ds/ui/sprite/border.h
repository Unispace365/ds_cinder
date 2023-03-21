#pragma once
#ifndef DS_UI_BORDER_H
#define DS_UI_BORDER_H

#include "ds/ui/sprite/sprite.h"
#include <cinder/gl/Vbo.h>
#include <string>

namespace ds { namespace ui {

	class Border : public Sprite {
	  public:
		Border(SpriteEngine&);
		Border(SpriteEngine&, const float width);

		void		setBorderWidth(const float borderWidth);
		const float getBorderWidth() { return mBorderWidth; }

		virtual void drawLocalClient();

		/// Initialization
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);

	  protected:
		virtual void writeAttributesTo(ds::DataBuffer&);
		virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);
		virtual void onBuildRenderBatch();

	  private:
		float			 mBorderWidth;
		ci::gl::BatchRef mLeftBatch;
		ci::gl::BatchRef mRightBatch;
		ci::gl::BatchRef mBotBatch;
	};

}} // namespace ds::ui

#endif // DS_UI_BORDER_H
