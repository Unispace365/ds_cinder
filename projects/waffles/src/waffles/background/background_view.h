#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

namespace waffles {
	class TemplateConfig;
/**
 * \class waffles::BackgroundView
 *			Show something in the background
 */
class BackgroundView final : public ds::ui::Sprite {
  public:
	BackgroundView(ds::ui::SpriteEngine& g);


  private:
	void startBackground(const int type, ds::model::ContentModelRef media, const int pdfPage);

	ds::EventClient mEventClient;
	ds::ui::Sprite* mCurrentBackground = nullptr;
	TemplateConfig* mTemplateConfig = nullptr;
};

} // namespace waffles
