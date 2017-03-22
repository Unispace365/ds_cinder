#include "globe_view.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

namespace globe_example {

GlobeView::GlobeView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mTouchGrabber(ds::ui::Sprite::makeAlloc<ds::ui::Sprite>([&g]()->ds::ui::Sprite*{return new ds::ui::Sprite(g.mEngine); }, this))
	, mXMomentum(g)
	, mYMomentum(g)
{

	const int meshResolution = mGlobals.getSettingsLayout().getInt("globe:mesh_resolution", 0, 50);
	const float radius = mGlobals.getSettingsLayout().getFloat("globe:radius", 0, 450.0f);

	setTransparent(false);
	setPosition(0.0f, 0.0f, radius);

	// Load the textures for the Earth.
	std::string earthDiffuse = ds::Environment::expand("%APP%/data/images/globe/earthDiffuse.png");
	std::string earthMask = ds::Environment::expand("%APP%/data/images/globe/earthMask.png");
	std::string earthNormal = ds::Environment::expand("%APP%/data/images/globe/earthNormal.png");
	auto fmt = ci::gl::Texture2d::Format().wrap(GL_REPEAT).mipmap().minFilter(GL_LINEAR_MIPMAP_LINEAR);
	mTexDiffuse = ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthDiffuse)), fmt);
	mTexNormal = ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthNormal)), fmt);
	mTexMask = ci::gl::Texture2d::create(ci::loadImage(ci::loadFile(earthMask)), fmt);

	// Create the Earth mesh with a custom shader.
	std::string vertProg = ds::Environment::expand("%APP%/data/shaders/earth.vert");
	std::string fragProg = ds::Environment::expand("%APP%/data/shaders/earth.frag");
	auto earthShader = ci::gl::GlslProg::create(ci::loadFile(vertProg), ci::loadFile(fragProg));
	earthShader->uniform("texDiffuse", 0);
	earthShader->uniform("texNormal", 1);
	earthShader->uniform("texMask", 2);
	earthShader->uniform("lightDir", glm::normalize(ci::vec3(0.025f, 0.25f, 1.0f)));
	mEarth = ci::gl::Batch::create(ci::geom::Sphere().radius(radius).subdivisions(meshResolution), earthShader);

	mTouchGrabber.setCenter(0.5f, 0.5f);
	mTouchGrabber.setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mTouchGrabber.enable(true);
	mTouchGrabber.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mTouchGrabber.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){

		if(ti.mPhase == ds::ui::TouchInfo::Added && ti.mNumberFingers == 1){
			mXMomentum.clear();
			mYMomentum.clear();
		}
		const float touchMultiplier = mGlobals.getSettingsLayout().getFloat("globe:touch_speed_divisor", 0, 45.0f);

		// X & Y are transposed intentionally, drag left / right and affect the y rotation axis
		mXMomentum.addDeltaPosition(ti.mDeltaPoint.y / touchMultiplier);
		mYMomentum.addDeltaPosition(ti.mDeltaPoint.x / touchMultiplier);
	});

}


void GlobeView::updateServer(const ds::UpdateParams& updateParams){
	ds::ui::Sprite::updateServer(updateParams);

	setRotation(ci::vec3(getRotation().x + mXMomentum.getDelta(), getRotation().y + 0.1f + mYMomentum.getDelta(), 0.0f));
	mTouchGrabber.setRotation(-getRotation());
}


void GlobeView::drawLocalClient(){
	if(!mEarth) return;

	ci::gl::ScopedFaceCulling cull(true, GL_BACK);
	ci::gl::ScopedTextureBind tex0(mTexDiffuse, 0);
	ci::gl::ScopedTextureBind tex1(mTexNormal, 1);
	ci::gl::ScopedTextureBind tex2(mTexMask, 2);

	mEarth->draw();
}

} // namespace globe_example
