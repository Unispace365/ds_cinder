#pragma once
#ifndef DS_UI_CIRCLE_H
#define DS_UI_CIRCLE_H

#include "ds/ui/sprite/sprite.h"

namespace ds { namespace ui {

	// working circle geomety
	class DsCircleGeom : public ci::geom::Source {
	  public:
		DsCircleGeom();

		DsCircleGeom& center(const ci::vec2& center) {
			mCenter = center;
			return *this;
		}
		DsCircleGeom& radius(float radius);
		DsCircleGeom& subdivisions(int subdivs);

		size_t				getNumVertices() const override;
		size_t				getNumIndices() const override { return 0; }
		ci::geom::Primitive getPrimitive() const override { return ci::geom::Primitive::TRIANGLE_FAN; }
		uint8_t				getAttribDims(ci::geom::Attrib attr) const override;
		ci::geom::AttribSet getAvailableAttribs() const override;
		void		  loadInto(ci::geom::Target* target, const ci::geom::AttribSet& requestedAttribs) const override;
		DsCircleGeom* clone() const override { return new DsCircleGeom(*this); }

	  private:
		void updateVertexCounts();

		ci::vec2 mCenter;
		float	 mRadius;
		int		 mRequestedSubdivisions, mNumSubdivisions;
		size_t	 mNumVertices;
	};

	/** Circle sprite is a convenience class to draw circles onscreen.
		This is faster than calling cinder's ci::gl::drawSolidCircle or drawStrokedCircle because this will cache the
	   vertex array. Circles are drawn around the point ci::vec2f(radius,radius)
	*/
	class Circle : public Sprite {
	  public:
		Circle(SpriteEngine&);
		Circle(SpriteEngine&, const bool filled, const float radius);

		/// Whether to draw just a stroke or just the fill
		void setFilled(const bool filled);
		/// Whether to draw just a stroke or just the fill
		bool getFilled() { return mFilled; }

		/// Set the size of the circle
		void setRadius(const float radius);
		/// Get the size of the circle
		const float getRadius() { return mRadius; }

		/// Only applies to non-filled circles
		void		setLineWidth(const float lineWidth);
		const float getLineWidth() { return mLineWidth; }

		void	  setNumberOfSegments(const int numSegments);
		const int getNumberOfSegments() { return mNumberOfSegments; }

		virtual void drawLocalClient();
		virtual void drawLocalServer();

		/// Initialization
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);

	  protected:
		virtual void writeAttributesTo(ds::DataBuffer&);
		virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);

		virtual void onSizeChanged();

	  private:
		typedef Sprite inherited;

		virtual void onBuildRenderBatch() override;

		int	  mNumberOfSegments;
		bool  mFilled;
		float mRadius;
		float mLineWidth;
		bool  mIgnoreSizeUpdates;
	};

}} // namespace ds::ui

#endif // DS_UI_CIRCLE_H
