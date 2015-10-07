#pragma once
#ifndef DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
#define DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_

#include <vector>

/**
 * \class VideoMetaCache
 * \b Store values for video to be quickly looked up later.
 */
namespace ds {
namespace ui {
class SpriteEngine;

class VideoMetaCache
{

private:
	class Entry;

public:
	static const enum Type { ERROR_TYPE, AUDIO_TYPE, VIDEO_TYPE };
	VideoMetaCache(const std::string& name);

	// responds with true if it had to go get the values
	bool					getValues(const std::string& videoPath, Type&, int& outWidth, int& outHeight, double& outDuration, std::string& outColorSpace);

protected:
	void					setValues(Entry&);

private:
	void					load();


	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&			operator=(const VideoMetaCache&);



	class Entry {
	public:
		Entry();
		Entry(const std::string& path, const Type, const int width, const int height, const double duration, const std::string& colorspace, const std::string& codec, const std::string& audioCodec);

		std::string			mPath;
		Type				mType;
		int					mWidth;
		int					mHeight;
		double				mDuration;
		std::string			mColorSpace;
		std::string			mVideoCodec;
		std::string			mAudioCodec;
	};
	const std::string		mName;
	std::vector<Entry>		mEntries;


	bool					getVideoInfo(Entry&);
};

} // namespace ui
} // namespace ds

#endif // DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
