#pragma once
#ifndef DS_UI_CUSTOM_SPRITE
#define DS_UI_CUSTOM_SPRITE

#include "ds/ui/sprite/sprite.h"

namespace ds { namespace ui {


	/** Circle sprite is a convenience class to draw circles onscreen.
	This is faster than calling cinder's ci::gl::drawSolidCircle or drawStrokedCircle because this will cache the vertex
	array. Circles are drawn around the point ci::vec2f(radius,radius)
	*/
	class CustomSprite : public Sprite {
	  public:
		CustomSprite(SpriteEngine&);

		void	  setNumberOfSegments(const int numSegments);
		const int getNumberOfSegments() { return mNumberOfSegments; }

		void	  setNumberOfInstances(const int numInstances);
		const int getNumberOfInstances() { return mNumberOfInstances; }

		virtual void drawLocalClient();
		virtual void drawLocalServer();

		// Initialization
		// This is required to register this sprite as a net sync sprite
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);

	  protected:
		// This is where custom properties get written to the data buffer blob
		virtual void writeAttributesTo(ds::DataBuffer&);
		// And where they're pulled back out again
		virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);

	  private:
		virtual void onBuildRenderBatch() override;

		int mNumberOfSegments;
		int mNumberOfInstances;
	};

}} // namespace ds::ui

#endif // DS_UI_CIRCLE_H
