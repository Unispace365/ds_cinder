#pragma once
#ifndef DS_UI_CIRCLE_BORDER_H
#define DS_UI_CIRCLE_BORDER_H

#include "ds/ui/sprite/sprite.h"

namespace ds {
namespace ui {

	class CircleBorder : public Sprite {
	public:
		CircleBorder(SpriteEngine&);
		CircleBorder(SpriteEngine&, const float width);

		virtual void				setSizeAll(const ci::Vec3f& size3d);
		virtual void				setSizeAll(float width, float height, float depth);

		void						setBorderWidth(const float borderWidth);
		const float					getBorderWidth(){ return mBorderWidth; }

		virtual void				updateServer(const UpdateParams&);
		
		// Initialization
		static void					installAsServer(ds::BlobRegistry&);
		static void					installAsClient(ds::BlobRegistry&);

	protected:
		virtual void				writeAttributesTo(ds::DataBuffer&);
		virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

		void						updateExtraShaderData();

	private:
		typedef Sprite				inherited;

		float						mBorderWidth;

	};

} // namespace ui
} // namespace ds

#endif //DS_UI_CIRCLE_BORDER_H
