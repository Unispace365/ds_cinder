#include "stdafx.h"

#include "video_meta_cache.h"


// Keep this at the front, can get messed if it comes later
#include "MediaInfoDLL/MediaInfoDLL.h"

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>
#include <ds/app/environment.h>
#include <ds/util/string_util.h>

#include "ds/ui/sprite/video.h"


// Win32 has UNICODE defined, meaning MediaInfo strings are wstring
// Linux just uses utf-8 strings.
#if defined(UNICODE) || defined(_UNICODE)
	#define _MI_TO_STR(__x) ds::utf8_from_wstr(__x)
	#define _STR_TO_MI(__x) ds::wstr_from_utf8(__x)
	#define _MI_TO_VALUE ds::wstring_to_value
#else
	#define _MI_TO_STR(__x) __x
	#define _STR_TO_MI(__x) __x
	#define _MI_TO_VALUE ds::string_to_value
#endif


namespace ds {
namespace ui {

namespace {
const std::string&	ERROR_TYPE_SZ() { static const std::string	ANS(""); return ANS; }
const std::string&	AUDIO_ONLY_TYPE_SZ() { static const std::string	ANS("a"); return ANS; }
const std::string&	VIDEO_ONLY_TYPE_SZ() { static const std::string	ANS("v"); return ANS; }
const std::string&	VIDEO_AND_AUDIO_TYPE_SZ() { static const std::string	ANS("va"); return ANS; }

std::string get_db_directory() {
	Poco::Path		p(Poco::Path::home());
	p.append("documents").append("downstream").append("cache").append("video");
	return Poco::Path::expand(p.toString());
}

std::string get_db_file(const std::string& name) {
	Poco::Path		p(get_db_directory());
	p.append(name + ".sqlite");
	return p.toString();
}

VideoMetaCache::Type type_from_db_type(const std::string& t) {
	if(t == VIDEO_AND_AUDIO_TYPE_SZ()) return VideoMetaCache::VIDEO_AND_AUDIO_TYPE;
	if(t == AUDIO_ONLY_TYPE_SZ()) return VideoMetaCache::AUDIO_ONLY_TYPE;
	if(t == VIDEO_ONLY_TYPE_SZ()) return VideoMetaCache::VIDEO_ONLY_TYPE;
	return VideoMetaCache::ERROR_TYPE;
}

const std::string& db_type_from_type(const VideoMetaCache::Type t) {
	if(t == VideoMetaCache::VIDEO_AND_AUDIO_TYPE) return VIDEO_AND_AUDIO_TYPE_SZ();
	if(t == VideoMetaCache::AUDIO_ONLY_TYPE) return AUDIO_ONLY_TYPE_SZ();
	if(t == VideoMetaCache::VIDEO_ONLY_TYPE) return VIDEO_ONLY_TYPE_SZ();
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
	if( (entry.mType == VIDEO_ONLY_TYPE || entry.mType == VIDEO_AND_AUDIO_TYPE) && (entry.mWidth < 1 || entry.mHeight < 1)) {
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
		return false;
	}
	media_info.Open(_STR_TO_MI(entry.mPath));
	
	size_t numAudio = media_info.Count_Get(MediaInfoDLL::Stream_Audio);
	size_t numVideo = media_info.Count_Get(MediaInfoDLL::Stream_Video);

	if(numAudio < 1 && numVideo < 1){
		DS_LOG_WARNING("Couldn't find any audio or video streams in " << entry.mPath);
		entry.mType = ERROR_TYPE;
		return false;
	}

	// If there's an audio channel, get it's codec
	if(numAudio > 0){
		entry.mAudioCodec = _MI_TO_STR(media_info.Get(MediaInfoDLL::Stream_Audio, 0, __T("Codec"), MediaInfoDLL::Info_Text));

		//std::cout << "Number of audio channels: " << _MI_TO_STR(media_info.Get(MediaInfoDLL::Stream_Audio, 0, __T("Channels"), MediaInfoDLL::Info_Text)) << std::endl;
	}

	// No video streams, but has at least one audio stream is a AUDIO_TYPE
	if(numAudio > 0 && numVideo < 1){
		entry.mType = AUDIO_ONLY_TYPE;
		entry.mWidth = 0;
		entry.mHeight = 0;
		if(!_MI_TO_VALUE(media_info.Get(MediaInfoDLL::Stream_Audio, 0, __T("Duration"), MediaInfoDLL::Info_Text), entry.mDuration)) return false;

	// Any number of audio streams and at least one video streams is VIDEO_TYPE
	} else if(numVideo > 0){
		if(numAudio > 0){
			entry.mType = VIDEO_AND_AUDIO_TYPE;
		} else {
			entry.mType = VIDEO_ONLY_TYPE;
		}
		if(!_MI_TO_VALUE(media_info.Get(MediaInfoDLL::Stream_Video, 0, __T("Width"), MediaInfoDLL::Info_Text), entry.mWidth)) return false;
		if(!_MI_TO_VALUE(media_info.Get(MediaInfoDLL::Stream_Video, 0, __T("Height"), MediaInfoDLL::Info_Text), entry.mHeight)) return false;

		// We don't check errors on these, cause they're not required by gstreamer to play a video, they're just nice-to-have
		entry.mVideoCodec = _MI_TO_STR(media_info.Get(MediaInfoDLL::Stream_Video, 0, __T("Codec"), MediaInfoDLL::Info_Text));
		entry.mColorSpace = _MI_TO_STR(media_info.Get(MediaInfoDLL::Stream_Video, 0, __T("Colorimetry"), MediaInfoDLL::Info_Text));
		_MI_TO_VALUE(media_info.Get(MediaInfoDLL::Stream_Video, 0, __T("Duration"), MediaInfoDLL::Info_Text), entry.mDuration);
	}

	entry.mDuration /= 1000.0f;

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
