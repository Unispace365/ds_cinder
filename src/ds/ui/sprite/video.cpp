#include "Video.h"
#include "cinder\Camera.h"
#include "sprite_engine.h"

using namespace ci;

namespace ds {
namespace ui {

Video::Video( SpriteEngine& engine, const std::string &filename )
    : inherited(engine)
    , mLooping(false)
    , mMuted(true)
    , mVolume(1.0f)
{
  setUseShaderTextuer(true);

    setTransparent(false);
    try
    {
        mMovie = ci::qtime::MovieGl( filename );
        mMovie.setLoop(mLooping);
        mMovie.play();
        mMovie.setVolume(0.0f);
    }
    catch (...)
    {
        return;
    }

    Sprite::setSize(static_cast<float>(mMovie.getWidth()), static_cast<float>(mMovie.getHeight()));

    mFbo = ci::gl::Fbo(getWidth(), getHeight(), true);
}

Video::~Video()
{

}

void Video::drawLocalClient()
{
  if (!inBounds()) {
    if (mMovie && !mMuted) {
      mMovie.setVolume(0.0f);
      mMuted = true;
    }
    return;
  }

    if ( mMovie ) {
      if (mMuted) {
        mMovie.setVolume(mVolume);
        mMuted = false;
      }
      mFrameTexture = mMovie.getTexture();
    }
    if ( mFrameTexture ) {
      {
        gl::SaveFramebufferBinding bindingSaver;

        gl::pushMatrices();
        mSpriteShader.getShader().unbind();
        ci::gl::setViewport(mFrameTexture.getBounds());
        ci::CameraOrtho camera;
        camera.setOrtho(mFrameTexture.getBounds().getX1(), mFrameTexture.getBounds().getX2(), mFrameTexture.getBounds().getY2(), mFrameTexture.getBounds().getY1(), -1.0f, 1.0f);
        gl::setMatrices(camera);
        // bind the framebuffer - now everything we draw will go there
        mFbo.bindFramebuffer();
        gl::clear(ci::Color(1.0f, 1.0f, 1.0f));
        ci::gl::draw(mFrameTexture);
        mFbo.unbindFramebuffer();
        mSpriteShader.getShader().bind();
        gl::popMatrices();
      }

      Rectf screenRect = mEngine.getScreenRect();
      gl::setViewport(Area((int)screenRect.getX1(), (int)screenRect.getY2(), (int)screenRect.getX2(), (int)screenRect.getY1()));
      Rectf area(0.0f, getHeight(), getWidth(), 0.0f);
      gl::draw( mFbo.getTexture(0), area );
    }
}

void Video::setSize( float width, float height )
{
    setScale( width / getWidth(), height / getHeight() );
}

void Video::loadVideo( const std::string &filename )
{
    try
    {
        mMovie = ci::qtime::MovieGl( filename );
        mMovie.setLoop(mLooping);
        mMovie.play();
        mMovie.setVolume(0.0f);
        mMuted = true;
    }
    catch (...)
    {
        return;
    }

    float prevWidth = getWidth() * getScale().x;
    float prevHeight = getHeight() * getScale().y;

    Sprite::setSize(static_cast<float>(mMovie.getWidth()), static_cast<float>(mMovie.getHeight()));
    setSize(prevWidth, prevHeight);
}

void Video::play()
{
    if ( mMovie )
        mMovie.play();
}

void Video::stop()
{
    if ( mMovie )
    {
        mMovie.stop();
        mMovie.reset();
    }
}

void Video::pause()
{
    if ( mMovie )
        mMovie.stop();
}

void Video::seek( float t )
{
    if ( mMovie )
        mMovie.seekToTime(t);
}

float Video::duration() const
{
    if ( mMovie )
        return mMovie.getDuration();
    return 0.0f;
}

bool Video::isPlaying() const
{
    if ( mMovie )
        return mMovie.isPlaying();
    return false;
}

void Video::loop( bool flag )
{
    mLooping = flag;
    if ( mMovie )
        mMovie.setLoop(mLooping);
}

bool Video::isLooping() const
{
    return mLooping;
}

void Video::setVolume( float volume )
{
  mVolume = volume;
}

float Video::getVolume() const
{
  return mVolume;
}

} // namespace ui
} // namespace ds
