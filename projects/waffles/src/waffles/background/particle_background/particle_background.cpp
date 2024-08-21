#include "stdafx.h"

#include "particle_background.h"

#include <cinder/gl/scoped.h>
#include <cinder/ImageIo.h>
#include <cinder/Rand.h>
#include <cinder/gl/gl.h>

#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/data/data_buffer.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "waffles/waffles_sprite.h"
#include "waffles/viewers/base_element.h"
#include "waffles/viewers/viewer_controller.h"
#include "waffles/util/waffles_helper.h"

namespace waffles {

namespace {

	const std::string particlesBackFrag =
		//"#version 150\n"
		"uniform sampler2D  tex0;\n"
		"in vec2		    iTexLoc;\n"
		"in float           Opacity;\n"
		"out vec4           oColor;\n"
		"void main()\n"
		"{\n"
		//"    oColor = Color;\n"
		"    oColor = texture2D( tex0, iTexLoc );\n"
		"    oColor.a = Opacity;\n"
		"}\n";

	const std::string particlesBackVert =
		"#version 150\n"
		"uniform mat4       ciModelMatrix;\n"
		"uniform mat4       ciModelViewProjection;\n"
		"in vec3		    vInstancePosition;\n"
		"in float           fInstanceOpacity;\n"
		"in float           fInstanceScale;\n"
		"in vec2            fTexLoc;\n"
		"out vec2           iTexLoc;\n"
		"in vec4            ciPosition;\n"
		"in vec4            ciColor;\n"
		"out float          Opacity;\n"
		"void main()\n"
		"{\n"
		"    vec4 newPositoin = ciPosition;\n"
		"    if(ciPosition.x < 0) newPositoin.x -= fInstanceScale; \n"
		"    else newPositoin.x += fInstanceScale; \n"
		"    if(ciPosition.y < 0) newPositoin.y -= fInstanceScale; \n"
		"    else newPositoin.y += fInstanceScale; \n"

		"    gl_Position = ciModelViewProjection * (newPositoin + vec4( vInstancePosition, 0 ) );\n"
		"    Opacity = fInstanceOpacity;\n"
		"    iTexLoc = fTexLoc;\n"
		"}\n";

	char BLOB_TYPE = 0;

	const ds::ui::DirtyState& PARTICLES_DIRTY = ds::ui::INTERNAL_A_DIRTY;

	const char PARTICLES_ATT = 80;
} // namespace


void ParticleBackground::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](ds::BlobReader& r) { ds::ui::Sprite::handleBlobFromClient(r); });
}

void ParticleBackground::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](ds::BlobReader& r) { ds::ui::Sprite::handleBlobFromServer<ParticleBackground>(r); });
}

