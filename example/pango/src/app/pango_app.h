#ifndef _PANGO_APP_H_
#define _PANGO_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"
#include "query/query_handler.h"


namespace pango {
class AllData;

class PangoApp : public ds::App {
public:
	PangoApp();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);

private:

	void				onAppEvent(const ds::Event&);

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	// App events can be handled here
	ds::EventClient		mEventClient;

};

} // !namespace pango

#endif // !_PANGO_APP_H_

