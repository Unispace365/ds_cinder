#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

#include "cinder/Perlin.h"
#include "cinder/Surface.h"

namespace waffles {

/**
 * \class mv::ParticleGenerator
 *			Particles for the FFM
 */
class ParticleGenerator : public ds::ui::Sprite {
  public:
	ParticleGenerator(ds::ui::SpriteEngine& g);

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
		ci::Color mColor;
		float	 mAge;
		float	 mLifespan;
	};

	virtual void onUpdateServer(const ds::UpdateParams& p);
	virtual void drawLocalClient();
	void		 resetParticle(Particle& p);
	bool		 particleOutOfBounds(Particle& p);

	float				  mParticleScale;
	std::vector<Particle> mParticles;

	std::vector<ci::Color> mColors;
};

} // namespace mv
