#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/sprite/web.h>

#include "cinder/Perlin.h"


namespace mv {

/**
 * \class mv::ParticleBackground
 *			Particles in the background
 */
class ParticleBackground : public ds::ui::Sprite {
  public:
	ParticleBackground(ds::ui::SpriteEngine& g);

	void addRespulsor(const ds::ui::Sprite*);
	void removeRepulsor(const ds::ui::Sprite*);
	void removeAllRepulsors();

	std::string getMediaPath() { return mMediaPath; }

	// Initialization
	static void installAsServer(ds::BlobRegistry&);
	static void installAsClient(ds::BlobRegistry&);

  protected:
	virtual void writeAttributesTo(ds::DataBuffer&);
	virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);

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

	virtual void onUpdateServer(const ds::UpdateParams& p);
	virtual void drawLocalClient();
	void		 resetParticle(Particle& p);
	bool		 particleOutOfBounds(Particle& p);


	ds::ui::Image* mImageBacky = nullptr;
	ds::ui::Video* mVideoBacky = nullptr;
	ds::ui::Pdf*   mPdfBacky   = nullptr;
	ds::ui::Web*   mWebBacky   = nullptr;

	std::vector<Repulsor> mRepulsors;

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
