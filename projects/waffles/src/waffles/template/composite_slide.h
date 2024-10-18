#pragma once

#include <ds/ui/layout/smart_layout.h>

#include "template_base.h"

namespace waffles {


/**
 * \class waffles::CompositeSlide
 */
class CompositeSlide : public TemplateBase {
  public:
	CompositeSlide(ds::ui::SpriteEngine& engine, TemplateDef& def,
				   ds::model::ContentModelRef content = ds::model::ContentModelRef(),
				   std::string channel_name = "");

	virtual float animateOn(float delay, std::function<void(void)> finishedCb = nullptr) override;
	virtual float animateOff(float delay, std::function<void(void)> finishedCb) override;

	ds::EventClient mEventClient;
};

} // namespace waffles
