#include "stdafx.h"

#include "particle_generator.h"

#include <cinder/ImageIo.h>
#include <cinder/Rand.h>
#include <cinder/gl/gl.h>

#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/data_buffer.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

ParticleGenerator::ParticleGenerator(ds::ui::SpriteEngine& g)
	: ds::ui::Sprite(g) {

	mParticleScale = mEngine.getAppSettings().getFloat("touch_menu:particles:scale", 0, 1.0f);

	auto county = mEngine.getAppSettings().countSetting("touch_menu:particles:color_name");
	mColors.reserve(county);
	for (int i = 0; i < county; ++i) {
		mColors.push_back(mEngine.getColors().getColorFromName(
			mEngine.getAppSettings().getString("touch_menu:particles:color_name", i, "brand_dark")));
	}

	int numParticles = mEngine.getAppSettings().getInt("touch_menu:particles:num", 0, 100);
	for (int i = 0; i < numParticles; i++) {
		mParticles.push_back(Particle());
		resetParticle(mParticles.back());
	}

	setTransparent(false);
	setBlendMode(ds::ui::getBlendModeByString(
		mEngine.getAppSettings().getString("touch_menu:particles:blend_mode", 0, "screen")));

	
	setColorA(ci::ColorA::white());
	/* setColorA(mEngine.getColors().getColorFromName(
		mEngine.getAppSettings().getString("touch_menu:particles:color_name", 0, "brand_dark"))); */

	setOpacity(mEngine.getAppSettings().getFloat("touch_menu:particles:opacity", 0, 1.0f));
}

bool ParticleGenerator::particleOutOfBounds(Particle& p) {
	if (p.mAge > p.mLifespan) {
		//|| p.mPosition.x < 0.0f || p.mPosition.y < 0.0f || p.mPosition.x > mEngine.getWorldWidth() || p.mPosition.y >
		// mEngine.getWorldHeight()){
		return true;
	}

	return false;
}

void ParticleGenerator::resetParticle(Particle& p) {
	float ageMin  = mEngine.getAppSettings().getFloat("touch_menu:particles:lifespan_min", 0, 0.0f);
	float ageMax  = mEngine.getAppSettings().getFloat("touch_menu:particles:lifespan_max", 0, 0.0f);
	float minVel  = mEngine.getAppSettings().getFloat("touch_menu:particles:min_vel", 0, 0.0f);
	float maxVel  = mEngine.getAppSettings().getFloat("touch_menu:particles:max_vel", 0, 0.0f);
	p.mPosition.x = 0.0f;
	p.mPosition.y = 0.0f; // ci::randFloat(0, mEngine.getWorldWidth()), ci::randFloat(0.0f, mEngine.getWorldHeight()));
	p.mVelocity.x = ci::randFloat(minVel, maxVel);
	p.mVelocity.y = ci::randFloat(minVel, maxVel);
	p.mLifespan	  = ci::randFloat(ageMin, ageMax);
	p.mAge		  = 0.0f;
	p.mColor	  = mColors[ci::randInt(0, mColors.size())];
}

void ParticleGenerator::onUpdateServer(const ds::UpdateParams& p) {
	float overallSpeed = mEngine.getAppSettings().getFloat("touch_menu:particles:anim_speed", 0, 1.0f);
	float friction	   = mEngine.getAppSettings().getFloat("touch_menu:particles:friction", 0, 0.99f);

	for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
		Particle& party = (*it);
		party.mPosition += party.mVelocity;
		party.mAge += p.getDeltaTime();

		if (particleOutOfBounds(party)) {
			resetParticle(party);
		}

		//	party.mVelocity.x += overallSpeed;
		//	party.mVelocity.y += overallSpeed;
		party.mVelocity *= friction;
	}
}

void ParticleGenerator::drawLocalClient() {
	/* float ww = mEngine.getWorldWidth();
	float wh = mEngine.getWorldHeight(); */
	for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
		ci::gl::color((*it).mColor);
		ci::vec2 pos = (*it).mPosition;


		//	partColor.a = mDrawOpacity * (*it).mAge / (*it).mLifespan;
		// ci::gl::color(getColor);
		float	 squareMag	= mParticleScale * ((*it).mLifespan - (*it).mAge) / 2.0f;
		ci::vec2 squareSize = ci::vec2(squareMag, squareMag);
		// ci::gl::drawSolidRect(ci::Rectf((*it).mPosition - squareSize, (*it).mPosition + squareSize));
		ci::gl::drawSolidCircle(pos, squareMag);
	}
}

} // namespace mv
