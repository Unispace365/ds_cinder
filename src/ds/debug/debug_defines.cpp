#include "ds/debug/debug_defines.h"

#include <sstream>
#include <cinder/gl/gl.h>
#include "logger.h"

#ifdef TURN_ON_REPORT_GL_ERRORS
void ds::report_gl_errors()
{
  GLenum			err;
  while ((err=glGetError()) != GL_NO_ERROR) {
    std::stringstream s;
    s << "OpenGL: Error 0x" << std::hex << err << " at file " << __FILE__ << ", line " << std::dec << __LINE__ << " thread context 0x" << std::hex << wglGetCurrentContext() << std::dec;
    std::cout << s.str() << std::endl;
    DS_LOG_WARNING(s.str());
    //assert(false);
  }
}
#endif // TURN_ON_REPORT_GL_ERRORS