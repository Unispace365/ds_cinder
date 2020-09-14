#pragma once

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "query/query_handler.h"

namespace downstream {

class waffles_data_viewer_app : public ds::App {
public:
	waffles_data_viewer_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
	QueryHandler		mQueryHandler;
};

} // !namespace downstream
