#pragma once

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace fullstarter {

class FullStarterApp : public ds::App {
public:
	FullStarterApp();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace fullstarter