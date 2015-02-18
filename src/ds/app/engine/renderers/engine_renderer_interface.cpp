#include "engine_renderer_interface.h"

#include <cinder/gl/gl.h>
#include <cinder/Color.h>

void ds::EngineRenderer::clearScreen()
{
	ci::gl::clear(ci::ColorA( 0.85f, 0.85f, 0.85f, 1 ));
}
