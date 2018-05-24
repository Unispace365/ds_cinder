#pragma once
#ifndef _DS_UI_TOUCH_TUIO_INPUT_H_
#define _DS_UI_TOUCH_TUIO_INPUT_H_

#include <ds/ui/sprite/sprite_engine.h>
#include <tuio/TuioClient.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::TuioInput
* \brief Handles multiplexed TUIO input, with unique transformations for each input
*		 Note that this is in additional to the built-in touch mode
*/
class TuioInput {
public:
	TuioInput(ds::ui::SpriteEngine& engine, const int port, const ci::vec2 scale, const ci::vec2 offset,
			  const float rotty, const int fingerIdOffset, const ci::Rectf& allowedArea);
	~TuioInput();

protected:
	ci::vec2 transformEventPosition(const ci::vec2& pos, const bool doWindowScale = false);

private:
	ds::ui::TouchEvent convertTouchEvent(ci::app::TouchEvent& e, const bool isAdd);
	ds::ui::SpriteEngine& mEngine;
	ci::tuio::Client      mTuioClient;

	int       mFingerIdOffset;
	glm::mat4 mTransform;
	ci::Rectf mAllowedRect;
};

} // ! namespace ui
}  // !namespace ds

#endif  // !_DS_UI_TOUCH_TUIO_INPUT_H_
