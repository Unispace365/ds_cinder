#include "stdafx.h"

#include "particles_view.h"

#include "cinder/gl/scoped.h"
#include <cinder/ImageIo.h>
#include <cinder/Rand.h>
#include <cinder/gl/gl.h>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"

namespace example {

namespace {

	const std::string particlesBackFrag =
		//"#version 150\n"
		"uniform sampler2D  tex0;\n"
		"in vec2		    iTexLoc;\n"
		"in float           Opacity;\n"
		"in vec4			theColor;\n"
		"out vec4           oColor;\n"
		"void main()\n"
		"{\n"
		//"    oColor = theColor;\n"
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
		"out vec4			theColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 newPositoin = ciPosition;\n"
		//"    newPositoin *= fInstanceScale;\n"
		//"    if(ciPosition.x < 0) newPositoin.x -= fInstanceScale; \n"
		//"    else newPositoin.x += fInstanceScale; \n"
		//"    if(ciPosition.y < 0) newPositoin.y -= fInstanceScale; \n"
		//"    else newPositoin.y += fInstanceScale; \n"
		//"    if(ciPosition.z < 0) newPositoin.z -= fInstanceScale; \n"
		//"    else newPositoin.z += fInstanceScale; \n"

		"    gl_Position = ciModelViewProjection * (newPositoin + vec4( vInstancePosition, 0 ) );\n"
		"    Opacity = fInstanceOpacity;\n"
		"    iTexLoc = fTexLoc;\n"
		"    theColor = ciColor;\n"
		"}\n";
} // namespace


ParticlesView::ParticlesView(ds::ui::SpriteEngine& eng)
  : ds::ui::Sprite(eng)
  , mAnimationCounter(0.0f)
  , mParticleScale(1.0f)
  , mBatchRef(nullptr)
  , mScaleDataVbo(nullptr)
  , mInstanceDataVbo(nullptr)
  , mOpacityVbo(nullptr)
  , mTextureLocVbo(nullptr)
  , mActive(true)
  , mEventClient(eng)
  , mImage(nullptr)
  , mVideo(nullptr) {
	auto theScale = mEngine.getAppSettings().getFloat("particles:overall_scale", 0, 1.0f);
	setScale(theScale, theScale);
	setSize(mEngine.getWorldWidth() / theScale, mEngine.getWorldHeight() / theScale);
	// setCenter(0.5f, 0.5f);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) { addTouch(ti); });

	int numParticles = mEngine.getAppSettings().getInt("particles:num", 0, 100);
#ifdef _DEBUG
	numParticles /= 10;
#endif
	for (int i = 0; i < numParticles; i++) {
		mParticles.push_back(Particle());
		resetParticle(mParticles.back());
		mParticles.back().mAge = ci::randFloat(0.0f, mParticles.back().mLifespan);
	}


	std::string blendName = mEngine.getAppSettings().getString("particles:blend_mode", 0, "normal");
	mPerlin.setSeed(clock());
	setTransparent(false);
	setBlendMode(ds::ui::getBlendModeByString(blendName));

	mEventClient.listenToEvents<ImageSwapEvent>([this](auto& e) { swapMedia(e.mPath); });
	swapMedia(mEngine.mContent.getPropertyString("media_path"));

	// setUseShaderTexture(true);

	setOpacity(0.0f);
	tweenOpacity(1.0f);

	mParticleScale = mEngine.getAppSettings().getFloat("particles:scale", 0, 1.0f);

	DS_LOG_INFO("Particle background creating mesh");


	auto			   meshStyle = mEngine.getAppSettings().getString("particles:mesh_style", 0, "square");
	ci::gl::VboMeshRef mesh;

	if (meshStyle == "circle") {
		auto circley = ci::geom::Circle();
		circley.radius(mParticleScale);
		mesh = ci::gl::VboMesh::create(circley);
	} else if (meshStyle == "icosahedron") {
		mesh = ci::gl::VboMesh::create(ci::geom::Icosahedron());
	} else if (meshStyle == "teapot") {
		mesh = ci::gl::VboMesh::create(ci::geom::Teapot().subdivisions(4));
	} else {
		mesh = ci::gl::VboMesh::create(
			ci::geom::Rect(ci::Rectf(-mParticleScale, -mParticleScale, mParticleScale, mParticleScale)));
	}


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

	addRespulsor(this);
}

bool ParticlesView::particleOutOfBounds(Particle& p) {
	if (p.mAge > p.mLifespan || p.mPosition.x < -mParticleScale || p.mPosition.y < -mParticleScale ||
		p.mPosition.x > mEngine.getWorldWidth() + mParticleScale ||
		p.mPosition.y > mEngine.getWorldHeight() + mParticleScale) {
		return true;
	}

	return false;
}

void ParticlesView::resetParticle(Particle& p) {

	float widdy	  = getWidth() / 2.0f;
	float hiddy	  = getHeight() / 2.0f;
	float ageMin  = mEngine.getAppSettings().getFloat("particles:lifespan_min", 0, 0.0f);
	float ageMax  = mEngine.getAppSettings().getFloat("particles:lifespan_max", 0, 0.0f);
	float minVel  = mEngine.getAppSettings().getFloat("particles:min_vel", 0, 0.0f);
	float maxVel  = mEngine.getAppSettings().getFloat("particles:max_vel", 0, 0.0f);
	float menuSi  = mEngine.getAppSettings().getFloat("particles:spawn_size", 0, 400.0f);
	p.mPosition.x = ci::randFloat(widdy - menuSi, widdy + menuSi);
	p.mPosition.y = ci::randFloat(hiddy - menuSi, hiddy + menuSi);
	p.mVelocity.x = ci::randFloat(minVel, maxVel);
	p.mVelocity.y = ci::randFloat(minVel, maxVel);
	p.mLifespan	  = ci::randFloat(ageMin, ageMax);
	p.mAge		  = 0.0f;
}

void ParticlesView::onUpdateServer(const ds::UpdateParams& p) {

	if (!mActive) return;
	/*
	removeAllRepulsors();
	for(auto it : mGlobals->mViewerController->getViewers()) {
	addRespulsor(it);
	}
	*/
	float		touchRad		  = mEngine.getAppSettings().getFloat("particles:touch_radius", 0, 400.0f);
	float		touchPow		  = mEngine.getAppSettings().getFloat("particles:touch_power", 0, 4.0f);
	float		maxOpacity		  = mEngine.getAppSettings().getFloat("particles:max_opacity", 0, 1.0f);
	float		overallSpeed	  = mEngine.getAppSettings().getFloat("particles:anim_speed", 0, 1.0f);
	float		counterMultiplier = mEngine.getAppSettings().getFloat("particles:counter_multiplier", 0, 10.0f);
	float		perlinScale		  = mEngine.getAppSettings().getFloat("particles:perlin_scale", 0, 0.001f);
	float		friction		  = mEngine.getAppSettings().getFloat("particles:friction", 0, 0.99f);
	const float repulsorFactor	  = mEngine.getAppSettings().getFloat("particles:repulsor:factor", 0, 16.0);
	const float repulsorBorder	  = mEngine.getAppSettings().getFloat("particles:repulsor:border", 0, 16.0);
	mAnimationCounter += p.getDeltaTime() * counterMultiplier;
	for (auto& repulsor : mRepulsors) {
		/*
		ci::Rectf bb = repSprit->getBoundingBox();
		const float repWid = bb.getWidth() / 2.0f + repulsorBorder;
		const float repHid = bb.getHeight() / 2.0f + repulsorBorder;
		ci::vec2 repPos = bb.getCenter();
		*/
		auto repSprit = repulsor.mSprite;
		repulsor.mWid = repSprit->getScaleWidth() / 2.0f;
		repulsor.mHid = repSprit->getScaleHeight() / 2.0f;
		repulsor.mCenter =
			ci::vec2(getWidth() / 2.0f,
					 getHeight() /
						 2.0f); //::vec2(repSprit->getPosition());
								// repulsor.mCenter.x += repulsor.mWid - 2.0f * repulsor.mWid * repSprit->getCenter().x;
								// repulsor.mCenter.y += repulsor.mHid - 2.0f * repulsor.mHid * repSprit->getCenter().y;
		repulsor.mWid += repulsorBorder;
		repulsor.mHid += repulsorBorder;
	}

	std::vector<ci::vec2> touchRespulsors;
	for (auto it : mTouchPoints) {
		ci::vec2 curPoint  = ci::vec2(it.second.mCurrentGlobalPoint);
		ci::vec2 globPoint = ci::vec2(globalToLocal(it.second.mCurrentGlobalPoint));
		// if(curPoint.x < 0.0f || curPoint.y < 0.0f) continue;
		touchRespulsors.push_back(globPoint);
	}

	if (!mInstanceDataVbo || !mOpacityVbo || !mTextureLocVbo || !mScaleDataVbo) return;

	float ww = mEngine.getWorldWidth();
	float wh = mEngine.getWorldHeight();


	ci::vec3* positions = (ci::vec3*)mInstanceDataVbo->mapReplace();
	float*	  opacities = (float*)mOpacityVbo->mapReplace();
	ci::vec2* texLocs	= (ci::vec2*)mTextureLocVbo->mapReplace();
	float*	  scales	= (float*)mScaleDataVbo->mapReplace();

	auto deltaTime = p.getDeltaTime() * 60.0f;
	bool topDown   = false;

	for (auto it = mParticles.begin(); it < mParticles.end(); ++it) {
		Particle& party = (*it);
		party.mPosition += party.mVelocity * deltaTime;
		party.mAge += deltaTime / 60.0f;

		if (particleOutOfBounds(party)) {
			resetParticle(party);
		}

		float xN =
			mPerlin.fBm(ci::vec3(party.mPosition.x + mAnimationCounter * 10.0f, party.mPosition.y, mAnimationCounter) *
						perlinScale);
		float yN = mPerlin.fBm(
			ci::vec3(party.mPosition.x + mAnimationCounter * 5.0f, party.mPosition.y, mAnimationCounter) * perlinScale);
		party.mVelocity.x += xN * overallSpeed;
		party.mVelocity.y += yN * overallSpeed;
		party.mVelocity *= friction;


		for (auto repSprit : mRepulsors) {

			float xDelt = fabs(party.mPosition.x - repSprit.mCenter.x); // / repSprit.mWid;
			float yDelt = fabs(party.mPosition.y - repSprit.mCenter.y); // / repSprit.mHid;
																		// if(xDelt < 1.0f && yDelt < 1.0f) {

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
			//}
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

		float fractionalAge = 2.0f * party.mAge / party.mLifespan;
		if (fractionalAge > 1.0f) fractionalAge = 2.0f - fractionalAge;
		*opacities++ = fractionalAge * maxOpacity;

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
}

void ParticlesView::drawLocalClient() {
	if (mBatchRef && mActive) {

		if (mImage && mImage->getImageTexture()) {
			mImage->getImageTexture()->setTopDown(false);
			mImage->getImageTexture()->bind();
		} else if (mVideo && mVideo->getFinalOutTexture()) {
			mVideo->getFinalOutTexture()->bind();
		}

		mBatchRef->drawInstanced((GLsizei)mParticles.size());

		if (mImage && mImage->getImageTexture()) {
			mImage->getImageTexture()->unbind();
		} else if (mVideo && mVideo->getFinalOutTexture()) {
			mVideo->getFinalOutTexture()->unbind();
		}
	}
}

void ParticlesView::addRespulsor(const ds::ui::Sprite* bs) {
	mRepulsors.push_back(Repulsor(const_cast<ds::ui::Sprite*>(bs)));
}

void ParticlesView::removeRepulsor(const ds::ui::Sprite* bs) {
	for (auto it = mRepulsors.begin(); it < mRepulsors.end(); ++it) {
		if ((*it).mSprite == bs) {
			mRepulsors.erase(it);
			break;
		}
	}
}

void ParticlesView::removeAllRepulsors() {
	mRepulsors.clear();
}

void ParticlesView::swapMedia(std::string media) {

	if (mImage) {
		mImage->release();
		mImage = nullptr;
	}

	if (mVideo) {
		mVideo->release();
		mVideo = nullptr;
	}

	ds::Resource reccy = ds::Resource(media);
	if (reccy.getType() == ds::Resource::IMAGE_TYPE) {
		mImage = new ds::ui::Image(mEngine);
		addChildPtr(mImage);
		mImage->hide();
		mImage->setImageFile(ds::Environment::expand(media));
	} else if (reccy.getType() == ds::Resource::VIDEO_TYPE) {
		mVideo = new ds::ui::Video(mEngine);
		mVideo->setFinalRenderToTexture(true);
		mVideo->setLooping(true);
		mVideo->setVolume(0.0f);
		mVideo->setAutoStart(true);
		if (media.find("rtsp") != std::string::npos) {
			mVideo->startStream(media, 1920.0f, 1080.0f);

		} else {
			mVideo->loadVideo(media);
		}
		addChildPtr(mVideo);
		// mVideo->hide();
	} else {

		DS_LOG_WARNING("Media not supported");
		return;
	}

	mEngine.mContent.setProperty("media_path", media);
}

void ParticlesView::addTouch(const ds::ui::TouchInfo& ti) {
	if (ti.mPhase == ds::ui::TouchInfo::Removed) {
		auto findy = mTouchPoints.find(ti.mFingerId);
		if (findy != mTouchPoints.end()) {
			mTouchPoints.erase(ti.mFingerId);
		}
	} else {
		mTouchPoints[ti.mFingerId] = ti;
	}
}

void ParticlesView::activate() {
	cancelDelayedCall();
	mActive = true;

	for (auto& it : mParticles) {
		resetParticle(it);
	}
}

void ParticlesView::deactivate() {
	callAfterDelay([this] { mActive = false; }, mEngine.getAnimDur());
}


} // namespace example
