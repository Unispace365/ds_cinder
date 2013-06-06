#include "video_meta_cache.h"

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

#include "video/gstreamer_video_sprite.h"

using namespace dtv;

namespace {

std::string get_db_directory() {
	Poco::Path		p("%USERPROFILE%");
	p.append("documents").append("downstream").append("resources").append("dtv").append("video");
	return Poco::Path::expand(p.toString());
}

std::string get_db_file() {
	Poco::Path		p(get_db_directory());
	p.append("memory.db");
	return p.toString();
}

}

/**
 * \class VideoMetaCache
 */
VideoMetaCache::VideoMetaCache(Globals& g)
	: mGlobals(g)
{
	load();
}

int VideoMetaCache::getWidth(const std::string& game, const int defaultValue)
{
	for (auto it=mEntry.begin(), end=mEntry.end(); it!=end; ++it) {
		const Entry&	e(*it);
		if (e.mKey == game) return e.mValue;
	}

	ds::ui::GstreamerVideoSprite* gsv = new ds::ui::GstreamerVideoSprite(mGlobals.mEngine);
	gsv->loadVideo(game);
	int width = (int)(gsv->getWidth());
	setWidth(game, width);
	//gsv->stop();
	//gsv->unloadVideo();
	//delete gsv;

	return width;
}

void VideoMetaCache::setWidth(const std::string& path, const int value)
{
	if(value < 1){
		DS_LOG_WARNING("Attempted to set the width of a video to an invalid value. Video=" << path);
		return;
	}
	try {
		const std::string				db = get_db_file();
		std::stringstream				buf;
		buf << "SELECT id FROM video_meta WHERE path='" << path << "'";
		ds::query::Result						ans;
		if (ds::query::Client::query(db, buf.str(), ans) && !ans.rowsAreEmpty()) {
			buf.str("");
			buf << "UPDATE video_meta SET width=" << value << " WHERE path='" << path << "'";
			ds::query::Client::queryWrite(db, buf.str(), ans);
			load();
			return;
		}
		buf.str("");
		buf << "INSERT INTO video_meta (path, width) values ('" << path << "', " << value << ")";
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

	const std::string			db(get_db_file());
	f = Poco::File(db);
	if (!f.exists()) {
		f.createFile();
		ds::query::Result				r;
		ds::query::Client::queryWrite(db, "CREATE TABLE video_meta(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL DEFAULT '', width INT NOT NULL DEFAULT '0');", r);
	}

	ds::query::Result					ans;
	ds::query::Client::query(db, "SELECT path,width FROM video_meta", ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		const std::string&		game(it.getString(0));
		if (!game.empty()) mEntry.push_back(Entry(game, it.getInt(1)));
		++it;
	}
}

/**
 * \class HighScore::Entry
 */
VideoMetaCache::Entry::Entry()
	: mValue(0)
{
}

VideoMetaCache::Entry::Entry(const std::string& key, const int value)
	: mKey(key)
	, mValue(value)
{
}
