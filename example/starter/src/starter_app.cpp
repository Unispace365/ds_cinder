#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class BasicTweenApp : public ds::App {
  public:
    BasicTweenApp();

    void				setupServer();

  private:
    typedef ds::App   inherited;
};

BasicTweenApp::BasicTweenApp()
{
}

void BasicTweenApp::setupServer()
{
  ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
  // add sprites...
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
