#pragma once
#ifndef DS_UI_SPRITE_UTIL_AUTOFBO_H_
#define DS_UI_SPRITE_UTIL_AUTOFBO_H_

#include <memory>
#include <cinder/gl/Texture.h>
#include "ds/ui/sprite/fbo/fbo.h"

namespace ds {
namespace ui {
class SpriteEngine;

/**
 * \class ds::ui::AutoFbo
 * \brief Utility for auto managing an Fbo.
 */
class AutoFbo {
public:
	AutoFbo(ds::ui::SpriteEngine&);
	AutoFbo(ds::ui::SpriteEngine&, ci::gl::Texture&);
	~AutoFbo();

	std::unique_ptr<ds::ui::FboGeneral>		mFbo;

private:
	ds::ui::SpriteEngine&					mEngine;
	const bool								mHasTexture;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_UTIL_AUTOFBO_H_