ParticleBackground::ParticleBackground(ds::ui::SpriteEngine& eng)
	: ds::ui::Sprite(eng)
	, mBatchRef(nullptr)
	, mInstanceDataVbo(nullptr)
	, mOpacityVbo(nullptr)
	, mTextureLocVbo(nullptr)
	, mScaleDataVbo(nullptr)
	, mAnimationCounter(0.0f)
	, mParticleScale(1.0f) {
	mBlobType = BLOB_TYPE;

	int numParticles = mEngine.getWafflesSettings().getInt("particles:num", 0, 100);
#ifdef _DEBUG
	numParticles /= 10;
#endif
	for (int i = 0; i < numParticles; i++) {
		mParticles.push_back(Particle());
		resetParticle(mParticles.back());
	}

	std::string blendName = mEngine.getWafflesSettings().getString("particles:blend_mode", 0, "normal");
	mPerlin.setSeed(clock());
	setTransparent(false);
	setBlendMode(ds::ui::getBlendModeByString(blendName));

	setOpacity(0.0f);
	tweenOpacity(1.0f);

	mParticleScale = mEngine.getWafflesSettings().getFloat("particles:scale", 0, 1.0f);

	///POTENTIAL TODO: Handle things from video, like size, volume, playback features etc.
	ds::Resource theRec;
	int thePage = 0;
	float theVolume = 0.0;
	if (auto helper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper> ()) {
		theRec = helper->getBackgroundForPlatform();
		thePage = helper->getBackgroundPdfPage();
	}

	if (theRec.empty()) {
		mMediaPath = ds::Environment::expand("%APP%/data/images/waffles/default_background.jpg");
		theRec = ds::Resource::fromImage(mMediaPath);
	}

	DS_LOG_INFO("Particle background starting : " << mMediaPath);

	if (theRec.getType() == ds::Resource::IMAGE_TYPE) {
		mImageBacky = new ds::ui::Image(mEngine, theRec);
		addChildPtr(mImageBacky);
		mImageBacky->hide();

	} else if (theRec.getType() == ds::Resource::VIDEO_TYPE) {
		mVideoBacky = new ds::ui::Video(mEngine);
		mVideoBacky->setFinalRenderToTexture(true);
		mVideoBacky->setLooping(true);
		mVideoBacky->setVolume(0.0f);
		mVideoBacky->setAutoStart(true);
		mVideoBacky->loadVideo(mMediaPath);
		addChildPtr(mVideoBacky);

	} else if (theRec.getType() == ds::Resource::VIDEO_STREAM_TYPE) {
		mVideoBacky = new ds::ui::Video(mEngine);
		mVideoBacky->setFinalRenderToTexture(true);
		mVideoBacky->setLooping(true);
		mVideoBacky->setVolume(0.0f);
		mVideoBacky->setAutoStart(true);
		mVideoBacky->startStream(mMediaPath, 1920.0f, 1080.0f);
		addChildPtr(mVideoBacky);

	} else if (theRec.getType() == ds::Resource::PDF_TYPE) {
		mPdfBacky = new ds::ui::Pdf(mEngine);
		mPdfBacky->setResourceFilename(mMediaPath);

		if (thePage > 0) {
			mPdfBacky->setPageNum(thePage);
		}

		addChildPtr(mPdfBacky);
		mPdfBacky->hide();

	} else if (theRec.getType() == ds::Resource::WEB_TYPE) {
		mWebBacky = new ds::ui::Web(mEngine, 1920.0f, 1080.0f);
		mWebBacky->setFinalRenderToTexture(true);
		addChildPtr(mWebBacky);
		mWebBacky->loadUrl(mMediaPath);
	} else {
		DS_LOG_WARNING("Particle Background, media type not supported for file " << mMediaPath);
	}

	DS_LOG_INFO("Particle background creating mesh");

	ci::gl::VboMeshRef mesh = ci::gl::VboMesh::create(ci::geom::Rect(ci::Rectf(-0.5f, -0.5f, 0.5f, 0.5f)));

	// auto circley = ci::geom::Circle();
	// circley.radius(mParticleScale);
	// ci::gl::VboMeshRef mesh = ci::gl::VboMesh::create(circley);

	try {
		mGlsl = ci::gl::GlslProg::create(particlesBackVert, particlesBackFrag);
	} catch (std::exception& e) {
		DS_LOG_WARNING("ParticleBackground glsl compile error: " << e.what());
		return;
	}

	std::vector<ci::vec3> positions;
	std::vector<float>	  opacities;
	std::vector<ci::vec2> textureLocs;
	std::vector<float>	  scales;

	for (size_t potX = 0; potX < numParticles; ++potX) {
		positions.push_back(ci::vec3(mParticles[potX].mPosition, 0.0f));
		opacities.push_back(0.0f);
		textureLocs.push_back(ci::vec2());
		scales.push_back(mParticleScale);
	}

	// create the VBO which will contain per-instance (rather than per-vertex) data
	mInstanceDataVbo =
		ci::gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(ci::vec3), positions.data(), GL_DYNAMIC_DRAW);
	mOpacityVbo =
		ci::gl::Vbo::create(GL_ARRAY_BUFFER, opacities.size() * sizeof(float), opacities.data(), GL_DYNAMIC_DRAW);
	mScaleDataVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, scales.size() * sizeof(float), scales.data(), GL_DYNAMIC_DRAW);
	mTextureLocVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, textureLocs.size() * sizeof(ci::vec2), textureLocs.data(),
										 GL_DYNAMIC_DRAW);

	// we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0)
	// as the last param indicates per-instance (rather than per-vertex)
	ci::geom::BufferLayout instanceDataLayout;
	instanceDataLayout.append(ci::geom::Attrib::CUSTOM_0, 3, 0, 0, 1 /* per instance */);
	ci::geom::BufferLayout instanceOpacityLayout;
	instanceOpacityLayout.append(ci::geom::Attrib::CUSTOM_1, 1, 0, 0, 1 /* per instance */);
	ci::geom::BufferLayout instanceScaleLayout;
	instanceScaleLayout.append(ci::geom::Attrib::CUSTOM_2, 1, 0, 0, 1 /* per instance */);
	ci::geom::BufferLayout instanceTexLocLayout;
	instanceTexLocLayout.append(ci::geom::Attrib::CUSTOM_3, 2, 0, 0, 1 /* per instance */);

	// now add it to the VboMesh we already made of the Teapot
	mesh->appendVbo(instanceDataLayout, mInstanceDataVbo);
	mesh->appendVbo(instanceOpacityLayout, mOpacityVbo);
	mesh->appendVbo(instanceScaleLayout, mScaleDataVbo);
	mesh->appendVbo(instanceTexLocLayout, mTextureLocVbo);

	ci::gl::Batch::AttributeMapping attriMap;
	attriMap[ci::geom::Attrib::CUSTOM_0] = "vInstancePosition";
	attriMap[ci::geom::Attrib::CUSTOM_1] = "fInstanceOpacity";
	attriMap[ci::geom::Attrib::CUSTOM_2] = "fInstanceScale";
	attriMap[ci::geom::Attrib::CUSTOM_3] = "fTexLoc";

	mBatchRef = ci::gl::Batch::create(mesh, mGlsl, attriMap);

	DS_LOG_INFO("Particle background ready");
}

