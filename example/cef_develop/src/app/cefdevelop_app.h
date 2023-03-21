#ifndef _CEFDEVELOP_APP_H_
#define _CEFDEVELOP_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

#include <ds/ui/sprite/web.h>

namespace cef {
class AllData;

class CefDevelop : public ds::App {
  public:
	CefDevelop();

	virtual void onKeyUp(ci::app::KeyEvent event) override;
	virtual void onKeyDown(ci::app::KeyEvent event) override;
	void		 setupServer();

	virtual void fileDrop(ci::app::FileDropEvent event);

  private:
	typedef ds::App inherited;

	// Data
	AllData mAllData;

	// Data acquisition
	Globals		 mGlobals;
	QueryHandler mQueryHandler;

	ds::ui::Web* mWebby;
};

} // namespace cef

#endif // !_CEFDEVELOP_APP_H_