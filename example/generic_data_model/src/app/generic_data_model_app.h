#ifndef _GENERIC_DATA_MODEL_APP_H_
#define _GENERIC_DATA_MODEL_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/storage/directory_watcher.h>

#include "app/globals.h"
#include "query/data_wrangler.h"

namespace downstream {
class AllData;

class generic_data_model_app : public ds::App {
public:
	generic_data_model_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	void				onAppEvent(const ds::Event&);

	DataWrangler		mDataWrangler;

	ds::DirectoryWatcher mDirectoryWatcher;

	// Data acquisition
	Globals				mGlobals;

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_GENERIC_DATA_MODEL_APP_H_
