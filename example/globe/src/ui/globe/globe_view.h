#pragma once
#ifndef DS_EXAMPLE_GLOBE_UI_GLOBE_VIEW
#define DS_EXAMPLE_GLOBE_UI_GLOBE_VIEW


#include <ds/ui/sprite/mesh.h>
#include <ds/ui/sprite/sprite.h>

#include <ds/touch/delayed_momentum.h>

namespace ds { namespace ui {
	class GlobePinManager;
	class GlobePin;

	/**
	 * \class GlobeView - where the magic happens (TM).
	 */
	class GlobeView : public ds::ui::Sprite {
	  public:
		GlobeView(SpriteEngine& e, float mMinTilt = -22.0f, float mMaxTilt = 65.0f, float radius = 400.0f,
				  int meshResolution = 120);
		ci::vec3 getGlobeRotation() { return mGlobeRotation; }

		void addPin(ds::ui::GlobePin* pin);

		void setFocusOffset(float latitude, float longitude);
		void rotateTo(float latitude, float longitude);

		void clearFilters();
		void addFilter(int nfilt);
		void removeFilter(int nfilt);

	  private:
		const int	mMeshResolution;
		const float mGlobeRadius;

		SpriteEngine&	mEngine;
		ds::ui::Sprite& mTouchGrabber;

		ci::gl::Texture2dRef mTexDiffuse;
		ci::gl::Texture2dRef mTexNight;

		ci::gl::Texture2dRef mTexNormal;
		ci::gl::Texture2dRef mTexMask;

		// If we just straight up focus on a location it looks unnatural, use this to offset the focus point offcenter
		// so it reads better
		ci::vec2 mFocusOffset;

		virtual void onUpdateServer(const ds::UpdateParams& updateParams) override;
		virtual void drawLocalClient() override;

		ci::gl::BatchRef mEarth;
		ci::gl::BatchRef mGlow;
		ci::gl::BatchRef mOccluder;

		ds::ui::GlobePinManager& mPinManager;

		DelayedMomentum mXMomentum;
		DelayedMomentum mYMomentum;


		ci::gl::GlslProgRef mEarthShader;
		ci::gl::GlslProgRef mGlowShader;

		float mMinTilt;
		float mMaxTilt;

		double mLastTouched;
		double mTouchWaitOrbit;

		ci::vec3 mGlobeRotation;

		ci::vec3 mTweenStartRotation;
		ci::vec3 mTweenEndRotation;
		bool	 mIsTweeningRotation;
	};


}} // namespace ds::ui


#endif
