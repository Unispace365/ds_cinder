#pragma once
#ifndef DS_UI_TOUCH_PICKING_H_
#define DS_UI_TOUCH_PICKING_H_

#include <functional>
#include "ds/ui/sprite/sprite.h"

namespace ds {

/**
 * \class ds::Picking
 * \brief Abstract superclass for picking.
 */
class Picking {
public:
	virtual ~Picking();

	virtual ds::ui::Sprite*	pickAt(const ci::Vec2f&, ds::ui::Sprite& root) = 0;
	
protected:
	Picking();
};

} // namespace ds

#endif
