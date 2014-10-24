#ifndef _INTERFACEXMLIMPORTEREXAMPLEAPP_APP_H_
#define _INTERFACEXMLIMPORTEREXAMPLEAPP_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"

namespace importer_example {
class AllData;

class InterfaceXmlImporterExampleApp : public ds::App {
public:
	InterfaceXmlImporterExampleApp();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;

	// Data acquisition
	Globals				mGlobals;
};

} // !namespace importer_example

#endif // !_INTERFACEXMLIMPORTEREXAMPLEAPP_APP_H_