bool ParticleBackground::particleOutOfBounds(Particle& p) {
	if (p.mAge > p.mLifespan || p.mPosition.x < 0.0f || p.mPosition.y < 0.0f ||
		p.mPosition.x > mEngine.getWorldWidth() || p.mPosition.y > mEngine.getWorldHeight()) {
		return true;
	}

	return false;
}

void ParticleBackground::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if (mDirty.has(PARTICLES_DIRTY)) {
		buf.add(PARTICLES_ATT);
		int numParts = (int)mParticles.size();
		buf.add(mParticleScale);
		buf.add(numParts);
		for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
			buf.add((*it).mPosition.x);
			buf.add((*it).mPosition.y);
			buf.add((*it).mAge);
			buf.add((*it).mLifespan);
		}
	}
}

void ParticleBackground::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == PARTICLES_ATT) {
		mParticleScale = buf.read<float>();
		int numParts   = buf.read<int>();
		if (mParticles.size() != numParts) {
			mParticles.clear();
			for (int i = 0; i < numParts; i++) {
				mParticles.push_back(Particle());
			}
		}

		for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
			Particle& p	  = (*it);
			float	  xp  = buf.read<float>();
			float	  yp  = buf.read<float>();
			p.mPosition.x = xp;
			p.mPosition.y = yp;
			p.mAge		  = buf.read<float>();
			p.mLifespan	  = buf.read<float>();
		}

	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

