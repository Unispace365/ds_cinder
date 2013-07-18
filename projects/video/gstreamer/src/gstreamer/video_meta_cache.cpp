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

}

/**
 * \class VideoMetaCache
 */
VideoMetaCache::VideoMetaCache(const std::string& name)
	: mName(name)
{
	load();
}

bool VideoMetaCache::getValues(const std::string& videoPath, int& outWidth, int& outHeight, double& duration)
{
	for (auto it=mEntry.begin(), end=mEntry.end(); it!=end; ++it) {
		const Entry&	e(*it);
		if (e.mKey == videoPath){
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
		double dur = movie.getDurationInMs();
		movie.close();

		setValues(videoPath, width, height, dur);

		outWidth = width;
		outHeight = height;
		duration = dur;

		return false;
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("VideoMetaCache::getWith() error=" << ex.what());
	}
	return 0;
}

void VideoMetaCache::setValues(const std::string& path, const int width, const int height, const double duration)
{
	if(width < 1 || height < 1 || duration < 0.0f){
		DS_LOG_WARNING("Attempted to set the width of a video to an invalid value. Video=" << path);
		return;
	}
	try {
		const std::string				db = get_db_file(mName);
		std::stringstream				buf;
		buf << "SELECT id FROM video_meta WHERE path='" << path << "'";
		ds::query::Result						ans;
		if (ds::query::Client::query(db, buf.str(), ans) && !ans.rowsAreEmpty()) {
			buf.str("");
			buf << "UPDATE video_meta SET width=" << width << ", height=" << height << ", duration=" << duration << " WHERE path='" << path << "'";
			ds::query::Client::queryWrite(db, buf.str(), ans);
			load();
			return;
		}
		buf.str("");
		buf << "INSERT INTO video_meta (path, width, height, duration) values ('" << path << "', " << width << ", " << height << ", " << duration << ")";
		ds::query::Client::queryWrite(db, buf.str(), ans);
	} catch (std::exception const&) {
	}

	load();
}

void VideoMetaCache::load()
{
	mEntry.clear();

	// Load in the high scores
	Poco::File					f(get_db_directory());
	if (!f.exists()) f.createDirectories();

	const std::string			db(get_db_file(mName));
	f = Poco::File(db);
	if (!f.exists()) {
		f.createFile();
		ds::query::Result				r;
		ds::query::Client::queryWrite(db, "CREATE TABLE video_meta(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL DEFAULT '', width INT NOT NULL DEFAULT '0', height INT NOT NULL DEFAULT '0', duration DOUBLE NOT NULL DEFAULT '0');", r);
	}

	ds::query::Result					ans;
	ds::query::Client::query(db, "SELECT path,width,height,duration FROM video_meta", ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		const std::string&		game(it.getString(0));
		if (!game.empty()) mEntry.push_back(Entry(game, it.getInt(1), it.getInt(2), it.getFloat(3)));
		++it;
	}
}

/**
 * \class HighScore::Entry
 */
VideoMetaCache::Entry::Entry()
	: mWidth(0)
	, mHeight(0)
	, mDuration(0.0f)
{
}

VideoMetaCache::Entry::Entry(const std::string& key, const int width, const int height, const double duration)
	: mKey(key)
	, mWidth(width)
	, mHeight(height)
	, mDuration(duration)
{
}

} // namespace ui
} // namespace ds
