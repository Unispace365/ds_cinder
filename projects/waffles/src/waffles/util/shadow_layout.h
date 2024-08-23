#pragma once

#include <ds/ui/layout/layout_sprite.h>

namespace waffles {

//// he has fallen into shadow https://www.youtube.com/watch?v=qdD6Cte8HrU
class ShadowLayout : public ds::ui::LayoutSprite {
  public:
	ShadowLayout(ds::ui::SpriteEngine& g);

	void loadShaders();

	// how wide the blur is in pixels
	void setShadowSize(int blueSize);
	// How many times to run the blur (more = moar blur)
	void setShadowIterations(int iterations);
	// how  much opacity the blur itself has, the main text is always the sprite's opacity
	void setShadowOpacity(float burOpacity);
	// how blurry the blur is, some value between like 2 and 5 probably
	void setShadowSigma(float theSigma);
	// how color the blur is, some value between black and white
	void setShadowColor(ci::Color shadowColor);
	// how far away the blur is
	void setShadowOffset(ci::vec2 offset);
	// set if the shadow should re-render every frame (for instance, if running programatic/shader animations inside)
	void setShadowEveryFrame(bool renderEveryFrame);

	void tweenBlur(int toValue, const float duration, const float delay, const ci::EaseFn& = ci::easeNone);

	/// if anything in the layout has changed, mark it dirty dude
	void dirtyBlur() { mBlurDirty = true; }

	void enableShadow(bool enabled){
		mShadowEnabled = enabled;
		mBlurDirty = true;
	}

	static void installAsServer(ds::BlobRegistry&);
	static void installAsClient(ds::BlobRegistry&);

  protected:

	virtual void drawClient(const ci::mat4& transformMatrix, const ds::DrawParams& drawParams) override;
	void		 drawBlur();

	virtual void onSizeChanged() override;
	virtual void onScaleChanged() override;
	virtual void onAppearanceChanged(bool visibile) override;
	virtual void onPositionChanged() override;

	// NetSync
	virtual void writeAttributesTo(ds::DataBuffer&) override;
	virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&) override;

	ci::gl::BatchRef mBatch;

	ci::gl::Texture2dRef mSourceTexture = nullptr;
	ci::gl::Texture2dRef mBlurTexture	= nullptr;

	ci::gl::FboRef mFbo0;
	ci::gl::FboRef mFbo1;

	bool	  mShadowEnabled = true;
	ci::Color mShadowColor;
	int		  mShadowSize;
	float	  mPaddedSize;
	int		  mIterations;
	float	  mShadowSigma;
	float	  mShadowOpacity;
	ci::vec2  mShadowOffset;
	float	  mShadowScale = 0.25f;

	ci::gl::GlslProgRef mBlurShader;
	ci::gl::GlslProgRef mDrawShader;

	float mSourceOpacity = -1.f;
	int	  mPendingBlurs	 = 0;	 // Might be missing a frame or two at the end of transitions?
	bool  mBlurDirty	 = true; // only render the blur if needed
	bool  mAlwaysRender	 = false;

	bool mWarnedAboutYourChildren = false;

	int mBlurStart, mBlurTarget; // for tweening
};

} // namespace waffles
