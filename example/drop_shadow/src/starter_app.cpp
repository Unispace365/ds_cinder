#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/nine_patch.h>

class ExampleApp : public ds::App {
public:
	ExampleApp();

	virtual void		setupServer();

private:
	typedef ds::App		inherited;
};

ExampleApp::ExampleApp() {
	mEngine.loadNinePatchCfg("nine_patch.xml");
}

void ExampleApp::setupServer() {
	// Get the master containing sprite
	ds::ui::Sprite&					root = mEngine.getRootSprite();
	root.setTransparent(false);
	root.setColor(0.38f, 0.05f, 0.30f);

	// Get the nine patch config objects. The names are from the
	// settings/nine_patch.xml file.
	const ds::cfg::NinePatch&		bg_cfg(mEngine.getEngineCfg().getNinePatch("panel:bg"));
	const ds::cfg::NinePatch&		fg_cfg(mEngine.getEngineCfg().getNinePatch("panel:fg"));

	// Make the sprites
	ds::ui::NinePatch&				bg(bg_cfg.createOrThrow(mEngine, &root));
	ds::ui::NinePatch&				fg(fg_cfg.createOrThrow(mEngine, &root));

	// Configure them.
	const ci::Rectf					frame(	floorf(mEngine.getWorldWidth()*0.25f), floorf(mEngine.getWorldHeight()*0.25f),
											floorf(mEngine.getWorldWidth()*0.75f), floorf(mEngine.getWorldHeight()*0.75f) );
	bg.setPosition(frame.x1-bg_cfg.getRadius(), frame.y1-bg_cfg.getRadius());
	bg.setSize(frame.getWidth()+(bg_cfg.getRadius()*2.0f), frame.getHeight()+(bg_cfg.getRadius()*2.0f));

	fg.setPosition(frame.x1, frame.y1);
	fg.setSize(frame.getWidth(), frame.getHeight());
}

// This line tells Cinder to actually create the application
CINDER_APP( ExampleApp,  ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)) )
