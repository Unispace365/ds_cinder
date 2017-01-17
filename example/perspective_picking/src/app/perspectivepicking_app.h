#ifndef _PERSPECTIVEPICKING_APP_H_
#define _PERSPECTIVEPICKING_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace perspective_picking {
class AllData;

class PerspectivePicking : public ds::App {
public:
	PerspectivePicking();

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

	int					mDebugCamera;

	void				moveRoot(const ci::vec3& deltaMove);
	void				moveCamera(const ci::vec3& deltaMove, const bool moveTarget);
	void				shiftLensH(const float amount);
};

} // !namespace perspective_picking

#endif // !_PERSPECTIVEPICKING_APP_H_

