#ifndef _GLOBEEXAMPLE_APP_H_
#define _GLOBEEXAMPLE_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"

namespace globe_example {
class AllData;

class GlobeExample : public ds::App {
public:
	GlobeExample();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;

	// Data acquisition
	Globals				mGlobals;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace globe_example

#endif // !_GLOBEEXAMPLE_APP_H_