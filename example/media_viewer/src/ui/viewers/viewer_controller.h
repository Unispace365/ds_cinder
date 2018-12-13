#pragma once
#ifndef _MEDIAVIEWER_APP_UI_VIEWERS_VIEWER_CONTROLLER
#define _MEDIAVIEWER_APP_UI_VIEWERS_VIEWER_CONTROLLER


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>

#include "model/generated/media_model.h"

namespace mv {

class Globals;
class TitledMediaViewer;

/**
* \class mv::ViewerController
*			Manages and mediates all media viewers
*/
class ViewerController : public ds::ui::Sprite  {
	public:
		ViewerController(Globals& g);

		void								addViewer(ds::model::MediaRef newMedia, const ci::vec3 location, const float startWidth, const bool openGl, const bool nvDecode);

		// immediately releases the viewer with no animation
		void								removeViewer(TitledMediaViewer* viewer);

		// tries to evenly space everything
		void								layoutViewers();

	private:
		void								onAppEvent(const ds::Event&);
		void								animateViewerOff(TitledMediaViewer* viewer, const float delayey);
		void								createGridLayout(const ci::Rectf area, const int numItems, std::vector<ci::vec2>& positions);

		virtual void						onUpdateServer(const ds::UpdateParams& updateParams);

		typedef ds::ui::Sprite				inherited;
		Globals&							mGlobals;

		ds::EventClient						mEventClient;
		
		std::vector<TitledMediaViewer*>		mViewers;
		ds::ui::Text*						mGstFps;

	};

} // namespace mv

#endif


