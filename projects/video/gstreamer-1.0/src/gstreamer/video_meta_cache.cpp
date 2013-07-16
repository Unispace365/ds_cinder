#include "video_meta_cache.h"

#include <sstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>
#include <ds/debug/logger.h>
#include <ds/app/environment.h>

//#include "gstreamer/_2RealGStreamerWrapper.h"

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

bool VideoMetaCache::getSize(const std::string& videoPath, int& widthOut, int& heightOut)
{
	for (auto it=mEntry.begin(), end=mEntry.end(); it!=end; ++it) {
		const Entry&	e(*it);
		if (e.mKey == videoPath){
			widthOut = e.mWidth;
			heightOut = e.mHeight;
			return false;
		}
	}

	try {
		//_2RealGStreamerWrapper::GStreamerWrapper	movie;
		//movie.open(videoPath, true, false, false, -1, -1);
		//int		width = movie.getWidth();
		//int		height = movie.getHeight();

		int width(0), height(0), valid(0);
		float duration(0.0f);

		if(!getVideoInfo(videoPath, duration, width, height, valid)){
			DS_LOG_WARNING("Error finding video info!");
			return false;
		}

		setValue(videoPath, width, height);

		widthOut = width;
		heightOut = height;
		return true;
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("VideoMetaCache::getWith() error=" << ex.what());
	}
	return true;
}

void VideoMetaCache::setValue(const std::string& path, const int widthy, const int heighty)
{
	if(widthy < 1 || heighty < 1){
		DS_LOG_WARNING("Attempted to set the width or height of a video to an invalid value. Video=" << path);
		return;
	}
	try {
		const std::string				db = get_db_file(mName);
		std::stringstream				buf;
		buf << "SELECT id FROM video_meta WHERE path='" << path << "'";
		ds::query::Result						ans;
		if (ds::query::Client::query(db, buf.str(), ans) && !ans.rowsAreEmpty()) {
			buf.str("");
			buf << "UPDATE video_meta SET width=" << widthy << ", height=" << heighty << " WHERE path='" << path << "'";
			ds::query::Client::queryWrite(db, buf.str(), ans);
			load();
			return;
		}
		buf.str("");
		buf << "INSERT INTO video_meta (path, width, height) values ('" << path << "', " << widthy << ", " << heighty << ")";
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
		ds::query::Client::queryWrite(db, "CREATE TABLE video_meta(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL DEFAULT '', width INT NOT NULL DEFAULT '0', height INT NOT NULL DEFAULT '0');", r);
	}

	ds::query::Result					ans;
	ds::query::Client::query(db, "SELECT path,width,height FROM video_meta", ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		const std::string&		game(it.getString(0));
		if (!game.empty()) mEntry.push_back(Entry(game, it.getInt(1), it.getInt(2)));
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


bool VideoMetaCache::getVideoInfo(const std::string& path, float& outDuration, int& outWidth, int& outHeight, int& valid){
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
	//int ret = system(cdPath);
	//std::string  pPath = "cd " + ofToDataPath("..\\", true);
	//cout << "ffprobe path = " << pPath << endl;
	//const char* cPath = pPath.c_str();
	//system(cPath);
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
		float seconds = atof(duration.substr(5).c_str());
		outDuration = static_cast<float>(hours * 60 * 60) + static_cast<float>(minutes * 60) + seconds;
	}

	// disabling duration checking for gstreamer caching	
	//if (outDuration < 0 || outDuration > 360000){
	//	DS_LOG_WARNING("Duration is negative or too long (must be less than 100 hours in length) " << outDuration);	
	//	return false;
	//}
	//} 
	if (outWidth < 1 || outHeight < 1){
		DS_LOG_WARNING("Width or height are invalid, width=" << outWidth << " height=" << outHeight);
		return false;
	}

	return true;
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
	: mWidth(0)
	, mHeight(0)
{
}

VideoMetaCache::Entry::Entry(const std::string& key, const int widthy, const int height)
	: mKey(key)
	, mWidth(widthy)
	, mHeight(height)
{
}

} // namespace ui
} // namespace ds
