#pragma once
#ifndef DS_UI_SPRITE_VIDEO_GSTREAMER_VIDEOSPRITEGSTREAMER_H_
#define DS_UI_SPRITE_VIDEO_GSTREAMER_VIDEOSPRITEGSTREAMER_H_

#include <ds/ui/sprite/sprite.h>
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "ds/data/resource.h"

namespace _2RealGStreamerWrapper
{
class GStreamerWrapper;
}

namespace ds {
	namespace ui {
	
		class Video : public Sprite
		{
		public:
			Video( SpriteEngine& );
			~Video();
			void				setAlphaMode(bool isTransparent);// set this before loading a video
			void				setSize( float width, float height );
			virtual void		updateServer(const UpdateParams&);
			void				drawLocalClient();

			Video&              loadVideo( const std::string &filename);
			Video              &setResourceId(const ds::Resource::Id &resourceId);

			void				unloadVideo();

			void				play();
			void				stop();
			void				pause();
			void				seek(double t); // 0.0 to 1.0 value
			double				duration();
			double				currentTime();
			bool				isPlaying();
			void				loop(bool flag);
			bool				isLooping() const;
			void				setVolume(float volume); // value between 0.0f & 1.0f
			float				getVolume() const;

			void				setMute(const bool doMute);

			// Loads a video and waits for a single frame to be loaded into texture memory.
			// After that, the video is stopped and unloaded. This is basically a "show poster frame" or a wait to create a thumbnail.
			void				generateSingleFrame( const std::string &filename);
			const bool			isGeneratingSingleFrame() const { return mGeneratingSingleFrame; }

			struct Status {
				static const int  STATUS_STOPPED = 0;
				static const int  STATUS_PLAYING = 1;
				static const int  STATUS_PAUSED  = 2;
				int               mCode;
			};
			void				setStatusCallback(const std::function<void(const Status&)>&);

			void				setVideoCompleteCallback(const std::function<void(Video* video)> &func);

		private:
			typedef Sprite inherited;

			void                setStatus(const int);
			void				setMovieVolume();

			// Done this way so I can completely hide any dependencies
			_2RealGStreamerWrapper::GStreamerWrapper*	mMoviePtr;
			_2RealGStreamerWrapper::GStreamerWrapper&	mMovie;

			ci::gl::Texture     mFrameTexture;
			ci::gl::Fbo         mFbo;
			bool				mFboCreated;

			bool                mLooping;
			// User-driven mute state
			bool				mMuted;
			// A mute state that gets turned on automatically in certain situations
			bool                mInternalMuted;
			float               mVolume;
			bool				mIsTransparent;

			bool				mGeneratingSingleFrame;

			Status              mStatus;
			bool                mStatusDirty;
			std::function<void(const Status&)>
				mStatusFn;

			std::function<void(Video*)> mVideoCompleteCallback;
			void				handleVideoComplete(_2RealGStreamerWrapper::GStreamerWrapper* wrapper);
		};

	} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_VIDEO_GSTREAMER_VIDEOSPRITEGSTREAMER_H_
