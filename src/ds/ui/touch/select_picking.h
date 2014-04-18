#pragma once
#ifndef DS_UI_TOUCH_SELECTPICKING_H_
#define DS_UI_TOUCH_SELECTPICKING_H_

#include <cinder/gl/gl.h>
#include "picking.h"

namespace ds {

/**
 * \class ds::SelectPicking
 * \brief Perform picking via OpenGL select mode.
 */
class SelectPicking : public Picking {
public:
	SelectPicking();

	virtual ds::ui::Sprite*	pickAt(const ci::Vec2f&, ds::ui::Sprite& root);

private:
	static const size_t		SELECT_BUFFER_SIZE = 99999;
	GLuint					mSelectBuffer[SELECT_BUFFER_SIZE];
};

} // namespace ds

#endif
