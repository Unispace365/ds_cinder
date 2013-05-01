#include "cinder/app/App.h"
#include "cinder/app/AppBasic.h"
#include "ds/app/app.h"
#include "ds/ui/sprite/image.h"

namespace ds {
	class App;
	int runApp( App *app );
	int go();

	namespace ui {
	class ShaderSprite : public Sprite {
	public:
		ShaderSprite( SpriteEngine&, float w=0.0f, float h=0.0f, const std::string &filename = "" );
		void drawLocalClient();
		virtual void onBindShader( ci::gl::GlslProg &shader );
		cinder::gl::GlslProg * getShader();
	};
	} // namespace ui
} //namespace ds
