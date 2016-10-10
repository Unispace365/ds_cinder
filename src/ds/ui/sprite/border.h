#pragma once
#ifndef DS_UI_BORDER_H
#define DS_UI_BORDER_H

#include <string>
#include <cinder/gl/Vbo.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/image_source/image_owner.h"
#include "ds/ui/mesh_source/mesh_owner.h"

namespace ds {
namespace ui {

	class Border : public Sprite {
	public:
		Border(SpriteEngine&);
		Border(SpriteEngine&, const float width);
		~Border();

		virtual void				onSizeChanged();
		
		void						setBorderWidth(const float borderWidth);
		const float					getBorderWidth(){ return mBorderWidth; }

		virtual void				updateServer(const UpdateParams&);
		virtual void				drawLocalClient();
		virtual void				drawLocalServer();

		// Initialization
		static void					installAsServer(ds::BlobRegistry&);
		static void					installAsClient(ds::BlobRegistry&);

	protected:
		virtual void				writeAttributesTo(ds::DataBuffer&);
		virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

	private:
		typedef Sprite				inherited;

		void						rebuildVertices();

		GLfloat*					mVertices;
		float						mBorderWidth;

	};

} // namespace ui
} // namespace ds

#endif //DS_UI_BORDER_H
