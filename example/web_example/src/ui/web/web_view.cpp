#include "web_view.h"

#include <ds/app/engine/engine_cfg.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "app/app_defs.h"
#include "app/globals.h"


// Add layout
#pragma warning(disable : 4355)

namespace web_example {

/**
 * web_example::WebView
 */

WebView::WebView(Globals& g)
  : inherited(g.mEngine)
  , mGlobals(g)
  , mWeb(ds::ui::Sprite::makeAlloc<ds::ui::Web>(
		[&g]() -> ds::ui::Web* { return new ds::ui::Web(g.mEngine, 0.0f, 0.0f); }, this)) {


	const std::string& url = mGlobals.getSettingsLayout().getString("web:url", 0, "http://google.com");

	mWeb.setDragScrolling(true);
	mWeb.setDragScrollingMinimumFingers(1);
	mWeb.setDrawWhileLoading(true);
	mWeb.setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mWeb.loadUrl(url);
}


} // namespace web_example
