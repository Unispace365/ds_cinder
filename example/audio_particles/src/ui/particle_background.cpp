#include "stdafx.h"

#include "particle_background.h"

#include <cinder/Rand.h>
#include <cinder/gl/gl.h>
#include <cinder/ImageIo.h>
#include "cinder/gl/scoped.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

namespace mv {

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

//"    if(oColor.r < 0.5) oColor.rgba = vec4(0.0);\n"
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

}

ParticleBackground::ParticleBackground(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mAnimationCounter(0.0f)
	, mParticleScale(1.0f)
	, mVideoBacky(nullptr)
	, mImageTexture(nullptr)
	, mBatchRef(nullptr)
	, mScaleDataVbo(nullptr)
	, mInstanceDataVbo(nullptr)
	, mOpacityVbo(nullptr)
	, mTextureLocVbo(nullptr)
{

	auto ctx = ci::audio::Context::master();

	// The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
	mInputDeviceNode = ctx->createInputDeviceNode();

	// By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
	// an increase in resolution of the resulting spectrum data.
	auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorSpectralNode = ctx->makeNode(new ci::audio::MonitorSpectralNode(monitorFormat));

	mInputDeviceNode >> mMonitorSpectralNode;

	// InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
	mInputDeviceNode->enable();
	ctx->enable();



	int numParticles = mGlobals.getAppSettings().getInt("particles:num", 0, 100);

	
	for(int i = 0; i < numParticles; i++){
		mParticles.push_back(Particle());
		resetParticle(mParticles.back());
	//	mParticles.back().mIsDead = true;
	}
	

	std::string blendName = mGlobals.getAppSettings().getText("particles:blend_mode", 0, "normal");
	mPerlin.setSeed(clock());
	setTransparent(false);
	setBlendMode(ds::ui::getBlendModeByString(blendName));

	//setUseShaderTexture(true);

	setOpacity(0.0f);
	tweenOpacity(1.0f);

	mParticleScale = mGlobals.getAppSettings().getFloat("particles:scale", 0, 1.0f);

	std::string vidPath = ds::Environment::expand(mGlobals.getAppSettings().getText("video_path", 0, ""));


	mMediaPath = vidPath;
	mVideoBacky = new ds::ui::Video(mEngine);
	mVideoBacky->setFinalRenderToTexture(true);
	mVideoBacky->setLooping(true);
	//mVideoBacky->setVolume(0.0f);
	mVideoBacky->setAutoStart(false);
	mVideoBacky->enable(true);
	mVideoBacky->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mVideoBacky->setTapCallback([this](ds::ui::Sprite*,const ci::vec3&){
		mVideoBacky->play();
	});
	if(vidPath.find("rtsp") != std::string::npos){
		mVideoBacky->startStream(vidPath, 1920.0f, 1080.0f);

	} else {
		mVideoBacky->loadVideo(vidPath);
	}
	addChildPtr(mVideoBacky);
	

	ci::gl::VboMeshRef mesh = ci::gl::VboMesh::create(ci::geom::Rect(ci::Rectf(-0.5f, -0.5f, 0.5f, 0.5f)));

	//auto circley = ci::geom::Circle();
	//circley.radius(mParticleScale);
	//ci::gl::VboMeshRef mesh = ci::gl::VboMesh::create(circley);

	try{
		mGlsl = ci::gl::GlslProg::create(particlesBackVert, particlesBackFrag);
	} catch(std::exception& e){
		DS_LOG_WARNING("ParticleBackground glsl compile error: " << e.what());
		return;
	}

	std::vector<ci::vec3> positions;
	std::vector<float> opacities;
	std::vector<ci::vec2> textureLocs;
	std::vector<float> scales;

	for(size_t potX = 0; potX < numParticles; ++potX) {
		positions.push_back(ci::vec3(mParticles[potX].mPosition, 0.0f));
		opacities.push_back(0.0f);
		textureLocs.push_back(ci::vec2());
		scales.push_back(mParticleScale);
	}

	// create the VBO which will contain per-instance (rather than per-vertex) data
	mInstanceDataVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(ci::vec3), positions.data(), GL_DYNAMIC_DRAW);
	mOpacityVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, opacities.size() * sizeof(float), opacities.data(), GL_DYNAMIC_DRAW);
	mScaleDataVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, scales.size() * sizeof(float), scales.data(), GL_DYNAMIC_DRAW);
	mTextureLocVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, textureLocs.size() * sizeof(ci::vec2), textureLocs.data(), GL_DYNAMIC_DRAW);

	// we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
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


