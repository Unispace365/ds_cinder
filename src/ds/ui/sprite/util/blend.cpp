#include "blend.h"
#include <cinder/gl/gl.h>

namespace ds {
namespace ui {

void applyBlendingMode(const BlendMode &blendMode)
{
  if (blendMode == NORMAL) {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else if (blendMode == MULTIPLY) {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
  } else if (blendMode == SCREEN) {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
  } else if (blendMode == ADD) {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  } else if (blendMode == SUBTRACT) {
    glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    glBlendFunc(GL_ONE, GL_ONE);
  } else if (blendMode == LIGHTEN) {
    glBlendEquation(GL_MAX);
    glBlendFunc(GL_ONE, GL_ONE);
  } else if (blendMode == DARKEN) {
    glBlendEquation(GL_MIN);
    glBlendFunc(GL_ONE, GL_ONE);
  }
}

bool premultiplyAlpha( const BlendMode &blendMode )
{
  if (blendMode == NORMAL) {
    return false;
  } else if (blendMode == MULTIPLY) {
    return true;
  } else if (blendMode == SCREEN) {
    return true;
  } else if (blendMode == ADD) {
    return true;
  } else if (blendMode == SUBTRACT) {
    return true;
  } else if (blendMode == LIGHTEN) {
    return true;
  } else if (blendMode == DARKEN) {
    return true;
  }
  return false;
}

}
}
