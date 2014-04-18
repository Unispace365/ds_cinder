#pragma once
#ifndef DS_UI_TOUCH_SELECTPICKING_H_
#define DS_UI_TOUCH_SELECTPICKING_H_

#include <functional>
#include <cinder/gl/gl.h>
#include "ds/ui/sprite/sprite.h"

namespace ds {

/**
 * \class ds::SelectPicking
 * \brief Perform picking via OpenGL select mode.
 */
class SelectPicking {
public:
	SelectPicking();

	ds::ui::Sprite*			pickAt(const ci::Vec2f&, const std::function<void(void)>& gl_draw_fn);

private:
	static const size_t		SELECT_BUFFER_SIZE = 99999;
	GLuint					mSelectBuffer[SELECT_BUFFER_SIZE];
};

} // namespace ds

#endif
