#pragma once
#ifndef _MEDIAVIEWER_APP_UI_BACKGROUND_PARTICLE_PARTICLE_BACKGROUND
#define _MEDIAVIEWER_APP_UI_BACKGROUND_PARTICLE_PARTICLE_BACKGROUND


#include <ds/app/event_client.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/video.h>


#include "cinder/Perlin.h"


namespace mv {

class Globals;

/**
 * \class mv::ParticleBackground
 *			Particles in the background
 */
class ParticleBackground final : public ds::ui::Sprite {
  public:
	ParticleBackground(Globals& g);

	void addRespulsor(const ds::ui::Sprite*);

  private:
	class Particle {
	  public:
		Particle()
		  : mPosition(ci::vec2())
		  , mVelocity(ci::vec2())
		  , mAge(0.0f)
		  , mLifespan(0.0f)
		  , mIsDead(false) {}
		ci::vec2 mPosition;
		ci::vec2 mVelocity;
		float	 mAge;
		float	 mLifespan;
		bool	 mIsDead;
		//		float			mPartScale;
	};


	// void								onAppEvent(const ds::Event&);
	virtual void onUpdateServer(const ds::UpdateParams& p);
	virtual void drawLocalClient();
	void		 resetParticle(Particle& p);
	bool		 particleOutOfBounds(Particle& p);

	Globals& mGlobals;

	ci::gl::TextureRef mImageTexture;

	ds::ui::Video* mVideoBacky;

	std::string mMediaPath;

	// for faster drawing
	ci::gl::BatchRef	mBatchRef;
	ci::gl::VboRef		mInstanceDataVbo;
	ci::gl::VboRef		mOpacityVbo;
	ci::gl::VboRef		mTextureLocVbo;
	ci::gl::VboRef		mScaleDataVbo;
	ci::gl::GlslProgRef mGlsl;

	std::vector<Particle> mParticles;
	float				  mAnimationCounter;
	ci::Perlin			  mPerlin;
	float				  mParticleScale;
};

} // namespace mv

#endif
