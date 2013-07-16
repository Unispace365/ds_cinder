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

	// returns true if the cache had to create a new entry
	bool					getSize(const std::string& videoPath, int& widthOut, int& heightOut);

protected:
	void					setValue(const std::string& videoPath, const int width, const int height);

private:
	static const int		ERROR_TYPE = 0;
	static const int		TRANSCODE_NEEDS = 1;
	static const int		TRANSCODE_IN_PROGRESS = 2;
	static const int		TRANSCODE_COMPLETE = 3;

	void					load();

	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&			operator=(const VideoMetaCache&);

	std::string				executeCommand(const char* cmd);
	bool					getVideoInfo(const std::string& path, float& outDuration, int& outWidth, int& outHeight, int& valid);
	std::string				parseVariable(std::string varName, std::string breakChar, std::string& stringToParse);

	class Entry {
	public:
		Entry();
		Entry(const std::string& key, const int width, const int height);

		std::string			mKey;
		int					mWidth;
		int					mHeight;
	};
	const std::string		mName;
	std::vector<Entry>		mEntry;
};

} // namespace ui
} // namespace ds

#endif // DS_PROJECTS_VIDEO_GSTREAMER_VIDEOMETACACHE_H_
