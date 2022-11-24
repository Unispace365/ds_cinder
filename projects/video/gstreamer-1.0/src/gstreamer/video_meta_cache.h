#pragma once
#ifndef DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
#define DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_

#include <string>
#include <vector>

/**
 * \class VideoMetaCache
 * \b Store values for video to be quickly looked up later.
 */
namespace ds { namespace ui {
	class SpriteEngine;

	class VideoMetaCache {

	  public:
		enum Type { ERROR_TYPE, AUDIO_ONLY_TYPE, VIDEO_ONLY_TYPE, VIDEO_AND_AUDIO_TYPE };
		VideoMetaCache(const std::string& name);

		/// responds with true if it had to go get the values
		bool getValues(const std::string& videoPath, Type&, int& outWidth, int& outHeight, double& outDuration,
					   std::string& outColorSpace);

		class Entry {
		  public:
			Entry();
			Entry(const std::string& path, const Type, const int width, const int height, const double duration,
				  const std::string& colorspace, const std::string& codec, const std::string& audioCodec);

			std::string mPath;
			Type		mType;
			int			mWidth;
			int			mHeight;
			double		mDuration;
			std::string mColorSpace;
			std::string mVideoCodec;
			std::string mAudioCodec;
		};

		/// Fills out the Entry with values, you must supply an absolute file path
		/// This does not add to the cache, simply gets the info for a single file
		/// Returns true if the type could be found, and if it's a video and the w/h was detected
		static bool getVideoInfo(Entry&);

	  protected:
		void setValues(Entry&);

	  private:
		void load();


		VideoMetaCache(const VideoMetaCache&);
		VideoMetaCache& operator=(const VideoMetaCache&);


		const std::string  mName;
		std::vector<Entry> mEntries;
	};

}} // namespace ds::ui

#endif // DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
