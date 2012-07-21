#include "fbo.h"

namespace ds {
namespace ui {

FboGeneral::FboGeneral()
  : mFboId(0)
  , mDepthId(0)
  , mStencilId(0)
  , mPreviousFbo(0)
  , mAttached(nullptr)
  , mWidth(0)
  , mHeight(0)
{

}

FboGeneral::~FboGeneral()
{
  reset();
}

void FboGeneral::setup( bool useDepth /*= false*/, bool useStencil /*= false*/ )
{
  reset();

  glGenFramebuffersEXT( 1, &mFboId );
  activate();
  if (useDepth) {
    glGenRenderbuffersEXT( 1, &mDepthId );
    glBindRenderbufferEXT( GL_RENDERBUFFER, mDepthId );
  }
  if (useStencil) {
    glGenRenderbuffersEXT( 1, &mStencilId );
    glBindRenderbufferEXT( GL_RENDERBUFFER, mStencilId );
  }
  deactivate();

  mAttached = nullptr;
}

void FboGeneral::attach( ci::gl::Texture &target, bool useDepth /*= false*/, bool useStencil /*= false*/ )
{
  activate();

  if (mAttached != nullptr) {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, mAttached->getTarget(), 0, 0);
    mAttached = nullptr;
  }

  mAttached = &target;

  GLint error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cout << "ouch" << std::endl;
  }

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, mAttached->getTarget(), mAttached->getId(), 0);

  error = glGetError();
  if (error != GL_NO_ERROR) {
    if (error == GL_INVALID_ENUM) {
      std::cout << "GL_INVALID_ENUM" << std::endl;
    } else if (error == GL_INVALID_VALUE) {
      std::cout << "GL_INVALID_VALUE" << std::endl;
    } else if (error == GL_INVALID_OPERATION) {
      std::cout << "GL_INVALID_OPERATION" << std::endl;
    } else if (error == GL_STACK_OVERFLOW) {
      std::cout << "GL_STACK_OVERFLOW" << std::endl;
    } else if (error == GL_STACK_UNDERFLOW) {
      std::cout << "GL_STACK_UNDERFLOW" << std::endl;
    } else if (error == GL_OUT_OF_MEMORY) {
      std::cout << "GL_OUT_OF_MEMORY" << std::endl;
    }
  }

  mWidth = mAttached->getWidth();
  mHeight = mAttached->getHeight();

  if ( useDepth ) {
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthId);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, mWidth, mHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthId);
  } else {
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
  }

  if ( useStencil ) {
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mStencilId);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, mWidth, mHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mStencilId);
  } else {
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
  }

  deactivate();
}

void FboGeneral::detach()
{
  activate();

  if (mAttached) {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, mAttached->getInternalFormat(), 0, 0);
    mAttached = nullptr;
    mWidth = 0;
    mHeight = 0;
  }

  deactivate();
}

void FboGeneral::begin()
{
  pushTransformation();
  activate();
}

void FboGeneral::end()
{
  deactivate();
  popTransformation();
}

ci::gl::Texture *FboGeneral::getAttached()
{
  return mAttached;
}

void FboGeneral::offsetViewport(int offsetX, int offsetY)
{
  glViewport(offsetX, offsetY, offsetX + mWidth, offsetY + mHeight);
}

void FboGeneral::pushTransformation()
{
  mOldViewport[4];
  glGetIntegerv(GL_VIEWPORT, mOldViewport);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
}

void FboGeneral::popTransformation()
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glViewport(mOldViewport[0], mOldViewport[1], mOldViewport[2], mOldViewport[3]);
}

int FboGeneral::getWidth() const
{
  return mWidth;
}

int FboGeneral::getHeight() const
{
  return mHeight;
}

void FboGeneral::activate()
{
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &mPreviousFbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
}

void FboGeneral::deactivate()
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mPreviousFbo);
}

void FboGeneral::reset()
{
  if (mFboId) {
    glDeleteFramebuffersEXT(1, &mFboId);
    mFboId = 0;
  }
  if (mDepthId) {
    glDeleteRenderbuffersEXT(1, &mDepthId);
    mDepthId = 0;
  }
  if (mStencilId) {
    glDeleteRenderbuffersEXT(1, &mStencilId);
    mStencilId = 0;
  }

  mPreviousFbo = 0;

  mWidth = 0;
  mHeight = 0;
}


FboGeneral::AutoAttach::AutoAttach(FboGeneral &fbo, ci::gl::Texture &target, bool useDepth /*= false*/, bool useStencil /*= false */)
  : mFbo(fbo)
{
  mFbo.attach(target, useDepth, useStencil);
}

FboGeneral::AutoAttach::~AutoAttach()
{
  mFbo.detach();
}


FboGeneral::AutoRun::AutoRun(FboGeneral &fbo)
  : mFbo(fbo)
{
  mFbo.begin();
}

FboGeneral::AutoRun::~AutoRun()
{
  mFbo.end();
}

}
}