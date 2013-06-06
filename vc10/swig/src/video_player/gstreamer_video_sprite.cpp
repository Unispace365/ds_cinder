#include "gstreamer_video_sprite.h"
#include "cinder\Camera.h"
#include "ds/app/engine.h"
#include "ds/debug/debug_defines.h"
#include "ds/data/resource_list.h"
#include "ds/util/file_name_parser.h"
#include "ds/math/math_func.h"
#include "ds/debug/logger.h"

using namespace ci;

using namespace _2RealGStreamerWrapper;

namespace ds {
	namespace ui {

		GstreamerVideoSprite::GstreamerVideoSprite( SpriteEngine& engine )
			: inherited(engine)
			, mLooping(false)
			, mMuted(false)
			, mInternalMuted(true)
			, mVolume(1.0f)
			, mStatusDirty(false)
			, mStatusFn(nullptr)
			, mIsTransparent(true)
		{
			setUseShaderTextuer(true);
			setTransparent(false);
			setStatus(Status::STATUS_STOPPED);
		}

		GstreamerVideoSprite::~GstreamerVideoSprite(){
			mMovie.stop();
			mMovie.close();
		}

		void GstreamerVideoSprite::updateServer(const UpdateParams& up)
		{
			inherited::updateServer(up);

			if (mStatusDirty) {
				mStatusDirty = false;
				if (mStatusFn) mStatusFn(mStatus);
			}
			
			mMovie.update();
		}

