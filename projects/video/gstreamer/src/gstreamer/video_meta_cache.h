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
public:
	VideoMetaCache(const std::string& name);

	// responds with true if it had to go get the values
	bool					getValues(const std::string& videoPath, int& outWidth, int& outHeight, double& outDuration);

protected:
	void					setValues(const std::string& videoPath, const int width, const int height, const double duration);

private:
	void					load();

	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&			operator=(const VideoMetaCache&);

	class Entry {
	public:
		Entry();
		Entry(const std::string& key, const int width, const int height, const double duration);

		std::string			mKey;
		int					mWidth;
		int					mHeight;
		double				mDuration;
	};
	const std::string		mName;
	std::vector<Entry>		mEntry;
};

} // namespace ui
} // namespace ds

#endif // DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
