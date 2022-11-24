#pragma once
#ifndef DS_UI_TOUCH_SELECTPICKING_H_
#define DS_UI_TOUCH_SELECTPICKING_H_

#include "picking.h"
#include <cinder/gl/gl.h>

namespace ds {

/**
 * \class SelectPicking
 * \brief Perform picking via OpenGL select mode.
 */
class SelectPicking : public Picking {
  public:
	SelectPicking();

	virtual ds::ui::Sprite* pickAt(const ci::vec2&, ds::ui::Sprite& root);

  private:
	static const size_t SELECT_BUFFER_SIZE = 99999;
	GLuint				mSelectBuffer[SELECT_BUFFER_SIZE];

	class Hit {
	  public:
		Hit();
		Hit(const sprite_id_t, const int z);

		bool operator<(const Hit&) const;

		sprite_id_t mId;
		int			mZ;
	};
	std::vector<Hit> mHits;
};

} // namespace ds

#endif
