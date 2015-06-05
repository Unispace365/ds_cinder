#ifndef _SCROLLEXAMPLE_APP_H_
#define _SCROLLEXAMPLE_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace example {
class AllData;
class InfoList;

class ScrollExample : public ds::App {
public:
	ScrollExample();

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

	InfoList*			mInfoList;

	// For an instantiated scroll list
	std::map<int, ds::model::StoryRef>							mInfoMap;

	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace example

#endif // !_SCROLLEXAMPLE_APP_H_