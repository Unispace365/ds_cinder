#include "sprite_shader.h"
#include "cinder/DataSource.h"


namespace {

const std::string DefaultFrag = 
"uniform sampler2D tex0;\n"
"uniform bool useTexture;\n"
"uniform bool preMultiply;\n"
"void main()\n"
"{\n"
"    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    color *= gl_Color;\n"
"    if (useTexture) {\n"
"        color = texture2D( tex0, gl_TexCoord[0].st );\n"
"    }\n"
"    if (preMultiply) {\n"
"        color.r *= color.a;\n"
"        color.g *= color.a;\n"
"        color.b *= color.a;\n"
"    }\n"
"    gl_FragColor = color;\n"
"}\n";

const std::string DefaultVert = 
"void main()\n"
"{\n"
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
"  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"  gl_FrontColor = gl_Color;\n"
"}\n";

}

namespace ds {
namespace ui {

SpriteShader::SpriteShader(const std::string &defaultLocation, const std::string &defaultName)
  : mDefaultLocation(defaultLocation)
  , mDefaultName(defaultName)
{

}

SpriteShader::~SpriteShader()
{

}

void SpriteShader::setShaders( const std::string &location, const std::string &name )
{
  if (mName.empty())
    return;

  if (mLocation == location && mName == name)
    return;

  if (mShader)
    mShader.reset();

  mLocation = location;
  mName = name;
}

void SpriteShader::loadShaders()
{
  loadShadersFromFile();
  if (!mShader)
    loadDefaultFromFile();
  if (!mShader)
    loadDefaultFromMemory();
}

bool SpriteShader::isValid() const
{
  return mShader;
}

ci::gl::GlslProg & SpriteShader::getShader()
{
  return mShader;
}

void SpriteShader::loadShadersFromFile()
{
  if (mName.empty())
    return;

  try {
    mShader = ci::gl::GlslProg(ci::loadFile((mLocation+"/"+mName+".vert").c_str()), ci::loadFile((mLocation+"/"+mName+".frag").c_str()));
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

void SpriteShader::loadDefaultFromFile()
{
  if (mDefaultName.empty())
    return;

  try {
    mShader = ci::gl::GlslProg(ci::loadFile((mDefaultLocation+"/"+mDefaultName+".vert").c_str()), ci::loadFile((mDefaultLocation+"/"+mDefaultName+".frag").c_str()));
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

void SpriteShader::loadDefaultFromMemory()
{
  try {
    mShader = ci::gl::GlslProg(DefaultVert.c_str(), DefaultFrag.c_str());
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

std::string SpriteShader::getLocation() const
{
  return mLocation;
}

std::string SpriteShader::getName() const
{
  return mName;
}

}
}
