#pragma once
#ifndef DS_UI_CIRCLE_H
#define DS_UI_CIRCLE_H

#include <string>
#include <cinder/gl/Vbo.h>
#include <cinder/gl/Light.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/image_source/image_owner.h"
#include "ds/ui/mesh_source/mesh_owner.h"

namespace ds {
namespace ui {

/** Circle sprite is a convenience class to draw circles onscreen.
	This is faster than calling cinder's ci::gl::drawSolidCircle or drawStrokedCircle because this will cache the vertex array.
	Circles are drawn around the point ci::vec2f(radius,radius)
*/
class Circle : public Sprite {
public:
	Circle(SpriteEngine&);
	Circle(SpriteEngine&, const bool filled, const float radius);
	~Circle();

	/// Whether to draw just a stroke or just the fill
	void						setFilled(const bool filled);
	/// Whether to draw just a stroke or just the fill
	bool						getFilled(){ return mFilled; }

	/// Set the size of the circle
	void						setRadius(const float radius);
	/// Get the size of the circle
	const float					getRadius(){ return mRadius; }

	/// Only applies to non-filled circles
	void						setLineWidth(const float lineWidth);
	const float					getLineWidth(){ return mLineWidth; }

	virtual void				updateServer(const UpdateParams&);
	virtual void				drawLocalClient();
	virtual void				drawLocalServer();

	// Initialization
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);

protected:
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

	virtual void				onSizeChanged();

private:
	typedef Sprite				inherited;

	void						init();

	GLfloat*					mVertices;
	int							mNumberOfSegments;
	bool						mFilled;
	float						mRadius;
	float						mLineWidth;
	bool						mIgnoreSizeUpdates;

};

} // namespace ui
} // namespace ds

#endif //DS_UI_CIRCLE_H
