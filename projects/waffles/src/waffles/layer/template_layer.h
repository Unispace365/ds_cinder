#pragma once

#include <ds/ui/layout/smart_layout.h>
#include "waffles/waffles_events.h"

namespace waffles {

struct ChangeTemplateRequest;
class TemplateBase;

/**
 * \class waffles::TemplateLayer
 *			The background layer. Holds either media or the ambient particle background
 */
class TemplateLayer : public ds::ui::SmartLayout {
  public:
	TemplateLayer(ds::ui::SpriteEngine& eng, ci::vec2 size, ci::vec2 pos);

	static TemplateLayer* get();

	TemplateBase* getCurrentTemplate() { return mCurrentTemplate; }

  private:
	virtual void changeTemplate(const waffles::ChangeTemplateRequest& event);


	TemplateBase* mCurrentTemplate = nullptr;
};

} // namespace waffles