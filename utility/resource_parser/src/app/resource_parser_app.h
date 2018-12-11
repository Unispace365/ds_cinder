#ifndef _RESOURCE_PARSER_APP_H_
#define _RESOURCE_PARSER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace downstream {
class AllData;

class resource_parser_app : public ds::App {
public:
	resource_parser_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_RESOURCE_PARSER_APP_H_
