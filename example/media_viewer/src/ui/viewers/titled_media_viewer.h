#pragma once
#ifndef _MEDIAVIEWER_APP_UI_VIEWERS_TITLED_MEDIA_VIEWER
#define _MEDIAVIEWER_APP_UI_VIEWERS_TITLED_MEDIA_VIEWER

#include <ds/ui/media/media_viewer.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/multiline_text.h>

#include "model/generated/media_model.h"


namespace mv {

class Globals;

/**
* \class mv::TitledMediaViewer
*			A single media viewer with a title and a close button
*/
class TitledMediaViewer : public ds::ui::MediaViewer {
public:
	TitledMediaViewer(Globals& g);


	void								setMedia(ds::model::MediaRef media);
	void								animateOn();
	void								animateOff();

private:

	virtual void						onLayout();

	Globals&							mGlobals;

	ds::ui::Sprite*						mBackground;
	ds::ui::MultilineText*				mTitle;
	ds::ui::MultilineText*				mBody;


};

} // namespace mv

#endif
