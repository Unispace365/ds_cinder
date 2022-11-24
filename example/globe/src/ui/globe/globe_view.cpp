#include "globe_view.h"

#include "cinder/CinderGlm.h"
#include <cinder/CinderMath.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "app/globals.h"
#include "pins/globe_pin.h"
#include "pins/globe_pin_manager.h"
#include <cinder/app/App.h>

#include <cinder/Rand.h>

namespace ds { namespace ui {

	GlobeView::GlobeView(SpriteEngine& e, float minTilt, float maxTilt, float radius, int resolution)
	  : ds::ui::Sprite(e)
	  , mEngine(e)
	  , mMinTilt(minTilt)
	  , mMaxTilt(maxTilt)
	  , mPinManager(ds::ui::Sprite::makeAlloc<ds::ui::GlobePinManager>(
			[this]() -> ds::ui::GlobePinManager* { return new ds::ui::GlobePinManager(mEngine); }, this))
	  , mTouchGrabber(ds::ui::Sprite::makeAlloc<ds::ui::Sprite>(
			[this]() -> ds::ui::Sprite* { return new ds::ui::Sprite(mEngine); }, this))
	  , mXMomentum(e)
	  , mYMomentum(e)
	  , mGlobeRadius(radius)
	  , mMeshResolution(resolution)
	  , mTouchWaitOrbit(3.0f)
	  , mFocusOffset(ci::vec2(0.0f, 0.0f)) {

		mLastTouched = -mTouchWaitOrbit;

		mPinManager.updateRadius(mGlobeRadius);

		setTransparent(false);
		setPosition(0.0f, 0.0f, mGlobeRadius);

		// Load the textures for the Earth.
		std::string earthDiffuse = ds::Environment::expand("%APP%/data/images/globe/earthDiffuse.png");
		std::string nightDiffuse = ds::Environment::expand("%APP%/data/images/globe/earthDiffuseNight.png");

		std::string earthMask	= ds::Environment::expand("%APP%/data/images/globe/earthMask.png");
		std::string earthNormal = ds::Environment::expand("%APP%/data/images/globe/earthNormal.png");
		auto		fmt			= ci::gl::Texture2d::Format().wrap(GL_REPEAT);
		mTexDiffuse				= ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthDiffuse)), fmt);
		mTexNormal				= ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthNormal)), fmt);
		mTexMask				= ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthMask)), fmt);
		mTexNight				= ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(nightDiffuse)), fmt);

		// Create the Earth mesh with a custom shader.
		std::string vertProg = ds::Environment::expand("%APP%/data/shaders/earth.vert");
		std::string fragProg = ds::Environment::expand("%APP%/data/shaders/earth.frag");
		mEarthShader		 = ci::gl::GlslProg::create(ci::loadFile(vertProg), ci::loadFile(fragProg));
		mEarthShader->uniform("texDiffuse", 0);
		mEarthShader->uniform("texDiffuseNight", 1);
		mEarthShader->uniform("texNormal", 2);
		mEarthShader->uniform("texMask", 3);
		mEarthShader->uniform("lightDir", glm::normalize(ci::vec3(0.025f, 0.35f, 1.0f)));
		mEarth =
			ci::gl::Batch::create(ci::geom::Sphere().radius(mGlobeRadius).subdivisions(mMeshResolution), mEarthShader);

		mOccluder = ci::gl::Batch::create(ci::geom::Circle().radius(mGlobeRadius + 40),
										  ci::gl::getStockShader(ci::gl::ShaderDef().color()));

		mTouchGrabber.setCenter(0.5f, 0.5f);
		mTouchGrabber.setPosition(ci::vec3(0.0f, 0.0f, 100.0f));
		mTouchGrabber.setSize(mEngine.getWorldWidth() / 2.28f, mEngine.getWorldHeight());
		mTouchGrabber.enable(true);
		mTouchGrabber.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mTouchGrabber.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
			mLastTouched = ci::app::getElapsedSeconds();

			if (ti.mPhase == ds::ui::TouchInfo::Added && ti.mNumberFingers == 1) {
				mXMomentum.clear();
				mYMomentum.clear();
			}

			const float touchMultiplier = 5.0;

			// X & Y are transposed intentionally, drag left / right and affect the y rotation axis
			mXMomentum.addDeltaPosition(ti.mDeltaPoint.y / touchMultiplier);
			mYMomentum.addDeltaPosition(ti.mDeltaPoint.x / touchMultiplier);
		});

		{
			std::string vertProg = ds::Environment::expand("%APP%/data/shaders/planet_glow.vert");
			std::string fragProg = ds::Environment::expand("%APP%/data/shaders/planet_glow.frag");
			mGlowShader			 = ci::gl::GlslProg::create(ci::loadFile(vertProg), ci::loadFile(fragProg));
			mGlow = ci::gl::Batch::create(ci::geom::Plane().size(ci::vec2(mGlobeRadius * 3.4, mGlobeRadius * 3.4)),
										  mGlowShader);
		}
	}


	void GlobeView::onUpdateServer(const ds::UpdateParams& updateParams) {

		float orbit = 0.0f;

		if (ci::app::getElapsedSeconds() - mLastTouched > mTouchWaitOrbit) {
			orbit = 5.0f * updateParams.getDeltaTime();
		}


		// Limit xRotation;
		auto rot =
			ci::vec3(mGlobeRotation.x + mXMomentum.getDelta(), mGlobeRotation.y + orbit + mYMomentum.getDelta(), 0.0f);

		if (rot.x < mMinTilt || rot.x > mMaxTilt) {
			mXMomentum.clear();
			rot.x = ci::constrain(rot.x, mMinTilt, mMaxTilt);
		}

		mGlobeRotation = ci::vec3((rot.x), (rot.y), 0.0);

		mPinManager.updateRotation(mGlobeRotation.x, mGlobeRotation.y);
		mEarthShader->uniform("time", (float)ci::app::getElapsedSeconds());
	}


	void GlobeView::setFocusOffset(float latitude, float longitude) {
		mFocusOffset.x = latitude;
		mFocusOffset.y = longitude;
	}

	void GlobeView::rotateTo(float latitude, float longitude) {

		longitude += 180.0f + mFocusOffset.y;
		latitude += mFocusOffset.x;

		float phi	= -longitude;
		float theta = (latitude / 180.0f) * 90.0f;

		mTweenEndRotation.y = phi;
		mTweenEndRotation.x = theta * 2.0f;

		//	if (mTweenEndRotation.x > mMaxTilt) {
		//		mTweenEndRotation.x = mMaxTilt;
		//	} else if (mTweenEndRotation.x < mMinTilt) {
		//		mTweenEndRotation.x = mMinTilt;
		//	}

		mTweenStartRotation = mGlobeRotation;
		// Clamp original rotation to something reasonable
		mTweenStartRotation.x = fmod(mTweenStartRotation.x, 360.0f);
		mTweenStartRotation.y = fmod(mTweenStartRotation.y, 360.0f);
		mTweenStartRotation.z = fmod(mTweenStartRotation.z, 360.0f);

		// check what the fastest way around the globe is
		float odist = abs(mTweenStartRotation.y - mTweenEndRotation.y);
		float pdist = abs(mTweenEndRotation.y - (mTweenStartRotation.y + 360.0f));
		float ndist = abs(mTweenEndRotation.y - (mTweenStartRotation.y - 360.0f));

		if (odist < pdist && odist < ndist) {
			// do nothing
		} else if (pdist < odist && pdist < ndist) {
			mTweenStartRotation.y += 360.0f;
		} else if (ndist < odist && ndist < pdist) {
			mTweenStartRotation.y -= 360.0f;
		}

		// Start tweenin'
		mIsTweeningRotation = true;
		//		tweenAmnt = 0.0f;
		//		mTimeSinceLastTouch = 0.0f;


		mGlobeRotation = mTweenEndRotation;

		mXMomentum.clear();
		mYMomentum.clear();
	}

	void GlobeView::addPin(ds::ui::GlobePin* gp) {
		mPinManager.addPin(gp);
	}

	void GlobeView::drawLocalClient() {
		Sprite::drawLocalClient();
		if (!mEarth || !mGlow) return;

		// Translate and draw the globe
		{

			ci::gl::colorMask(true, true, true, true);

			ci::gl::ScopedMatrices();
			ci::gl::ScopedModelMatrix();

			ci::gl::rotate(ci::toRadians(mGlobeRotation.x), 1.0, 0.0, 0.0);
			ci::gl::rotate(ci::toRadians(mGlobeRotation.y), 0.0, 1.0, 0.0);

			ci::gl::ScopedFaceCulling cull(true, GL_BACK);
			ci::gl::ScopedTextureBind tex0(mTexDiffuse, 0);
			ci::gl::ScopedTextureBind tex1(mTexNight, 1);
			ci::gl::ScopedTextureBind tex2(mTexNormal, 2);
			ci::gl::ScopedTextureBind tex3(mTexMask, 3);
			ci::gl::enableDepthRead(true);
			ci::gl::enableDepthWrite(true);
			mEarth->draw();
		}

		{

			ci::gl::enableDepthRead(true);
			ci::gl::enableDepthWrite(false);

			ci::gl::ScopedMatrices();
			ci::gl::ScopedModelMatrix();

			ci::gl::rotate(ci::toRadians(-mGlobeRotation.y), 0.0, 1.0, 0.0);
			ci::gl::rotate(ci::toRadians(-mGlobeRotation.x + 90), 1.0, 0.0, 0.0);
			ci::gl::translate(ci::vec3(0.0f, 0.0f, 5.0f));
			ci::gl::ScopedColor(ci::Color(0, 0, 1.0));
			mGlow->draw();
		}


		auto  gw = getWidth();
		float gh = getHeight();
	}

	void GlobeView::clearFilters() {
		mPinManager.clearFilters();
	}

	void GlobeView::addFilter(int f) {
		mPinManager.addFilter(f);
	}

	void GlobeView::removeFilter(int f) {
		mPinManager.removeFilter(f);
	}


}} // namespace ds::ui