bool ParticleBackground::particleOutOfBounds(Particle& p){
	if(p.mAge > p.mLifespan 
	   || p.mPosition.x < -p.mAge * mParticleScale * 2.0f 
	   || p.mPosition.y < -p.mAge * mParticleScale * 2.0f 
	   || p.mPosition.x > mEngine.getWorldWidth() + p.mAge * mParticleScale * 2.0f
	   || p.mPosition.y > mEngine.getWorldHeight() + p.mAge * mParticleScale * 2.0f){
		return true;
	}

	return false;
}


void ParticleBackground::resetParticle(Particle& p){
	float ageMin = mGlobals.getAppSettings().getFloat("particles:lifespan_min", 0, 0.0f);
	float ageMax = mGlobals.getAppSettings().getFloat("particles:lifespan_max", 0, 0.0f);
	float minVel = mGlobals.getAppSettings().getFloat("particles:min_vel", 0, 0.0f);
	float maxVel = mGlobals.getAppSettings().getFloat("particles:max_vel", 0, 0.0f);
	p.mPosition.x = ci::randFloat(0, mEngine.getWorldWidth());
	p.mPosition.y = ci::randFloat(0.0f, mEngine.getWorldHeight());
	p.mVelocity.x = ci::randFloat(minVel, maxVel);
	p.mVelocity.y = ci::randFloat(minVel, maxVel);
	p.mLifespan = ci::randFloat(ageMin, ageMax);
	p.mIsDead = false;
	p.mAge = 0.0f;
}

