#include "tapestry_view.h"

#include <Poco/LocalDateTime.h>

#include <cinder/Rand.h>
#include <cinder/gl/gl.h>
#include <cinder/ImageIo.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace nwm {

#define M_SQRT_3_2 0.8660254037844386

TapestryView::SurfaceLoader::SurfaceLoader(){

}

void TapestryView::SurfaceLoader::run(){
	mImageSurface.reset();
	try{
		mImageSurface = ci::Surface8u(ci::loadImage(ds::Environment::expand(mImagePath)));
	} catch(std::exception& e){
		DS_LOG_WARNING("Couldn't load the image thingy " << e.what());
	}
}


TapestryView::TapestryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mImage(nullptr)
	, mSurfaceQuery(g.mEngine, [this]{ return new SurfaceLoader(); })
	, mInitialized(false)
{

	TriangleSettings ts;
	ts.mScale = mGlobals.getSettingsLayout().getFloat("triangles:scale", 0, 55.0f);
	ts.mRows = mGlobals.getSettingsLayout().getInt("triangles:rows", 0, 64);
	ts.mCols = mGlobals.getSettingsLayout().getInt("triangles:cols", 0, 16);
	ts.mXOffset = mGlobals.getSettingsLayout().getFloat("triangles:x_offset", 0, 40.0f);
	ts.mYOffset = mGlobals.getSettingsLayout().getFloat("triangles:y_offset", 0, 40.0f);
	ts.mRandVelMin = mGlobals.getSettingsLayout().getFloat("triangles:rand_vel_min", 0, 0.00005f);
	ts.mRandVelMax = mGlobals.getSettingsLayout().getFloat("triangles:rand_vel_max", 0, 0.0001f);
	ts.mRandMaxMin = mGlobals.getSettingsLayout().getFloat("triangles:rand_max_min", 0, 0.025f);
	ts.mRandMaxMax = mGlobals.getSettingsLayout().getFloat("triangles:rand_max_max", 0, 0.075f);
	ts.mRandChance = mGlobals.getSettingsLayout().getFloat("triangles:rand_chance", 0, 0.99f);
	ts.mTouchMaxMin = mGlobals.getSettingsLayout().getFloat("triangles:touch_max_min", 0, 0.2f);
	ts.mTouchMaxMax = mGlobals.getSettingsLayout().getFloat("triangles:touch_max_max", 0, 0.3f);
	ts.mTouchDist = mGlobals.getSettingsLayout().getFloat("triangles:touch_dist", 0, 300.0f);
	ts.mTouchVelDenom = mGlobals.getSettingsLayout().getFloat("triangles:touch_vel_denom", 0, 50.0f);
	ts.mDepreciation = mGlobals.getSettingsLayout().getFloat("triangles:depreciation", 0, 0.0001f);
	mTriangleSettings = ts;

	hide();
	setOpacity(0.0f);
	setTransparent(false);

	mImage = new ds::ui::Image(mEngine);
	mImage->setOpacity(0.1f);
	//mImage->set
	addChildPtr(mImage);
	mImage->enable(true);
	mImage->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);

	mSurfaceQuery.setReplyHandler([this](SurfaceLoader& l){ onSurfaceQuery(l); });

	setImage("%APP%/data/images/merica.png");
}

void TapestryView::onAppEvent(const ds::Event& in_e){
	
}

void TapestryView::setImage(const std::string& filename) {
	mSurfaceQuery.start([this, filename](SurfaceLoader& sl){ sl.mImagePath = filename; }, false);
	if(mImage){
		mImage->setImageFile(filename);
	}
}

void TapestryView::onSurfaceQuery(SurfaceLoader& l){
	mImageSurface = l.mImageSurface;

	initialize();

	animateOn();
}


