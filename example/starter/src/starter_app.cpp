#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>


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
CINDER_APP(BasicTweenApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))


