#ifndef _FULLSTARTER_APP_H_
#define _FULLSTARTER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace fullstarter {
class AllData;

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

#endif // !_FULLSTARTER_APP_H_