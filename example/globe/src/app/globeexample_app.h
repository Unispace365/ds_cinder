#ifndef _GLOBEEXAMPLE_APP_H_
#define _GLOBEEXAMPLE_APP_H_

#include <cinder/app/RendererGl.h>
#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

namespace globe_example {
class AllData;

class GlobeExample : public ds::App {
public:
	GlobeExample();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;

	// Data acquisition
	Globals				mGlobals;




	void				moveCamera(const ci::vec3& deltaMove);
};

} // !namespace globe_example

#endif // !_GLOBEEXAMPLE_APP_H_