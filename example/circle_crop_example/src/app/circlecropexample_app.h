#ifndef _CIRCLECROPEXAMPLE_APP_H_
#define _CIRCLECROPEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include <ds/ui/sprite/image.h>

namespace example {
class AllData;

class CircleCropExample : public ds::App {
  public:
	CircleCropExample();

	virtual void onKeyDown(ci::app::KeyEvent event) override;
	void		 setupServer();
	void		 update();

  private:
	typedef ds::App inherited;

	// Data
	AllData mAllData;

	// Data acquisition
	Globals		 mGlobals;
	QueryHandler mQueryHandler;

	// Idle state of the app to detect state change
	bool		   mIdling;
	ds::ui::Image* mShaderCircleCrop;


	void moveCamera(const ci::vec3& deltaMove);
};

} // namespace example

#endif // !_CIRCLECROPEXAMPLE_APP_H_