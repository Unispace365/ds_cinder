#ifndef _PARTICLE_EFFECTS_APP_H_
#define _PARTICLE_EFFECTS_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

namespace example {
class AllData;

class particle_effects_app : public ds::App {
  public:
	particle_effects_app();

	void setupServer();

	virtual void fileDrop(ci::app::FileDropEvent event) override;

  private:
	// App events can be handled here
	ds::EventClient mEventClient;
};

} // namespace example

#endif // !_PARTICLE_EFFECTS_APP_H_
