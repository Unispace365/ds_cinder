#pragma once
#ifndef _EXPERIMENTS_APP_UI_HEX_LINE_SPRITE_H_
#define _EXPERIMENTS_APP_UI_HEX_LINE_SPRITE_H_


#include <cinder/BSpline.h>
#include <cinder/Path2d.h>

#include <ds/ui/sprite/sprite.h>

namespace ds::ui {

/**
 \class LineSprite
 \brief
	- UI elements extend from Sprite to participate in the display hierarchy, drawing transformations, built-in
 tweens, touch picking, etc.
	- Sprite can have children, which will be positioned in local space relative to the parent sprite.
	- Sprite will release all children when removing. Use release() to remove the sprite from it's parent and delete
 it.
	- The base Sprite class can be set to display a solid rectangle, assuming it has a size, is non-transparent, and
 has a color.
	- Sprites can be animated using the SpriteAnimatable functions to gracefully change size, position, opacity,
 scale, color, and rotation.
	- Sprites can clip their children along their bounds using setClipping(true)	 */
class LineSprite : public ds::ui::Sprite {
  public:
	LineSprite(ds::ui::SpriteEngine& eng, const std::vector<ci::vec2>& points = std::vector<ci::vec2>());

	/// \brief Add a point to the end of the line
	void addPoint(ci::vec2 point);
	/// \brief Set all points
	void setPoints(const std::vector<ci::vec2>& points);
	/// \brief Get all the points that make up the line
	std::vector<ci::vec2> getPoints() { return mPoints; }
	/// \brief Clear entire line
	void clearPoints();


	/// \brief Set percentage of the line to start drawing at
	void setStartPercentage(const float startAtPercent);
	/// \brief Get percentage of the line to start drawing at
	float getStartPercentage() { return mLineStart; }
	/// \brief Set percentage of the line to stop drawing at
	void setEndPercentage(const float endAtPercent);
	/// \brief Get percentage of the line to stop drawing at
	float getEndPercentage() { return mLineEnd; }
	/// \brief Sets the portion of the line to be drawn, in percentages (0.0f - 1.0f)
	void setStartEndPercentages(const float startAtPercent, const float endAtPercent);

	/// \brief Sets if the line should be converted to a smooth spline
	void setSmoothing(const bool doSmooth);
	bool getSmoothing() { return mSmoothSpline; }

	/// \brief Sets if the line should be converted to a smooth spline
	void setLineWidth(const float linewidth);
	/// \brief Sets if the line should be converted to a smooth spline
	float getLineWidth() { return mLineWidth; }

	/// \brief Set if sharp angled joins should be capped off. Ranges from -1: "Always miter" to +1: "Never Miter"
	void setMiterLimit(const float miterLimit);
	/// \brief Get current miter limit
	float getMiterLimit() { return mMiterLimit; }
	/// \brief Get the point at a certain percentage on the line.
	ci::vec2 getPointAtPercentage(float percentage);

	virtual void drawLocalClient();

	// Initialization
	static void installAsServer(ds::BlobRegistry&);
	static void installAsClient(ds::BlobRegistry&);

	virtual bool contains(const ci::vec3& point, const float pad = 0.0f) const;

  protected:
	virtual void writeAttributesTo(ds::DataBuffer&);
	virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);
	virtual void buildRenderBatch();
	virtual void onBuildRenderBatch();

	virtual bool getInnerHit(const ci::vec3& pos) const;

  private:
	void buildVbo();

	bool  mSmoothSpline;
	float mMiterLimit;
	float mLineWidth;
	float mLineStart, mLineEnd;
	float mLineLength;

	ci::gl::BatchRef	  mBatch;
	std::vector<ci::vec2> mPoints;

	ci::gl::GlslProgRef mShader;
	ci::gl::VboMeshRef	mVboMesh;
};

} // namespace ds::ui

#endif
