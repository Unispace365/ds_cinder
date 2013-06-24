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

	int						getWidth(const std::string& videoPath, const int defaultValue = -1);

protected:
	void					setWidth(const std::string& videoPath, const int value);

private:
	void					load();

	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&			operator=(const VideoMetaCache&);

	class Entry {
	public:
		Entry();
		Entry(const std::string& key, const int value);

		std::string			mKey;
		int					mValue;
	};
	const std::string		mName;
	std::vector<Entry>		mEntry;
};

} // namespace ui
} // namespace ds

#endif // DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