		void GstreamerVideoSprite::drawLocalClient()
		{
			if (!mFbo) return;

			if (mMovie.getState() == STOPPED) setStatus(Status::STATUS_STOPPED);
			else if (mMovie.getState() == PLAYING) setStatus(Status::STATUS_PLAYING);
			else setStatus(Status::STATUS_PAUSED);

			if (!inBounds()) {
				if (!mInternalMuted) {
					mMovie.setVolume(0.0f);
					mInternalMuted = true;
				}
				return;
			}

			if (mInternalMuted) {
				mInternalMuted = false;
				setMovieVolume();
			}
			if(mMovie.hasVideo() && mMovie.isNewVideoFrame()){
				unsigned char* pImg = mMovie.getVideo();
				if(pImg != nullptr){		
					int vidWidth( mMovie.getWidth()), vidHeight(mMovie.getHeight());
					if(mIsTransparent){
						mFrameTexture = gl::Texture(pImg, GL_RGBA, vidWidth, vidHeight);
					} else {
						mFrameTexture = gl::Texture(pImg, GL_RGB, vidWidth, vidHeight);
					}
				// 	DS_LOG_INFO("New video frame, texture id: " <<mFrameTexture.getId());
				}
			}

			if ( mFrameTexture ) {
				mFrameTexture.bind();
				if (getPerspective())
					ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mFrameTexture.getHeight()), static_cast<float>(mFrameTexture.getWidth()), 0.0f));
				else
					ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mFrameTexture.getWidth()), static_cast<float>(mFrameTexture.getHeight())));
				mFrameTexture.unbind();

				/*
				{

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

					if(mIsTransparent){
						gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
					} else {
						gl::clear(ci::Color(0.0f, 0.0f, 0.0f));
					}

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

				DS_REPORT_GL_ERRORS();
				*/
			}
		}

		void GstreamerVideoSprite::setSize( float width, float height )
		{
			setScale( width / getWidth(), height / getHeight() );
		}

		GstreamerVideoSprite& GstreamerVideoSprite::loadVideo( const std::string &filename, int videoWidth )
		{
			if(filename.empty()){
				DS_LOG_WARNING("GstreamerVideoSprite::loadVideo recieved a blank filename. Cancelling load.");
				return *this;
			}

			try
			{
				mMovie.open( filename, true, false, mIsTransparent, videoWidth );
				if(mLooping){
					mMovie.setLoopMode(LOOP);
				} else {
					mMovie.setLoopMode(NO_LOOP);
				}
				//mMovie.play();
				setMovieVolume();
				mInternalMuted = true;
				mMovie.setVideoCompleteCallback([this](GStreamerWrapper* video){ handleVideoComplete(video);});

				setStatus(Status::STATUS_PLAYING);
			}
			catch (std::exception const& ex)
			{
				DS_DBG_CODE(std::cout << "ERROR Video::loadVideo() ex=" << ex.what() << std::endl);
				return *this;
			}

			if(mMovie.getWidth() < 1.0f || mMovie.getHeight() < 1.0f){
				DS_LOG_WARNING("Video is too small to be used or didn't load correctly! " << filename << " " << getWidth() << " " << getHeight());
				return *this;
			}

			Sprite::setSizeAll(static_cast<float>(mMovie.getWidth()), static_cast<float>(mMovie.getHeight()), mDepth);

			if (getWidth() > 0 &&  getHeight() > 0) {
				setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
			}
			mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
			return *this;
		}

		GstreamerVideoSprite &GstreamerVideoSprite::setResourceId( const ds::Resource::Id &resourceId )
		{
			try
			{
				ds::Resource            res;
				if (mEngine.getResources().get(resourceId, res)) {
					Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
					std::string filename = res.getAbsoluteFilePath();
					loadVideo(filename);
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

		void GstreamerVideoSprite::play()
		{
			mMovie.play();
		}

		void GstreamerVideoSprite::stop()
		{
			mMovie.stop();
		}

		void GstreamerVideoSprite::pause()
		{
			mMovie.pause();
		}

		void GstreamerVideoSprite::seek( float t )
		{
			mMovie.setTimePositionInMs(t);
			//	mMovie.seekToTime(t);
		}

		double GstreamerVideoSprite::duration() 
		{
			return mMovie.getDurationInMs();
		}

		bool GstreamerVideoSprite::isPlaying()
		{
			if(mMovie.getState() == PLAYING){
				return true;
			} else {
				return false;
			}
		}

		void GstreamerVideoSprite::loop( bool flag )
		{
			mLooping = flag;
			if(mLooping){
				mMovie.setLoopMode(LOOP);
			} else {
				mMovie.setLoopMode(NO_LOOP);
			}
		}

		bool GstreamerVideoSprite::isLooping() const
		{
			return mLooping;
		}

		void GstreamerVideoSprite::setVolume( float volume )
		{
			mVolume = volume;
			setMovieVolume();
		}
		
		void GstreamerVideoSprite::setMute( const bool doMute ){
			mMuted = doMute;
			setMovieVolume();
		}

		float GstreamerVideoSprite::getVolume() const
		{
			return mVolume;
		}

		void GstreamerVideoSprite::setStatusCallback(const std::function<void(const Status&)>& fn)
		{
			DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
			mStatusFn = fn;
		}

		void GstreamerVideoSprite::setStatus(const int code)
		{
			if (code == mStatus.mCode) return;

			mStatus.mCode = code;
			mStatusDirty = true;
		}

		void GstreamerVideoSprite::setMovieVolume()
		{
			if (mMuted || mInternalMuted) {
				mMovie.setVolume(0.0f);
			} else {
				mMovie.setVolume(mVolume);
			}
		}

		double GstreamerVideoSprite::currentTime()
		{
			return mMovie.getPosition();
		}

		void GstreamerVideoSprite::unloadVideo(){
			mMovie.stop();
			mMovie.close();
		}

		// set this before loading a video
		void GstreamerVideoSprite::setAlphaMode( bool isTransparent ){
			mIsTransparent = isTransparent;
		}

		void GstreamerVideoSprite::setVideoCompleteCallback( const std::function<void(GstreamerVideoSprite* video)> &func ){
			mVideoCompleteCallback = func;
		}

		void GstreamerVideoSprite::handleVideoComplete(GStreamerWrapper* wrapper){
			if(mVideoCompleteCallback){
				mVideoCompleteCallback(this);
			}
		}



	} // namespace ui
} // namespace ds