void ParticleBackground::onUpdateServer(const ds::UpdateParams& p){
	float ageMin = mGlobals.getAppSettings().getFloat("particles:lifespan_min", 0, 0.0f);
	float ageMax = mGlobals.getAppSettings().getFloat("particles:lifespan_max", 0, 0.0f);
	float minVel = mGlobals.getAppSettings().getFloat("particles:min_vel", 0, 0.0f);
	float maxVel = mGlobals.getAppSettings().getFloat("particles:max_vel", 0, 0.0f);

	float touchRad = mGlobals.getAppSettings().getFloat("particles:touch_radius", 0, 400.0f);
	float touchPow = mGlobals.getAppSettings().getFloat("particles:touch_power", 0, 4.0f);
	float overallSpeed = mGlobals.getAppSettings().getFloat("particles:anim_speed", 0, 1.0f);
	float audioSpeed = mGlobals.getAppSettings().getFloat("particles:audio_anim_speed", 0, 1.0f);
	float counterMultiplier = mGlobals.getAppSettings().getFloat("particles:counter_multiplier", 0, 10.0f);
	float perlinScale = mGlobals.getAppSettings().getFloat("particles:perlin_scale", 0, 0.001f);
	float friction = mGlobals.getAppSettings().getFloat("particles:friction", 0, 0.99f);
	float thresh = mGlobals.getAppSettings().getFloat("particles:lum_threshold", 0, 0.99f);
	mAnimationCounter += p.getDeltaTime() * counterMultiplier;

	std::vector<ci::vec2> touchRespulsors;
	/*
	if(mVideoBacky){
		unsigned char * dat = mVideoBacky->getRawVideoData();
		if(dat){
			ci::Channel8u yChannel(mVideoBacky->getWidth(), mVideoBacky->getHeight(), mVideoBacky->getWidth(), 1, dat);
			auto surf = ci::Surface8u::create(yChannel);
			ci::Area area(0, 0, mVideoBacky->getWidth(), mVideoBacky->getHeight());
			ci::Surface::Iter iter = surf->getIter(area);
			bool keepLooking = true;
			while(iter.line()) {
				while(iter.pixel()) {
					if(iter.r() > thresh){
						touchRespulsors.push_back(iter.getPos());
						if(touchRespulsors.size() > 100){
							keepLooking = false;
							break;
						}
						

						bool found = false;
						for(auto it = mParticles.begin(); it < mParticles.end(); ++it){
						Particle& party = (*it);
						if(party.mIsDead){
						party.mVelocity.x = ci::randFloat(minVel, maxVel);
						party.mVelocity.y = ci::randFloat(minVel, maxVel);
						party.mLifespan = ci::randFloat(ageMin, ageMax);
						party.mPosition = iter.getPos();
						party.mIsDead = false;
						party.mAge = 0.0f;
						found = true;
						break;
						}
						}

						if(!found){
						keepLooking = false;
						break;
						}
						
					}
				}

				if(!keepLooking){
					break;
				}


			}
			
		}
	}
	*/
	
	/*
	for(auto it : mGlobals.mAllData.mTouchPoints){
		ci::vec2 curPoint = it.second;
		if(curPoint.x < 0.0f || curPoint.y < 0.0f) continue;
		touchRespulsors.push_back(curPoint);
	}
	*/

	if(!mInstanceDataVbo || !mOpacityVbo || !mTextureLocVbo || !mScaleDataVbo) return;

	float vol = mMonitorSpectralNode->getVolume();
	mParticleScale = mGlobals.getAppSettings().getFloat("audio:boost", 0, 10.0f) * vol * vol;
	audioSpeed *= vol;

	float ww = mEngine.getWorldWidth();
	float wh = mEngine.getWorldHeight();


	ci::vec3* positions = (ci::vec3*)mInstanceDataVbo->mapReplace();
	float* opacities = (float*)mOpacityVbo->mapReplace();
	ci::vec2* texLocs = (ci::vec2*)mTextureLocVbo->mapReplace();
	float *scales = (float*)mScaleDataVbo->mapReplace();

	for(auto it = mParticles.begin(); it < mParticles.end(); ++it){
		Particle& party = (*it);

		if(particleOutOfBounds(party)){
			resetParticle(party);

		//	party.mIsDead = true;
		}

		/*
		if(party.mIsDead){
			*positions++ = ci::vec3();
			*opacities++ = 0.0f;
			*texLocs++ = ci::vec2();
			*scales++ = 0.0f;
			continue;
		}
		*/
		party.mPosition += party.mVelocity;
		party.mAge += p.getDeltaTime();

		float xN = mPerlin.fBm(ci::vec3(party.mPosition.x + mAnimationCounter * 10.0f, party.mPosition.y, mAnimationCounter) * perlinScale);
		party.mVelocity.x += xN * overallSpeed * audioSpeed;
		party.mVelocity.y += xN * overallSpeed * audioSpeed;
		party.mVelocity *= friction;


		/*
		for(auto repSprit : mRepulsors){

			float xDelt = fabs(party.mPosition.x - repSprit.mCenter.x) / repSprit.mWid;
			float yDelt = fabs(party.mPosition.y - repSprit.mCenter.y) / repSprit.mHid;
			if(xDelt < 1.0f && yDelt < 1.0f){

				if(party.mPosition.x < repSprit.mCenter.x){
					party.mVelocity.x -= xDelt / repulsorFactor;
				} else {
					party.mVelocity.x += xDelt / repulsorFactor;
				}

				if(party.mPosition.y < repSprit.mCenter.y){
					party.mVelocity.y -= yDelt / repulsorFactor;
				} else {
					party.mVelocity.y += yDelt / repulsorFactor;
				}
			}
		}

		for(auto repTouch : touchRespulsors){
			float disty = glm::distance(party.mPosition, repTouch);
			if(disty < touchRad){
				float power = (1.0f - disty / touchRad) * touchPow;
				float xDelt = repTouch.x - party.mPosition.x;
				float yDelt = repTouch.y - party.mPosition.y;
				party.mVelocity.x += xDelt * power;
				party.mVelocity.y += yDelt * power;

			}

			}
			*/


		*positions++ = ci::vec3(party.mPosition, 0.0f);

		*texLocs++ = ci::vec2(party.mPosition.x / ww, party.mPosition.y / wh);
		

		float squareMag = mParticleScale * (party.mLifespan - party.mAge) / 2.0f;
		*opacities++ = party.mAge / party.mLifespan;
		*scales++ = squareMag;
		
	}

	mOpacityVbo->unmap();
	mInstanceDataVbo->unmap();
	mScaleDataVbo->unmap();
	mTextureLocVbo->unmap();
}

void ParticleBackground::drawLocalClient(){
	if(mBatchRef) {
		if(mVideoBacky && mVideoBacky->getFinalOutTexture()){
			mVideoBacky->getFinalOutTexture()->bind();
		} else if(mImageTexture){
			mImageTexture->bind();
		}

		//ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mEngine.getWorldWidth(), mEngine.getWorldHeight()));

		mBatchRef->drawInstanced(mParticles.size());

		if(mVideoBacky && mVideoBacky->getFinalOutTexture()){
			mVideoBacky->getFinalOutTexture()->unbind();
		} else if(mImageTexture){
			mImageTexture->unbind();
		}
	}
}

} // namespace mv

