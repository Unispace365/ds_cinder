#include "ds/ui/sprite/fbo/auto_fbo.h"

#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

/**
* \class ds::ui::AutoFbo
*/
AutoFbo::AutoFbo(ds::ui::SpriteEngine& engine)
	: mEngine(engine)
	, mHasTexture(false)
{
	mFbo = std::move(mEngine.getFbo());
}

AutoFbo::AutoFbo(ds::ui::SpriteEngine& engine, ci::gl::TextureRef tex)
	: mEngine(engine)
	, mHasTexture(true)
{
	mFbo = std::move(mEngine.getFbo());
	mFbo->attach(tex);
	// bind the framebuffer - now everything we draw will go there
	mFbo->begin();
}

AutoFbo::~AutoFbo()
{
	if(mHasTexture) {
		mFbo->end();
		mFbo->detach();
	}
	mEngine.giveBackFbo(std::move(mFbo));
}

} // namespace ui
} // namespace ds