void TapestryView::initialize(){

	mColors.clear();
	mThingies.clear();

	if(mImageSurface.getWidth() < 1 || mImageSurface.getHeight() < 1){
		mInitialized = false;
		return;
	}

	mInitialized = true;

	ci::gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticColorsRGBA();

	float theScale = mTriangleSettings.mScale;
	int cols = mTriangleSettings.mCols;
	int rows = mTriangleSettings.mRows;

	mMesh = ci::gl::VboMesh(cols * rows * 3, cols * rows * 3, layout, GL_TRIANGLES);
	std::vector<uint32_t> indices;
	std::vector<ci::Vec3f> positions;

	float w = theScale; // (float)(M_SQRT_3_2 * theScale);
	float h = 1.0f * theScale;
	int index = -1;
	ci::ColorA color1;
	ci::ColorA color2;
	ci::ColorA color3;


	float ww = mEngine.getWorldWidth();
	float wh = mEngine.getWorldHeight();
	float iw = (float)mImageSurface.getWidth();
	float ih = (float)mImageSurface.getHeight();
	float alphaMin = mGlobals.getSettingsLayout().getFloat("triangles:alpha_min", 0, 0.0f);

	float x = 1.0f;

	float xOverflow = mGlobals.getSettingsLayout().getFloat("triangles:x_overflow", 0, 0.0f);
	float yOverflow = mGlobals.getSettingsLayout().getFloat("triangles:y_overflow", 0, 0.0f);

	for(int col = 0; col < cols; col++) {
		x = col * 2 * w + w / 2.0f + mTriangleSettings.mXOffset;
		int direction = -1;// (col % 2) ? 1 : -1;
		for(int row = 0; row < rows; row++) {
			direction *= -1;
			float y = row * h + mTriangleSettings.mYOffset;

			float x1, x2, x3, y1, y2, y3;
			if(direction > 0){
				x1 = x - (w + xOverflow) * direction;
				x2 = x + (w + xOverflow) * direction;
				x3 = x - (w + xOverflow) * direction;
				y1 = y + h / 2.0f - yOverflow;
				y2 = y + h / 2.0f - yOverflow;
				y3 = y + h * 2.5f + yOverflow;
			} else {

				x1 = x - (w + xOverflow) * direction;
				x2 = x + (w + xOverflow) * direction;
				x3 = x - (w + xOverflow) * direction;
				y1 = y - h * 2.5f - yOverflow;
				y2 = y - h / 2.0f + yOverflow;
				y3 = y - h / 2.0f + yOverflow;
			}

			positions.push_back(ci::Vec3f(x1, y1, 0.0f));
			positions.push_back(ci::Vec3f(x2, y2, 0.0f));
			positions.push_back(ci::Vec3f(x3, y3, 0.0f));

// 			positions.push_back(ci::Vec3f(x - w * direction, y - h, 0));
// 			positions.push_back(ci::Vec3f(x + w * direction, y + 0, 0));
// 			positions.push_back(ci::Vec3f(x - w * direction, y + h, 0));

			// TODO: try 3 colors per triangle? 
			// TODO: try averaging 3 samples
			float posXPer1 = iw * x1 / ww;
			float posYPer1 = ih * y1 / wh;
			float posXPer2 = iw * x2 / ww;
			float posYPer2 = ih * y2 / wh;
			float posXPer3 = iw * x3 / ww;
			float posYPer3 = ih * y3 / wh;
			color1 = mImageSurface.getPixel(ci::Vec2i((int)posXPer1, (int)posYPer1));
			color2 = mImageSurface.getPixel(ci::Vec2i((int)posXPer2, (int)posYPer2));
			color3 = mImageSurface.getPixel(ci::Vec2i((int)posXPer3, (int)posYPer3));

			color1.a = alphaMin;
			color2.a = alphaMin;
			color3.a = alphaMin;


			ci::ColorA avColor = ci::ColorA(
				(color1.r + color2.r + color3.r) / 3.0f,
				(color1.g + color2.g + color3.g) / 3.0f,
				(color1.b + color2.b + color3.b) / 3.0f,
				1.0f);

			mColors.push_back(avColor);
			mColors.push_back(avColor);
			mColors.push_back(avColor);

			index += 3;
			indices.push_back(index - 2);
			indices.push_back(index - 1);
			indices.push_back(index);

			mThingies.push_back(TriangleThingy(ci::Vec2f(x, y), index, alphaMin, 1.0f));
		}
	}

	if(mImage){
		mImage->setScale(ww / mImage->getWidth());
	}

	mMesh.bufferIndices(indices);
	mMesh.bufferPositions(positions);
	mMesh.bufferColorsRGBA(mColors);

	setTransparent(false);
}

void TapestryView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f));
}

void TapestryView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void TapestryView::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	if(!mInitialized) return;

	float alphaMin = mGlobals.getSettingsLayout().getFloat("triangles:alpha_min", 0, 0.0f);
	float deltaTime = p.getDeltaTime() * 60.0f;
	for(auto it = mThingies.begin(); it < mThingies.end(); ++it){
		auto& thingy = (*it);

		thingy.mCurAlpha += thingy.mVelocity * deltaTime;
		if(thingy.mCurAlpha < alphaMin) thingy.mCurAlpha = alphaMin;
		if(thingy.mCurAlpha > thingy.mMaxAlpha){
			//thingy.mCurAlpha = thingy.mMaxAlpha;
			if(thingy.mGrowing){
				thingy.mGrowing = false;
			}
		}
		if(!thingy.mGrowing && thingy.mCurAlpha > alphaMin){
			thingy.mVelocity -= mTriangleSettings.mDepreciation * deltaTime;
		}

		mColors[thingy.mColorIndex].a = thingy.mCurAlpha;
		mColors[thingy.mColorIndex - 1].a = thingy.mCurAlpha;
		mColors[thingy.mColorIndex - 2].a = thingy.mCurAlpha;

		if(!thingy.mGrowing && ci::randFloat(0.0f, 1.0f) > mTriangleSettings.mRandChance){
			thingy.mGrowing = true;
			thingy.mVelocity = ci::randFloat(mTriangleSettings.mRandVelMin, mTriangleSettings.mRandVelMax);
			thingy.mMaxAlpha = ci::randFloat(mTriangleSettings.mRandMaxMin, mTriangleSettings.mRandMaxMax);
		}
	}
}


void TapestryView::drawLocalClient(){
	if(!mInitialized) return;

	mMesh.bufferColorsRGBA(mColors);
	ci::gl::draw(mMesh);
}

} // namespace nwm
