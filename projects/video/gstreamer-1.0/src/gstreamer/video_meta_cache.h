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
	static const enum Type { ERROR_TYPE, AUDIO_TYPE, VIDEO_TYPE };
	VideoMetaCache(const std::string& name);

	// responds with true if it had to go get the values
	bool					getValues(const std::string& videoPath, Type&, int& outWidth, int& outHeight, double& outDuration);

protected:
	void					setValues(const Type, const std::string& videoPath, const int width, const int height, const double duration);

private:
	void					load();


	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&			operator=(const VideoMetaCache&);

	std::string				executeCommand(const char* cmd);
	bool					getVideoInfo(const std::string& path, float& outDuration, int& outWidth, int& outHeight, int& valid);
	std::string				parseVariable(std::string varName, std::string breakChar, std::string& stringToParse);


	class Entry {
	public:
		Entry();
		Entry(const std::string& key, const Type, const int width, const int height, const double duration);

		std::string			mKey;
		Type				mType;
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
