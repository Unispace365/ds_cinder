#pragma once
#ifndef TAP_ACTION_DS_H
#define TAP_ACTION_DS_H

#include "base_action.h"
#include <cinder/Vector.h>

namespace ds {
namespace ui {
	class SpriteEngine;
}
namespace debug {

	/**
	 * \class TapActionFactory
	 */
	class TapActionFactory : public BaseActionFactory {
	  public:
		TapActionFactory() {}

		virtual float getLimit() const;
		virtual int	  getNumberOfFingers() const;

		virtual BaseAction* build(std::vector<int>& freeList, ds::ui::SpriteEngine& engine,
								  const ci::Rectf& frame) const;
	};

	/**
	 * \class TapAction
	 */
	class TapAction : public BaseAction {
	  public:
		TapAction(std::vector<int>& freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame);
		virtual ~TapAction();

		void setup(float limit, int numberOfFingers);

		virtual bool update(float dt);

	  protected:
		std::vector<ci::vec2> mTouchPos;
	};

} // namespace debug
} // namespace ds

#endif // TAP_ACTION_DS_H
