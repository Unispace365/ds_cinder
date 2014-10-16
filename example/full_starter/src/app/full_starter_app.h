#ifndef _FULLSTARTER_APP_H_
#define _FULLSTARTER_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace fullstarter {
class AllData;

class FullStarterApp : public ds::App {
public:
	FullStarterApp();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	//Idle state of the app to detect state change
	bool				mIdling;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_H_