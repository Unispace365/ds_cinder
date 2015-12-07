#include "video_meta_cache.h"


// Keep this at the front, can get messed if it comes later
#include "MediaInfoDLL.h"

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>
#include <ds/app/environment.h>
#include <ds/util/string_util.h>

#include "ds/ui/sprite/video.h"

namespace ds {
namespace ui {

namespace {
const std::string&	ERROR_TYPE_SZ() { static const std::string	ANS(""); return ANS; }
const std::string&	AUDIO_TYPE_SZ() { static const std::string	ANS("a"); return ANS; }
const std::string&	VIDEO_TYPE_SZ() { static const std::string	ANS("v"); return ANS; }

std::string get_db_directory() {
	Poco::Path		p("%USERPROFILE%");
	p.append("documents").append("downstream").append("cache").append("video");
	return Poco::Path::expand(p.toString());
}

std::string get_db_file(const std::string& name) {
	Poco::Path		p(get_db_directory());
	p.append(name + ".sqlite");
	return p.toString();
}

VideoMetaCache::Type type_from_db_type(const std::string& t) {
	if (t == AUDIO_TYPE_SZ()) return VideoMetaCache::AUDIO_TYPE;
	if (t == VIDEO_TYPE_SZ()) return VideoMetaCache::VIDEO_TYPE;
	return VideoMetaCache::ERROR_TYPE;
}

const std::string& db_type_from_type(const VideoMetaCache::Type t) {
	if (t == VideoMetaCache::AUDIO_TYPE) return AUDIO_TYPE_SZ();
	if (t == VideoMetaCache::VIDEO_TYPE) return VIDEO_TYPE_SZ();
	return ERROR_TYPE_SZ();
}

}

/**
 * \class VideoMetaCache
 */
VideoMetaCache::VideoMetaCache(const std::string& name)
	: mName(name)
{
	load();
}

bool VideoMetaCache::getValues(const std::string& videoPath, Type& outType, int& outWidth, int& outHeight, double& outDuration, std::string& outColorSpace) {
	for(auto it = mEntries.begin(), end = mEntries.end(); it != end; ++it) {
		const Entry&	e(*it);
		if (e.mPath == videoPath){

// With updates to this system , I don't believe this fallback system is needed.
// Previously, a video could be assumed to be audio if it simply couldn't be found or read.
// Now, actually find audio streams, and never assume stuff like that.
// Also, if a file is type==ERROR_TYPE, it isn't recorded anywhere, so the below condition can't exist
#if 0
			// Sort of a long story, but here's the deal:
			//		1. Try to load a video that doesn't exist yet, type gets saved as audio, width=0, height=0
			//		2. Try to load that same video again, but now it exists. The cache will pull up audio, width & height = 0
			//		3. The video will then never play until the cache is cleared manually.
			//		4. So to fix that, we just re-query media info every time width or height are < 1. 
			//		5. This means audio files are queried every time. If this becomes an issue performance-wise, we can try to short-cut this.
			if(e.mWidth < 1 || e.mHeight < 1){

				Entry newEntry = Entry();
				newEntry.mPath = videoPath;

				if(getVideoInfo(newEntry) && newEntry.mType != ERROR_TYPE){
					if(width > 0 && height > 0){
						setValues(t, videoPath, width, height, duration);
						outType = t;
						outWidth = width;
						outHeight = height;
						outDuration = duration;
						return true;
					}
				}

			}

#endif 

			outType = e.mType;
			outWidth = e.mWidth;
			outHeight = e.mHeight;
			outDuration = e.mDuration;
			outColorSpace = e.mColorSpace;

			return true;
		}
	}


	// The above search for a pre-cached video info failed, so find the video info
	try {

		Entry newEntry = Entry();
		newEntry.mPath = videoPath;

		if(!getVideoInfo(newEntry) || newEntry.mType == ERROR_TYPE){
			return false;
		}

		setValues(newEntry);

		outType = newEntry.mType;
		outWidth = newEntry.mWidth;
		outHeight = newEntry.mHeight;
		outDuration = newEntry.mDuration;
		outColorSpace = newEntry.mColorSpace;
		return true;
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("VideoMetaCache::getWith() error=" << ex.what());
	}
	return true;
}

void VideoMetaCache::setValues(Entry& entry) {
	if(entry.mType == ERROR_TYPE) {
		DS_LOG_WARNING("Attempted to cache an invalid media (path=" << entry.mPath << ")");
		return;
	}
	if(entry.mDuration < 0.0f) {
		DS_LOG_WARNING("Attempted to cache media with no duration (path=" << entry.mPath << ")");
		return;
	}
	if(entry.mType == VIDEO_TYPE && (entry.mWidth < 1 || entry.mHeight < 1)) {
		DS_LOG_WARNING("Attempted to cache video with no size (path=" << entry.mPath << ", width=" << entry.mWidth << ", height=" << entry.mHeight << ")");
		return;
	}
	try {
		const std::string				db = get_db_file(mName);
		std::stringstream				buf;
		buf << "SELECT id FROM video_meta WHERE path='" << entry.mPath << "'";
		ds::query::Result						ans;
		if (ds::query::Client::query(db, buf.str(), ans) && !ans.rowsAreEmpty()) {
			buf.str("");
			buf << "UPDATE video_meta SET type='" << db_type_from_type(entry.mType) 
				<< "', width=" << entry.mWidth 
				<< ", height=" << entry.mHeight 
				<< ", duration=" << entry.mDuration
				<< ", colorspace='" << entry.mColorSpace
				<< "', videocodec='" << entry.mVideoCodec
				<< "', audiocodec='" << entry.mAudioCodec
				<< "' WHERE path='" << entry.mPath << "'";
			ds::query::Client::queryWrite(db, buf.str(), ans);
			load();
			return;
		}
		buf.str("");
		buf << "INSERT INTO video_meta (type, path, width, height, duration, colorspace, videocodec, audiocodec) values ('" << 
			db_type_from_type(entry.mType) << "', '" 
			<< entry.mPath << "', " 
			<< entry.mWidth << ", " 
			<< entry.mHeight << ", " 
			<< entry.mDuration << ", '" 
			<< entry.mColorSpace << "', '" 
			<< entry.mVideoCodec << "', '" 
			<< entry.mAudioCodec << "')";
		ds::query::Client::queryWrite(db, buf.str(), ans);
		mEntries.push_back(entry);
	} catch (std::exception const&) {
	}

	//load();
}

void VideoMetaCache::load() {
	mEntries.clear();

	// Load in the high scores
	Poco::File						f(get_db_directory());
	if (!f.exists()) f.createDirectories();

	const std::string				db(get_db_file(mName));
	f = Poco::File(db);
	if (!f.exists()) {
		f.createFile();
		ds::query::Result			r;
		ds::query::Client::queryWrite(db, "CREATE TABLE video_meta(id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT NOT NULL DEFAULT '', path TEXT NOT NULL DEFAULT '', width INT NOT NULL DEFAULT '0', height INT NOT NULL DEFAULT '0', duration DOUBLE NOT NULL DEFAULT '0', colorspace TEXT NOT NULL DEFAULT '', videocodec TEXT NOT NULL DEFAULT '', audiocodec TEXT NOT NULL DEFAULT '');", r);
	}

	ds::query::Result				ans;
	ds::query::Client::query(db, "SELECT type, path, width, height, duration, colorspace, videocodec, audiocodec FROM video_meta", ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		const std::string&			game(it.getString(1));
		if (!game.empty()) mEntries.push_back(Entry(game, type_from_db_type(it.getString(0)), it.getInt(2), it.getInt(3), it.getFloat(4), it.getString(5), it.getString(6), it.getString(7)));
		++it;
	}
}

bool VideoMetaCache::getVideoInfo(Entry& entry) {
	MediaInfoDLL::MediaInfo		media_info;
	if (!media_info.IsReady()) {
		// Indicates the DLL couldn't be loaded
		DS_LOG_ERROR("VideoMetaCache::getVideoInfo() MediaInfo not loaded, does dll/MediaInfo.dll exist in the app folder?");
		throw std::runtime_error("VideoMetaCache::getVideoInfo() MediaInfo not loaded, does dll/MediaInfo.dll exist in the app folder?");
		return false;
	}
	media_info.Open(ds::wstr_from_utf8(entry.mPath));
	
	size_t numAudio = media_info.Count_Get(MediaInfoDLL::Stream_Audio);
	size_t numVideo = media_info.Count_Get(MediaInfoDLL::Stream_Video);

	if(numAudio < 1 && numVideo < 1){
		DS_LOG_WARNING("Couldn't find any audio or video streams in " << entry.mPath);
		entry.mType = ERROR_TYPE;
		return false;
	}

	// If there's an audio channel, get it's codec
	if(numAudio > 0){
		entry.mAudioCodec = ds::utf8_from_wstr(media_info.Get(MediaInfoDLL::Stream_Audio, 0, L"Codec", MediaInfoDLL::Info_Text));
	}


	// No video streams, but has at least one audio stream is a AUDIO_TYPE
	if(numAudio > 0 && numVideo < 1){
		entry.mType = AUDIO_TYPE;
		entry.mWidth = 0;
		entry.mHeight = 0;
		if(!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Audio, 0, L"Duration", MediaInfoDLL::Info_Text), entry.mDuration)) return false;

