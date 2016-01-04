#pragma once
#ifndef _TRIANGLE_MAN_APP_UI_TAPESTRY_TAPESTRY_VIEW
#define _TRIANGLE_MAN_APP_UI_TAPESTRY_TAPESTRY_VIEW


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/image.h>
#include <ds/thread/serial_runnable.h>
#include <Poco/Runnable.h>
#include "cinder/Surface.h"


namespace nwm {

class Globals;

/**
* \class nwm::TapestryView
*			Async load an image and make some triangles from it
*/
class TapestryView : public ds::ui::Sprite  {
public:
	TapestryView(Globals& g);

private:

	class SurfaceLoader : public Poco::Runnable {
	public:
		SurfaceLoader();

		virtual void					run();
		ci::Surface8u					mImageSurface;
		std::string						mImagePath;

	};


	void								onAppEvent(const ds::Event&);

	virtual void						updateServer(const ds::UpdateParams& p);

	void								animateOn();
	void								animateOff();

	void								setImage(const std::string& filename);
	void								onSurfaceQuery(SurfaceLoader&);

	Globals&							mGlobals;

	ds::EventClient						mEventClient;
	ds::SerialRunnable<SurfaceLoader>	mSurfaceQuery;
	ci::Surface8u						mImageSurface;

	// to layer on top
	ds::ui::Image*						mImage;

};

} // namespace nwm

#endif
