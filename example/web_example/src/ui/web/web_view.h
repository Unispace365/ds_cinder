#pragma once
#ifndef UI_WEB_WEB_VIEW
#define UI_WEB_WEB_VIEW

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/web.h>

namespace ds { namespace ui {
	class Web;
}} // namespace ds::ui

namespace web_example {

class Globals;

/**
 * \class web_example::WebView
 * \brief A single website or something
 */
class WebView : public ds::ui::Sprite {

  public:
	WebView(Globals& g);

	void goBack() { mWeb.goBack(); }
	void goForward() { mWeb.goForward(); }
	void reload() { mWeb.reload(); }

	void setUrl(const std::string& newUrl) { mWeb.setUrl(newUrl); }

  private:
	typedef ds::ui::Sprite inherited;

	Globals&	 mGlobals;
	ds::ui::Web& mWeb;
};

} // namespace web_example

#endif
