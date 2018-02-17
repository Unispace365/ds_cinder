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
*/
class LineSprite final : public ds::ui::Sprite {
  public:
    LineSprite(ds::ui::SpriteEngine& eng);

    void addPoint(ci::vec2 point);
    void setPoints(const std::vector<ci::vec2>& points);
    std::vector<ci::vec2> getPoints() { return mPoints; }
    void clearPoints();

	void setSplineSmoothing(const bool doSmooth);

    void setLineWidth(const float linewidth);
    float getLineWidth() { return mLineWidth; }

    void setMiterLimit(const float miterLimit);
	float getMiterLimit() { return mMiterLimit; }

    virtual void drawLocalClient();
    virtual void onUpdateServer(const ds::UpdateParams& updateParams);

	// Initialization
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);

protected:
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);
    virtual void buildRenderBatch();
    virtual void onBuildRenderBatch();

  private:
    void buildVbo();

    bool  mSmoothSpline;
    float mMiterLimit;
    float mLineWidth;

    bool mNeedsVboUpdate;
    ci::gl::BatchRef      mBatch;
    std::vector<ci::vec2> mPoints;

    // ci::BSpline2f         mSpline;
    // ci::Path2d            mPath, mSplinePath;

    ci::gl::GlslProgRef mShader;
    ci::gl::VboMeshRef  mVboMesh;
};

}  // namespace experiments
}  // namespace experiments

#endif
