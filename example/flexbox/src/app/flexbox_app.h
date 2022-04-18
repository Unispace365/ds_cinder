#pragma once

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace ds {

class flexbox_app : public ds::App {
public:
	flexbox_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace ds
