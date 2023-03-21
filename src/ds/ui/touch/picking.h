#pragma once
#ifndef DS_UI_TOUCH_PICKING_H_
#define DS_UI_TOUCH_PICKING_H_

#include "ds/ui/sprite/sprite.h"
#include <functional>

namespace ds {

/**
 * \class Picking
 * \brief Abstract superclass for picking.
 */
class Picking {
  public:
	virtual ~Picking();

	void setWorldSize(const ci::vec2&);

	virtual ds::ui::Sprite* pickAt(const ci::Ray&, const ci::vec3&, ds::ui::Sprite& root) = 0;

  protected:
	ci::vec2 mWorldSize;

	Picking();
};

} // namespace ds

#endif
