#pragma once
#ifndef DS_VIDEO_GSTREAMER_VIDEO_SPRITE
#define DS_VIDEO_GSTREAMER_VIDEO_SPRITE

#include "ds/ui/sprite/sprite.h"
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "ds/data/resource.h"

#include "_2RealGStreamerWrapper.h"

namespace ds {
	namespace ui {

		class GstreamerVideoSprite : public Sprite
		{
		public:
			GstreamerVideoSprite( SpriteEngine& );
			~GstreamerVideoSprite();
			void				setAlphaMode(bool isTransparent);// set this before loading a video
			void				setSize( float width, float height );
			virtual void		updateServer(const UpdateParams&);
			void				drawLocalClient();

			GstreamerVideoSprite&              loadVideo( const std::string &filename, int videoWidth = -1 );
			GstreamerVideoSprite              &setResourceId(const ds::Resource::Id &resourceId);

			void				unloadVideo();

			void				play();
			void				stop();
			void				pause();
			void				seek(float t);
			double				duration();
			double				currentTime();
			bool				isPlaying();
			void				loop(bool flag);
			bool				isLooping() const;
			// value between 0.0f & 1.0f
			void				setVolume(float volume);
			float				getVolume() const;

			void				setMute(const bool doMute);

			struct Status {
				static const int  STATUS_STOPPED = 0;
				static const int  STATUS_PLAYING = 1;
				static const int  STATUS_PAUSED  = 2;
				int               mCode;
			};
			void				setStatusCallback(const std::function<void(const Status&)>&);

			void				setVideoCompleteCallback(const std::function<void(GstreamerVideoSprite* video)> &func);

		private:
			typedef Sprite inherited;

			void                setStatus(const int);
			void				setMovieVolume();

			_2RealGStreamerWrapper::GStreamerWrapper	mMovie;

			ci::gl::Texture     mFrameTexture;
			ci::gl::Fbo         mFbo;

			bool                mLooping;
			// User-driven mute state
			bool				mMuted;
			// A mute state that gets turned on automatically in certain situations
			bool                mInternalMuted;
			float               mVolume;
			bool				mIsTransparent;

			Status              mStatus;
			bool                mStatusDirty;
			std::function<void(const Status&)>
				mStatusFn;

			std::function<void(GstreamerVideoSprite*)> mVideoCompleteCallback;
			void				handleVideoComplete(_2RealGStreamerWrapper::GStreamerWrapper* wrapper);
		};

	} // namespace ui
} // namespace ds

#endif//DS_VIDEO_H
