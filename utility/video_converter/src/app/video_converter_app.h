#ifndef _VIDEO_CONVERTER_APP_H_
#define _VIDEO_CONVERTER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace downstream {
class AllData;

class video_converter_app : public ds::App {
public:
	video_converter_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_VIDEO_CONVERTER_APP_H_
