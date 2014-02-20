#include "video_meta_cache.h"

#define USE_MEDIAINFO		(1)
//#define USE_FFPROBE			(1)

#ifdef USE_MEDIAINFO
// Keep this at the front, can get messed if it comes later
#include "MediaInfoDLL.h"
#endif // USE_MEDIAINFO

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>
#include <ds/app/environment.h>
#include <ds/util/string_util.h>

//#include "gstreamer/_2RealGStreamerWrapper.h"

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

bool VideoMetaCache::getValues(const std::string& videoPath, Type& outType, int& outWidth, int& outHeight, double& outDuration) {
	for (auto it=mEntry.begin(), end=mEntry.end(); it!=end; ++it) {
		const Entry&	e(*it);
		if (e.mKey == videoPath){
			outType = e.mType;
			outWidth = e.mWidth;
			outHeight = e.mHeight;
			outDuration = e.mDuration;
			return true;
		}
	}

	try {
		//_2RealGStreamerWrapper::GStreamerWrapper	movie;
		//movie.open(videoPath, true, false, false, -1, -1);
		//int		width = movie.getWidth();
		//int		height = movie.getHeight();

		int width(0), height(0), valid(0);
		Type	t = VIDEO_TYPE;
		float duration(0.0f);

		getVideoInfo(videoPath, duration, width, height, valid);
		if(width < 1 || height < 1) t = AUDIO_TYPE;


		setValues(t, videoPath, width, height, duration);

		outWidth = width;
		outHeight = height;
		outDuration = duration;
		return true;
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("VideoMetaCache::getWith() error=" << ex.what());
	}
	return true;
}

void VideoMetaCache::setValues(const Type t, const std::string& path, const int width, const int height, const double duration) {
	if (t == ERROR_TYPE) {
		DS_LOG_WARNING("Attempted to cache an invalid media (path=" << path << ")");
		return;
	}
	if (duration < 0.0f) {
		DS_LOG_WARNING("Attempted to cache media with no duration (path=" << path << ")");
		return;
	}
	if (t == VIDEO_TYPE && (width < 1 || height < 1)) {
		DS_LOG_WARNING("Attempted to cache video with no size (path=" << path << ", width=" << width << ", height=" << height << ")");
		return;
	}
	try {
		const std::string				db = get_db_file(mName);
		std::stringstream				buf;
		buf << "SELECT id FROM video_meta WHERE path='" << path << "'";
		ds::query::Result						ans;
		if (ds::query::Client::query(db, buf.str(), ans) && !ans.rowsAreEmpty()) {
			buf.str("");
			buf << "UPDATE video_meta SET type='" << db_type_from_type(t) << "', width=" << width << ", height=" << height << ", duration=" << duration << " WHERE path='" << path << "'";
			ds::query::Client::queryWrite(db, buf.str(), ans);
			load();
			return;
		}
		buf.str("");
		buf << "INSERT INTO video_meta (type, path, width, height, duration) values ('" << db_type_from_type(t) << "', '" << path << "', " << width << ", " << height << ", " << duration << ")";
		ds::query::Client::queryWrite(db, buf.str(), ans);
	} catch (std::exception const&) {
	}

	load();
}

void VideoMetaCache::load() {
	mEntry.clear();

	// Load in the high scores
	Poco::File						f(get_db_directory());
	if (!f.exists()) f.createDirectories();

	const std::string				db(get_db_file(mName));
	f = Poco::File(db);
	if (!f.exists()) {
		f.createFile();
		ds::query::Result			r;
		ds::query::Client::queryWrite(db, "CREATE TABLE video_meta(id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT NOT NULL DEFAULT '', path TEXT NOT NULL DEFAULT '', width INT NOT NULL DEFAULT '0', height INT NOT NULL DEFAULT '0', duration DOUBLE NOT NULL DEFAULT '0');", r);
	}

	ds::query::Result				ans;
	ds::query::Client::query(db, "SELECT type,path,width,height,duration FROM video_meta", ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		const std::string&			game(it.getString(1));
		if (!game.empty()) mEntry.push_back(Entry(game, type_from_db_type(it.getString(0)), it.getInt(2), it.getInt(3), it.getFloat(4)));
		++it;
	}
}


