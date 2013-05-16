#include "Video.h"
#include "cinder\Camera.h"
#include "sprite_engine.h"
#include "ds/debug/debug_defines.h"
#include "ds/data/resource_list.h"
#include "ds/util/file_name_parser.h"
#include "ds/math/math_func.h"

using namespace ci;

namespace ds {
namespace ui {
  
Video::Video( SpriteEngine& engine )
    : inherited(engine)
    , mLooping(false)
    , mInternalMuted(true)
    , mVolume(1.0f)
	, mMute(false)
    , mStatusDirty(false)
    , mStatusFn(nullptr)
{
  setUseShaderTextuer(true);
  setTransparent(false);
  setStatus(Status::STATUS_STOPPED);
}

Video::~Video()
{
}

void Video::updateServer(const UpdateParams& up)
{
  inherited::updateServer(up);

  if (mStatusDirty) {
    mStatusDirty = false;
    if (mStatusFn) mStatusFn(mStatus);
  }
}

void Video::drawLocalClient()
{
  if (!mFbo) return;

  if (mMovie) {
    // The movie considers itself still playing even after the
    // video is over -- that state is covered by "done"
    if (mMovie.isDone()) setStatus(Status::STATUS_STOPPED);
    else if (mMovie.isPlaying()) setStatus(Status::STATUS_PLAYING);
    else setStatus(Status::STATUS_PAUSED);
  }

  if (!inBounds()) {
    if (mMovie && !mInternalMuted) {
      mMovie.setVolume(0.0f);
      mInternalMuted = true;
    }
    return;
  }

    if ( mMovie ) {
      if (mInternalMuted) {
		setMovieVolume();
        mInternalMuted = false;
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
        camera.setOrtho(float(mFrameTexture.getBounds().getX1()), float(mFrameTexture.getBounds().getX2()), float(mFrameTexture.getBounds().getY2()), float(mFrameTexture.getBounds().getY1()), -1.0f, 1.0f);
        gl::setMatrices(camera);
        // bind the framebuffer - now everything we draw will go there
        mFbo.bindFramebuffer();

        glPushAttrib( GL_TRANSFORM_BIT | GL_ENABLE_BIT );
        for (int i = 0; i < 4; ++i) {
          glDisable( GL_CLIP_PLANE0 + i );
        }

       // gl::clear(ci::Color(0.0f, 0.0f, 0.0f));
		gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
        ci::gl::draw(mFrameTexture);

        glPopAttrib();

        mFbo.unbindFramebuffer();
        mSpriteShader.getShader().bind();
        gl::popMatrices();
      }

      Rectf screenRect = mEngine.getScreenRect();
      gl::setViewport(Area((int)screenRect.getX1(), (int)screenRect.getY2(), (int)screenRect.getX2(), (int)screenRect.getY1()));

      if (getPerspective()) {
        Rectf area(0.0f, 0.0f, getWidth(), getHeight());
        gl::draw( mFbo.getTexture(0), area );
      } else {
        Rectf area(0.0f, getHeight(), getWidth(), 0.0f);
        gl::draw( mFbo.getTexture(0), area );
      }
    }
}

void Video::setSize( float width, float height )
{
    setScale( width / getWidth(), height / getHeight() );
}

Video& Video::loadVideo( const std::string &filename )
{
  try
  {
//std::cout << "loadVideo() 1" << std::endl;
    mMovie = ci::qtime::MovieGl( filename );
//std::cout << "loadVideo() 2" << std::endl;
    mMovie.setLoop(mLooping);
//std::cout << "loadVideo() 3" << std::endl;
    mMovie.play();
//std::cout << "loadVideo() 4" << std::endl;
	setMovieVolume();
//std::cout << "loadVideo() 5" << std::endl;
    mInternalMuted = true;
//std::cout << "loadVideo() 6" << std::endl;
    setStatus(Status::STATUS_PLAYING);
  }
  catch (std::exception const& ex)
  {
    DS_DBG_CODE(std::cout << "ERROR Video::loadVideo() ex=" << ex.what() << std::endl);
    return *this;
  }

  Sprite::setSizeAll(static_cast<float>(mMovie.getWidth()), static_cast<float>(mMovie.getHeight()), mDepth);

  if (getWidth() > 0 &&  getHeight() > 0) {
    setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
  }
  mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
  return *this;
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

void Video::setVolume( float volume, const bool turnOffMute )
{
	mVolume = volume;
	if (turnOffMute) mMute = false;
	setMovieVolume();
}

float Video::getVolume() const
{
	return mVolume;
}

void Video::setMute(const bool on)
{
	if (on == mMute) return;

	mMute = on;
	setMovieVolume();
}

bool Video::getMute() const
{
	return mMute;
}

void Video::setStatusCallback(const std::function<void(const Status&)>& fn)
{
  DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
  mStatusFn = fn;
}

void Video::setStatus(const int code)
{
  if (code == mStatus.mCode) return;

  mStatus.mCode = code;
  mStatusDirty = true;
}

void Video::setMovieVolume()
{
	if (mMovie) {
		if (mMute) mMovie.setVolume(0.0f);
		else mMovie.setVolume(mVolume);
	}
}

float Video::currentTime() const
{
	if (mMovie)
		return mMovie.getCurrentTime();
	else
		return 0.0f;
}

Video &Video::setResourceId( const ds::Resource::Id &resourceId )
{
  try
  {
    ds::Resource            res;
    if (mEngine.getResources().get(resourceId, res)) {
      Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
      std::string filename = res.getAbsoluteFilePath();

      mMovie = ci::qtime::MovieGl( filename );
      mMovie.setLoop(mLooping);
      mMovie.play();
	  setMovieVolume();
      mInternalMuted = true;
      setStatus(Status::STATUS_PLAYING);
    }
  }
  catch (std::exception const& ex)
  {
    DS_DBG_CODE(std::cout << "ERROR Video::loadVideo() ex=" << ex.what() << std::endl);
    return *this;
  }

  mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
  return *this;
}

} // namespace ui
} // namespace ds
