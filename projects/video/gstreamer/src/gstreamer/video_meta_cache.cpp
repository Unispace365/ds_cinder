#include "video_meta_cache.h"

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>

#include "gstreamer/_2RealGStreamerWrapper.h"

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
		: mName(name) {
	load();
}

bool VideoMetaCache::getValues(const std::string& videoPath, Type& outType, int& outWidth, int& outHeight, double& duration) {
	for (auto it=mEntry.begin(), end=mEntry.end(); it!=end; ++it) {
		const Entry&	e(*it);
		if (e.mKey == videoPath){
			outType = e.mType;
			outWidth = e.mWidth;
			outHeight = e.mHeight;
			duration = e.mDuration;
			return true;
		}
	}

	try {
		_2RealGStreamerWrapper::GStreamerWrapper	movie;
		movie.open(videoPath, true, false, false, -1);
		int		width = movie.getWidth();
		int		height = movie.getHeight();
		Type	t = VIDEO_TYPE;
		// For now, take no width/height to mean this is an audio type.
		if (width < 1 || height < 1) t = AUDIO_TYPE;
		double	dur = movie.getDurationInMs();
		movie.close();

		setValues(t, videoPath, width, height, dur);

		outType = t;
		outWidth = width;
		outHeight = height;
		duration = dur;

		return false;
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("VideoMetaCache::getWith() error=" << ex.what());
	}
	return 0;
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
