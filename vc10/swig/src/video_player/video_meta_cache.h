#pragma once
#ifndef DTV_VIDEO_VIDEO_META_CACHE
#define DTV_VIDEO_VIDEO_META_CACHE

#include <vector>

/**
 * \class VideoMetaCache
 * \b Store values for video to be quickly looked up later.
 */

namespace dtv {
	class Globals;

class VideoMetaCache
{
public:
	VideoMetaCache(Globals& g);

	int						getWidth(const std::string& videoPath, const int defaultValue = -1);

protected:
	void						setWidth(const std::string& videoPath, const int value);

private:
	void						load();

	VideoMetaCache(const VideoMetaCache&);
	VideoMetaCache&					operator=(const VideoMetaCache&);

	class Entry {
	public:
		Entry();
		Entry(const std::string& key, const int value);

		std::string		mKey;
		int			mValue;
	};
	std::vector<Entry>	mEntry;
	Globals&			mGlobals;
};

} // namespace dtv

#endif // DTV_VIDEO_VIDEO_META_CACHE