void ParticleBackground::resetParticle(Particle& p) {
	float ageMin  = mEngine.getWafflesSettings().getFloat("particles:lifespan_min", 0, 0.0f);
	float ageMax  = mEngine.getWafflesSettings().getFloat("particles:lifespan_max", 0, 0.0f);
	float minVel  = mEngine.getWafflesSettings().getFloat("particles:min_vel", 0, 0.0f);
	float maxVel  = mEngine.getWafflesSettings().getFloat("particles:max_vel", 0, 0.0f);
	p.mPosition.x = ci::randFloat(0, mEngine.getWorldWidth());
	p.mPosition.y = ci::randFloat(0.0f, mEngine.getWorldHeight());
	p.mVelocity.x = ci::randFloat(minVel, maxVel);
	p.mVelocity.y = ci::randFloat(minVel, maxVel);
	p.mLifespan	  = ci::randFloat(ageMin, ageMax);
	p.mAge		  = 0.0f;
}

void ParticleBackground::onUpdateServer(const ds::UpdateParams& p) {
	removeAllRepulsors();

	auto vc = ViewerController::getInstance();
	if (vc) {
		for (auto it : vc->getViewers()) {
			addRespulsor(it);
		}
	}

	float		touchRad		  = mEngine.getWafflesSettings().getFloat("particles:touch_radius", 0, 400.0f);
	float		touchPow		  = mEngine.getWafflesSettings().getFloat("particles:touch_power", 0, 4.0f);
	float		overallSpeed	  = mEngine.getWafflesSettings().getFloat("particles:anim_speed", 0, 1.0f);
	float		counterMultiplier = mEngine.getWafflesSettings().getFloat("particles:counter_multiplier", 0, 10.0f);
	float		perlinScale		  = mEngine.getWafflesSettings().getFloat("particles:perlin_scale", 0, 0.001f);
	float		friction		  = mEngine.getWafflesSettings().getFloat("particles:friction", 0, 0.99f);
	const float repulsorFactor	  = mEngine.getWafflesSettings().getFloat("particles:repulsor:factor", 0, 16.0);
	const float repulsorBorder	  = mEngine.getWafflesSettings().getFloat("particles:repulsor:border", 0, 16.0);
	mAnimationCounter += p.getDeltaTime() * counterMultiplier;
	for (auto& repulsor : mRepulsors) {
		auto repSprit	 = repulsor.mSprite;
		repulsor.mWid	 = repSprit->getScaleWidth() / 2.0f;
		repulsor.mHid	 = repSprit->getScaleHeight() / 2.0f;
		repulsor.mCenter = ci::vec2(repSprit->getGlobalPosition());
		repulsor.mCenter.x += repulsor.mWid - 2.0f * repulsor.mWid * repSprit->getCenter().x;
		repulsor.mCenter.y += repulsor.mHid - 2.0f * repulsor.mHid * repSprit->getCenter().y;
		repulsor.mWid += repulsorBorder;
		repulsor.mHid += repulsorBorder;
	}

	std::vector<ci::vec2> touchRespulsors;

	auto wallFools = WafflesSprite::getDefault();
	if (wallFools) {
		auto tp = wallFools->mTouchPoints;
		for (auto it : tp) {
			ci::vec2 curPoint = it.second;
			if (curPoint.x < 0.0f || curPoint.y < 0.0f) continue;
			touchRespulsors.push_back(curPoint);
		}
	}

	if (!mInstanceDataVbo || !mOpacityVbo || !mTextureLocVbo || !mScaleDataVbo) return;

	float ww = mEngine.getWorldWidth();
	float wh = mEngine.getWorldHeight();


	ci::vec3* positions = (ci::vec3*)mInstanceDataVbo->mapReplace();
	float*	  opacities = (float*)mOpacityVbo->mapReplace();
	ci::vec2* texLocs	= (ci::vec2*)mTextureLocVbo->mapReplace();
	float*	  scales	= (float*)mScaleDataVbo->mapReplace();

	// both images and videos are top-down
	bool topDown = true;

	for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
		Particle& party = (*it);
		party.mPosition += party.mVelocity;
		party.mAge += p.getDeltaTime();

		if (particleOutOfBounds(party)) {
			resetParticle(party);
		}

		float xN =
			mPerlin.fBm(ci::vec3(party.mPosition.x + mAnimationCounter * 10.0f, party.mPosition.y, mAnimationCounter) *
						perlinScale);
		party.mVelocity.x += xN * overallSpeed;
		party.mVelocity.y += xN * overallSpeed;
		party.mVelocity *= friction;


		for (auto repSprit : mRepulsors) {

			float xDelt = fabs(party.mPosition.x - repSprit.mCenter.x) / repSprit.mWid;
			float yDelt = fabs(party.mPosition.y - repSprit.mCenter.y) / repSprit.mHid;
			if (xDelt < 1.0f && yDelt < 1.0f) {

				if (party.mPosition.x < repSprit.mCenter.x) {
					party.mVelocity.x -= xDelt / repulsorFactor;
				} else {
					party.mVelocity.x += xDelt / repulsorFactor;
				}

				if (party.mPosition.y < repSprit.mCenter.y) {
					party.mVelocity.y -= yDelt / repulsorFactor;
				} else {
					party.mVelocity.y += yDelt / repulsorFactor;
				}
			}
		}

		for (auto repTouch : touchRespulsors) {
			float disty = glm::distance(party.mPosition, repTouch);
			if (disty < touchRad) {
				float power = (1.0f - disty / touchRad) * touchPow;
				float xDelt = repTouch.x - party.mPosition.x;
				float yDelt = repTouch.y - party.mPosition.y;
				party.mVelocity.x += xDelt * power;
				party.mVelocity.y += yDelt * power;
			}
		}

		*positions++ = ci::vec3(party.mPosition, 0.0f);

		*opacities++ = party.mAge / party.mLifespan;
		if (topDown) {
			*texLocs++ = ci::vec2(party.mPosition.x / ww, 1.0f - party.mPosition.y / wh);
		} else {
			*texLocs++ = ci::vec2(party.mPosition.x / ww, party.mPosition.y / wh);
		}

		float squareMag = mParticleScale * (party.mLifespan - party.mAge) / 2.0f;
		*scales++		= squareMag;
	}

	mOpacityVbo->unmap();
	mInstanceDataVbo->unmap();
	mScaleDataVbo->unmap();
	mTextureLocVbo->unmap();

	markAsDirty(PARTICLES_DIRTY);
}

