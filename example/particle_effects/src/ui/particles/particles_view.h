#pragma once
#ifndef EXAMPLE_UI_PARTICLES_PARTICLES_VIEW
#define EXAMPLE_UI_PARTICLES_PARTICLES_VIEW


#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/video.h>

#include "cinder/Perlin.h"

#include "ds/app/event_client.h"


namespace example {

/**
 * \class example::ParticlesView
 *			Particles in the view
 */
class ParticlesView : public ds::ui::Sprite {
  public:
	ParticlesView(ds::ui::SpriteEngine& g);

	void addRespulsor(const ds::ui::Sprite*);
	void removeRepulsor(const ds::ui::Sprite*);
	void removeAllRepulsors();
	void swapMedia(std::string media);

	void addTouch(const ds::ui::TouchInfo& ti);

	void activate();
	void deactivate();

  private:
	class Particle {
	  public:
		Particle()
		  : mPosition(ci::vec2())
		  , mVelocity(ci::vec2())
		  , mAge(0.0f)
		  , mLifespan(0.0f) {}
		ci::vec2 mPosition;
		ci::vec2 mVelocity;
		float	 mAge;
		float	 mLifespan;
	};
	class Repulsor {
	  public:
		Repulsor(ds::ui::Sprite* root)
		  : mSprite(root)
		  , mCenter(ci::vec2())
		  , mWid(0.0f)
		  , mHid(0.0f) {}
		ds::ui::Sprite* mSprite;
		ci::vec2		mCenter;
		float			mWid;
		float			mHid;
	};

	// void								onAppEvent(const ds::Event&);
	virtual void onUpdateServer(const ds::UpdateParams& p);
	virtual void drawLocalClient();
	void		 resetParticle(Particle& p);
	bool		 particleOutOfBounds(Particle& p);

	ds::EventClient		  mEventClient;
	std::vector<Repulsor> mRepulsors;
	// for faster drawing
	ci::gl::BatchRef	mBatchRef;
	ci::gl::VboRef		mInstanceDataVbo;
	ci::gl::VboRef		mOpacityVbo;
	ci::gl::VboRef		mTextureLocVbo;
	ci::gl::VboRef		mScaleDataVbo;
	ci::gl::GlslProgRef mGlsl;
	ds::ui::Image*		mImage;
	ds::ui::Video*		mVideo;

	std::map<int, ds::ui::TouchInfo> mTouchPoints;

	std::vector<Particle> mParticles;
	float				  mAnimationCounter;
	ci::Perlin			  mPerlin;
	float				  mParticleScale;

	bool mActive;
};

} // namespace example

#endif
