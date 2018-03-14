#ifndef _GETTING_STARTED_APP_H_
#define _GETTING_STARTED_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace downstream {
class AllData;

class getting_started_app : public ds::App {
public:
	getting_started_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_GETTING_STARTED_APP_H_
