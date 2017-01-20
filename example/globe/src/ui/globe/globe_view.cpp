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
	, mGlobeMesh(ds::ui::Sprite::makeAlloc<ds::ui::Mesh>([&g]()->ds::ui::Mesh*{return new ds::ui::Mesh(g.mEngine); }, this))
	, mTouchGrabber(ds::ui::Sprite::makeAlloc<ds::ui::Sprite>([&g]()->ds::ui::Sprite*{return new ds::ui::Sprite(g.mEngine); }, this))
	, mXMomentum(g)
	, mYMomentum(g)
{

	const int meshResolution = mGlobals.getSettingsLayout().getInt("globe:mesh_resolution", 0, 50);
	const float radius = mGlobals.getSettingsLayout().getFloat("globe:radius", 0, 450.0f);

	mGlow = new Sprite(g.mEngine);
	mGlow->setBaseShader(ds::Environment::getAppFolder("data/shaders"), "planet_glow");
	mGlow->setSize(radius*2.9f, radius*2.9f);
	mGlow->setTransparent(false);
	mGlow->setColor(ci::Color::hex(0xccddff));
	mGlow->getUniform().setVec4f("size", ci::vec4(mGlow->getWidth(), mGlow->getHeight(), 0, 0));
	addChild(*mGlow);
	mGlow->sendToBack();

	mGlobeMesh.setImageFile(ds::Environment::expand("%APP%/data/images/globe/nasa_earth_day.png"), ds::ui::Image::IMG_ENABLE_MIPMAP_F);
	mGlobeMesh.setBaseShader(ds::Environment::getAppFolder("data/shaders"), "planet");
	mGlobeMesh.setSphereMesh(1.0f, meshResolution * 2, meshResolution);
	mGlobeMesh.setScale(radius, radius, radius);

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

void GlobeView::drawClient(const ci::mat4 &trans, const ds::DrawParams &drawParams){
	if(visible()) {
		mGlobeMesh.setRotation(ci::vec3(mGlobeMesh.getRotation().x + mXMomentum.getDelta(), mGlobeMesh.getRotation().y + 0.1f + mYMomentum.getDelta(), 0.0f));
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(0);
		mGlobeMesh.getUniform().setVec4f("camPos", ci::vec4(p.mPosition, 1.0f));
		auto modelMat = mGlobeMesh.getGlobalTransform();
		mGlobeMesh.getUniform().setMatrix44f("modelMatrix", modelMat);
	}

	ds::ui::Sprite::drawClient(trans, drawParams);
}

} // namespace globe_example
