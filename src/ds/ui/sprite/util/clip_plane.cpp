#include "clip_plane.h"
#include <cinder/gl/gl.h>

namespace ds {
namespace ui {

void enableClipping( float x0, float y0, float x1, float y1 )
{
  glEnable(GL_SCISSOR_TEST);
  glScissor(x0, y0, x1, y1);
}

void disableClipping()
{
  glDisable(GL_SCISSOR_TEST);
}

}
}
