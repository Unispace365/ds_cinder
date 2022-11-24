#ifndef _DRAG_DESTINATION_EXAMPLE_APP_H_
#define _DRAG_DESTINATION_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace example {
class AllData;

class drag_destination_example : public ds::App {
  public:
	drag_destination_example();

	void setupServer();

  private:
	// Data
	AllData mAllData;

	// Data acquisition
	Globals		 mGlobals;
	QueryHandler mQueryHandler;
};

} // namespace example

#endif // !_DRAG_DESTINATION_EXAMPLE_APP_H_