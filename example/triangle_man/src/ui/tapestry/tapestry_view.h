#pragma once
#ifndef _TRIANGLE_MAN_APP_UI_TAPESTRY_TAPESTRY_VIEW
#define _TRIANGLE_MAN_APP_UI_TAPESTRY_TAPESTRY_VIEW


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/image.h>
#include <ds/thread/serial_runnable.h>
#include <Poco/Runnable.h>
#include "cinder/Surface.h"
#include <cinder/gl/Vbo.h>


namespace nwm {

class Globals;

/**
* \class nwm::TapestryView
*			Async load an image and make some triangles from it
*/
class TapestryView : public ds::ui::Sprite  {
public:

	struct TriangleSettings {

		TriangleSettings() : mScale(55.0f), mRows(16), mCols(64), mXOffset(30.0f), mYOffset(0.0f)
			, mRandMaxMin(0.025f), mRandMaxMax(0.075f), mRandVelMin(0.00005f), mRandVelMax(0.0001f), mRandChance(0.99f),
			mTouchMaxMin(0.2f), mTouchMaxMax(0.3f), mTouchDist(300.0f), mTouchVelDenom(50.0f), mDepreciation(0.0001f){}

		float	mScale;
		int		mRows;
		int		mCols;
		float	mXOffset;
		float	mYOffset;
		float	mRandVelMin;
		float	mRandVelMax;
		float	mRandMaxMin;
		float	mRandMaxMax;
		float	mRandChance;
		float	mTouchMaxMin;
		float	mTouchMaxMax;
		float	mTouchDist;
		float	mTouchVelDenom;
		float	mDepreciation;
	};
	
	TapestryView(Globals& g);


	void								setTriangleSettings(TriangleSettings& ts);
	void								setImage(const std::string& filename);

private:

	/// Async image loading
	class SurfaceLoader : public Poco::Runnable {
	public:
		SurfaceLoader();

		virtual void					run();
		ci::Surface8u					mImageSurface;
		std::string						mImagePath;

	};

	void								onSurfaceQuery(SurfaceLoader&);

	/// Triangle effects
	class TriangleThingy {
	public:
		TriangleThingy(ci::vec2 loc, int colorIndex, float curAlpha, float maxAlpha)
			: mCurAlpha(curAlpha), mVelocity(0.0f), mLocation(loc), mColorIndex(colorIndex), mGrowing(false), mMaxAlpha(maxAlpha)
		{};

		ci::vec2	mLocation;
		int			mColorIndex;
		float		mCurAlpha;
		float		mVelocity;
		bool		mGrowing;
		float		mMaxAlpha;
	};

	std::vector<ci::ColorA>				mColors;
	std::vector<TriangleThingy>			mThingies;
	bool								mInitialized;
	TriangleSettings					mTriangleSettings;
	ci::gl::VboMesh						mMesh;


	void								onAppEvent(const ds::Event&);

	virtual void						updateServer(const ds::UpdateParams& p);
	virtual void						drawLocalClient();
	void								initialize();

	void								animateOn();
	void								animateOff();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;
	ds::SerialRunnable<SurfaceLoader>	mSurfaceQuery;
	ci::Surface8u						mImageSurface;

	// to layer on top
	ds::ui::Image*						mImage;

};

} // namespace nwm

#endif


