#ifndef _LAYOUT_EXAMPLE_APP_H_
#define _LAYOUT_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace example {
class AllData;

class layout_example : public ds::App {
public:
	layout_example();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

};

} // !namespace example

#endif // !_LAYOUT_EXAMPLE_APP_H_

