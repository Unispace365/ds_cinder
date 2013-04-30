#include "cinder/app/App.h"
#include "cinder/app/AppBasic.h"
#include "ds/app/app.h"

namespace ds {
    class App;
    int runApp( App *app );
	int go();

    namespace ui {
    class ShaderSprite : public Sprite {
        void drawLocalClient();
        virtual void onBindShaders( ci::gl::GlslProg &shader );
    };
	} // namespace ui
} //namespace ds