	// Any number of audio streams and at least one video streams is VIDEO_TYPE
	} else if(numVideo > 0){
		entry.mType = VIDEO_TYPE;
		if(!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Width", MediaInfoDLL::Info_Text), entry.mWidth)) return false;
		if(!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Height", MediaInfoDLL::Info_Text), entry.mHeight)) return false;

		// We don't check errors on these, cause they're not required by gstreamer to play a video, they're just nice-to-have
		entry.mVideoCodec = ds::utf8_from_wstr(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Codec", MediaInfoDLL::Info_Text));
		entry.mColorSpace = ds::utf8_from_wstr(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Colorimetry", MediaInfoDLL::Info_Text));
		ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Duration", MediaInfoDLL::Info_Text), entry.mDuration);
	}

	entry.mDuration /= 1000.0f;

	// Disabling duration check, if MediaInfo can't find it, that's ok, GStreamer can fill it in
// 	if(entry.mDuration <= 0.0f || entry.mDuration > 360000.0f) {  // 360000.0f == 100 hours. That should be enough, right?
// 		DS_LOG_WARNING("VideoMetaCache::getVideoInfo() illegal duration (" << entry.mDuration << ") for file (" << entry.mPath << ")");
// 		return false;
// 	}
	return true;
}

/**
 * \class HighScore::Entry
 */
VideoMetaCache::Entry::Entry()
	: mType(ERROR_TYPE)
	, mWidth(0)
	, mHeight(0)
	, mDuration(0.0f) {
}

VideoMetaCache::Entry::Entry(const std::string& key, const Type t, const int width, const int height, const double duration, const std::string& colorSpace, const std::string& videoCodec, const std::string& audioCodec)
	: mPath(key)
	, mType(t)
	, mWidth(width)
	, mHeight(height)
	, mDuration(duration) 
	, mAudioCodec(audioCodec)
	, mVideoCodec(videoCodec)
	, mColorSpace(colorSpace)
{
}

} // namespace ui
} // namespace ds
