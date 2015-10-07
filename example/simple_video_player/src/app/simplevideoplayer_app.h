#ifndef _SIMPLEVIDEOPLAYER_APP_H_
#define _SIMPLEVIDEOPLAYER_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/ui/sprite/video.h"
#include "ds/ui/button/sprite_button.h"
#include "ds/ui/sprite/text.h"

namespace example {
class AllData;

class SimpleVideoPlayer : public ds::App {
public:
	SimpleVideoPlayer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
	void				startVideos(const std::vector<std::string> vidPaths);
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	ds::ui::SpriteButton*		mStressTestButton;
	ds::ui::Text*				mStressText;
	bool						mStressTesting;

	void						fitVideoInArea(ci::Rectf area, ds::ui::Video* video);

	std::vector<ds::ui::Video*>	mVideos;
	std::vector<std::string>	mVideoPaths;

	ds::ui::Text*				mFpsDisplay;
};

} // !namespace example

#endif // !_SIMPLEVIDEOPLAYER_APP_H_