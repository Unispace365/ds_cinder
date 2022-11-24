#ifndef _GLITCH_SHADERS_APP_H_
#define _GLITCH_SHADERS_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"

namespace downstream {
class AllData;

class glitch_shaders_app : public ds::App {
  public:
	glitch_shaders_app();

	virtual void onKeyDown(ci::app::KeyEvent event) override;
	void		 setupServer();

	virtual void fileDrop(ci::app::FileDropEvent event) override;

  private:
	void onAppEvent(const ds::Event&);

	// Data
	AllData mAllData;

	// Data acquisition
	Globals mGlobals;

	// App events can be handled here
	ds::EventClient mEventClient;
};

} // namespace downstream

#endif // !_GLITCH_SHADERS_APP_H_