void ParticleBackground::drawLocalClient() {
	if (mBatchRef) {
		if (mVideoBacky && mVideoBacky->getFinalOutTexture()) {
			mVideoBacky->getFinalOutTexture()->bind();
		} else if (mImageBacky && mImageBacky->getImageTexture()) {
			mImageBacky->getImageTexture()->bind();
		} else if (mPdfBacky && mPdfBacky->getTextureRef()) {
			mPdfBacky->getTextureRef()->bind();
		} else if (mWebBacky && mWebBacky->getFinalOutTexture()) {
			mWebBacky->getFinalOutTexture()->bind();
		}

		mBatchRef->drawInstanced((GLsizei)mParticles.size());

		if (mVideoBacky && mVideoBacky->getFinalOutTexture()) {
			mVideoBacky->getFinalOutTexture()->unbind();
		} else if (mImageBacky && mImageBacky->getImageTexture()) {
			mImageBacky->getImageTexture()->unbind();
		} else if (mPdfBacky && mPdfBacky->getTextureRef()) {
			mPdfBacky->getTextureRef()->unbind();
		} else if (mWebBacky && mWebBacky->getFinalOutTexture()) {
			mWebBacky->getFinalOutTexture()->unbind();
		}
	}
}

void ParticleBackground::addRespulsor(const ds::ui::Sprite* bs) {
	mRepulsors.push_back(Repulsor(const_cast<ds::ui::Sprite*>(bs)));
}

void ParticleBackground::removeRepulsor(const ds::ui::Sprite* bs) {
	for (auto it = mRepulsors.begin(); it < mRepulsors.end(); ++it) {
		if ((*it).mSprite == bs) {
			mRepulsors.erase(it);
			break;
		}
	}
}

void ParticleBackground::removeAllRepulsors() {
	mRepulsors.clear();
}


} // namespace mv
