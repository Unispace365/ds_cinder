#pragma once

#include <ds/ui/layout/smart_layout.h>

#include "template_config.h"

namespace waffles {


/**
 * \class waffles::TemplateBase
 */
class TemplateBase : public ds::ui::SmartLayout {
  public:
	TemplateBase(ds::ui::SpriteEngine& engine, ds::model::ContentModelRef content = ds::model::ContentModelRef())
		: ds::ui::SmartLayout(engine, TemplateConfig::getDefault()->getTemplateDefFromName(content.getPropertyString("template_type")).layoutXml) {
		setContentModel(content);
	}

	TemplateBase(ds::ui::SpriteEngine& engine, TemplateDef& def,
				 ds::model::ContentModelRef content = ds::model::ContentModelRef())
		: ds::ui::SmartLayout(engine, def.layoutXml) {
		setContentModel(content);
	}

	void setAnimStartCb(std::function<void(void)> cb) { mAnimStartCb = cb; }

	virtual float animateOn(float delay, std::function<void(void)> finishedCb = nullptr) {
		if (mAnimStartCb) mAnimStartCb();
		return tweenAnimateOn(true, delay, 0.05f, finishedCb);
	}

	virtual float animateOff(float delay, std::function<void(void)> finishedCb = nullptr) {
		return tweenAnimateOff(true, delay, 0.00f, finishedCb);
	}

  protected:
	std::function<void(void)> mAnimStartCb;
	
};

} // namespace waffles
