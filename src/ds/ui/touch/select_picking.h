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

	class Hit {
	public:
		Hit();
		Hit(const sprite_id_t, const int z);

		bool				operator<(const Hit&) const;

		sprite_id_t			mId;
		int					mZ;
	};
	std::vector<Hit>		mHits;
};

} // namespace ds

#endif
