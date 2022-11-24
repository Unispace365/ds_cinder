#ifndef _GLOBEEXAMPLE_APP_H_
#define _GLOBEEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"

namespace globe_example {
class AllData;

class GlobeExample : public ds::App {
  public:
	GlobeExample();

	virtual void onKeyDown(ci::app::KeyEvent event) override;
	void		 setupServer();
	void		 update();

  private:
	// Data acquisition
	Globals mGlobals;


	void moveCamera(const ci::vec3& deltaMove);
};

} // namespace globe_example

#endif // !_GLOBEEXAMPLE_APP_H_