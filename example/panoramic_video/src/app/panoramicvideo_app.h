#ifndef _PANORAMICVIDEO_APP_H_
#define _PANORAMICVIDEO_APP_H_

#include <cinder/Rand.h>
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "ds/ui/menu/touch_menu.h"
#include "query/query_handler.h"

namespace panoramic {
class AllData;

class PanoramicVideo : public ds::App {
  public:
	PanoramicVideo();

	void setupServer();

  private:
	typedef ds::App inherited;

	// Data
	AllData mAllData;

	// Data acquisition
	Globals		 mGlobals;
	QueryHandler mQueryHandler;

	ds::ui::TouchMenu* mTouchMenu;
};

} // namespace panoramic

#endif // !_PANORAMICVIDEO_APP_H_
