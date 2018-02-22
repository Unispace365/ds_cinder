#pragma once
#ifndef _EXPERIMENTS_APP_UI_HEX_LINE_SPRITE_H_
#define _EXPERIMENTS_APP_UI_HEX_LINE_SPRITE_H_


#include <cinder/BSpline.h>
#include <cinder/Path2d.h>

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::Text
*	A sprite for drawing lines made up of multiple points, nicely joined.
*	!! USE WITH CAUTION, WIP !!
*/
class LineSprite final : public ds::ui::Sprite {
  public:
	LineSprite(ds::ui::SpriteEngine& eng);

	void addPoint(ci::vec2 point);
	void setPoints(const std::vector<ci::vec2>& points);
	std::vector<ci::vec2> getPoints() { return mPoints; }
	void				  clearPoints();


	void setLineStart(const float startAtPercent);
	float getLineStart() { return mLineStart; }
	void setLineEnd(const float endAtPercent);
	float getLineEnd() { return mLineEnd; }
	void setLineStartEnd(const float startAtPercent, const float endAtPercent);

	void setSmoothing(const bool doSmooth);
	bool getSmoothing() { return mSmoothSpline; }

	void setLineWidth(const float linewidth);
	float getLineWidth() { return mLineWidth; }

	void setMiterLimit(const float miterLimit);
	float getMiterLimit() { return mMiterLimit; }

	virtual void drawLocalClient();
	virtual void onUpdateServer(const ds::UpdateParams& updateParams);

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

	ci::gl::BatchRef	  mBatch;
	std::vector<ci::vec2> mPoints;

	ci::gl::GlslProgRef mShader;
	ci::gl::VboMeshRef  mVboMesh;
};

}  // namespace experiments
}  // namespace experiments

#endif
