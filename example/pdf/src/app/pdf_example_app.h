#ifndef _PDF_EXAMPLE_APP_H_
#define _PDF_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace downstream {

class pdf_example_app : public ds::App {
public:
	pdf_example_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	void addMediaViewer(std::string uri);
	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_PDF_EXAMPLE_APP_H_
