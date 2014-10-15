#pragma once
#ifndef EXCHANGELOOP_APP_EXCHANGELOOPAPP_H
#define EXCHANGELOOP_APP_EXCHANGELOOPAPP_H

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

	// Data acquision
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	//Idle state of the app to detect state change
	bool				mIdling;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // namespace fullstarter

#endif