std::string VideoMetaCache::executeCommand(const char* cmd){
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return "ERROR";
	char buffer[128];
	std::string result = "";
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
}


bool VideoMetaCache::getVideoInfo(const std::string& path, float& outDuration, int& outWidth, int& outHeight, int& valid) {
#ifdef USE_MEDIAINFO
	try {
		MediaInfoDLL::MediaInfo		media_info;
		if (!media_info.IsReady()) {
			// Indicates the DLL couldn't be loaded
			DS_LOG_ERROR("VideoMetaCache::getVideoInfo() MediaInfo not loaded, does dll/MediaInfo.dll exist in the app folder?");
			return false;
		}
		media_info.Open(ds::wstr_from_utf8(path));
		if (!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Width", MediaInfoDLL::Info_Text), outWidth)) return false;
		if (!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Height", MediaInfoDLL::Info_Text), outHeight)) return false;
		if (!ds::wstring_to_value(media_info.Get(MediaInfoDLL::Stream_Video, 0, L"Duration", MediaInfoDLL::Info_Text), outDuration)) return false;
		outDuration /= 1000.0f;
		if (outDuration <= 0.0f || outDuration > 360000.0f) {
			DS_LOG_WARNING("VideoMetaCache::getVideoInfo() illegal duration (" << outDuration << ") for file (" << path << ")");
			return false;
		}
		return true;
	} catch (std::exception const&) {
	}
	return false;
#elif defined USE_FFPROBE
	// ffprobe.exe needs to be in the bin folder
	// "-show_streams" will print info about every stream
	// "sexagesimal" prints time in HH:MM:SS.MICROSECONDS
	// "-print_format compact" forces the format into a smaller form to be parsed
	// path is the path to the file
	// "2>&1" ffprobe prints everything in stderror, "2>" redirects stderror to stdout, or "&1"
	//	valid = ERROR_TYPE;
	std::stringstream ss;
	std::string pPath = ds::Environment::getAppFolder("ffprobe/", "");
	ss << "cd \"" << pPath << "\"";
	pPath = ss.str();
	const char* cdPath = pPath.c_str();

	ss.str("");
	ss << "ffprobe.exe -show_streams -pretty -sexagesimal -pretty -print_format compact -i \"" + path  + "\" 2>&1"; 
	std::string s = ss.str();
	const char* c = s.c_str();
	std::string out = executeCommand(c);
	if (out == "ERROR"){
		return false;
	}

	std::string width = parseVariable("width=", "|", out);
	outWidth = atoi(width.c_str());

	std::string height = parseVariable("height=", "|", out);
	outHeight = atoi(height.c_str());

	std::string duration = parseVariable("duration=", "|", out);
	if(duration.size() < 5){
		std::cout << "invalid duration: " << duration << " for " << path << std::endl;
	} else {
		int hours = atoi(duration.substr(0,1).c_str());
		int minutes = atoi(duration.substr(2,2).c_str());
		float seconds = static_cast<float>(atof(duration.substr(5).c_str()));
		outDuration = static_cast<float>(hours * 60 * 60) + static_cast<float>(minutes * 60) + seconds;
	}

	// disabling duration checking for gstreamer caching	
	if (outDuration < 0 || outDuration > 360000){
		DS_LOG_WARNING("Duration is negative or too long (must be less than 100 hours in length) " << outDuration);	
		return false;
	}
	//} 
	//if (outWidth < 1 || outHeight < 1){
	//	DS_LOG_WARNING("Width or height are invalid, width=" << outWidth << " height=" << outHeight << " (output was " << out << ")");
	//	return false;
	//}

	return true;
#endif
}

std::string VideoMetaCache::parseVariable(std::string varName, std::string breakChar, std::string& stringToParse){
	int pos = stringToParse.find(varName);
	if(pos > 0){
		std::string output = stringToParse.substr(pos+varName.length());
		pos = output.find(breakChar);
		if(pos > 0){
			output = output.substr(0, pos);
			return output;
		}
	}
	return "";
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

VideoMetaCache::Entry::Entry(const std::string& key, const Type t, const int width, const int height, const double duration)
	: mKey(key)
	, mType(t)
	, mWidth(width)
	, mHeight(height)
	, mDuration(duration) {
}

} // namespace ui
} // namespace ds
