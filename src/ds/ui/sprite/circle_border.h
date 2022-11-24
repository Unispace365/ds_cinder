#pragma once
#ifndef DS_UI_CIRCLE_BORDER_H
#define DS_UI_CIRCLE_BORDER_H

#include "ds/ui/sprite/sprite.h"

namespace ds { namespace ui {

	class CircleBorder : public Sprite {
	  public:
		CircleBorder(SpriteEngine&);
		CircleBorder(SpriteEngine&, const float width);

		void		setBorderWidth(const float borderWidth);
		const float getBorderWidth() { return mBorderWidth; }

		/// Initialization
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);

	  protected:
		virtual void writeAttributesTo(ds::DataBuffer&);
		virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);

		void updateShaderExtraData();

	  private:
		typedef Sprite inherited;
		void		   initialize();

		float mBorderWidth;
	};

}} // namespace ds::ui

#endif // DS_UI_CIRCLE_BORDER_H
