#ifndef _SIMPLEVIDEOPLAYER_APP_H_
#define _SIMPLEVIDEOPLAYER_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/ui/sprite/video.h"

namespace example {
class AllData;

class SimpleVideoPlayer : public ds::App {
public:
	SimpleVideoPlayer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
	void				startVideo(const std::string& vidPath);
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	ds::ui::Video*		mVideo;
};

} // !namespace example

#endif // !_SIMPLEVIDEOPLAYER_APP_H_