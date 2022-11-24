#include "stdafx.h"

#include "ds/debug/logger.h"

#include "ds/app/environment.h"
#include "ds/cfg/settings.h"
#include "ds/util/string_util.h"
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Path.h>
#include <Poco/Semaphore.h>
#include <Poco/String.h>
#include <fstream>
#include <iostream>

using namespace ds;
// using namespace std;

const ds::BitMask ds::GENERAL_LOG = ds::Logger::newModule("general");
const ds::BitMask ds::IO_LOG	  = ds::Logger::newModule("io");
const ds::BitMask ds::IMAGE_LOG	  = ds::Logger::newModule("image");
const ds::BitMask ds::VIDEO_LOG	  = ds::Logger::newModule("video");

namespace {
const std::string EMPTY_SZ("");

// Special error codes for levels so we can communicate to the thread
const int LOG_LEVEL_ERROR_CODE = -1;
const int LOG_LEVEL_BLOCK_CODE = LOG_LEVEL_ERROR_CODE - 1;

const int LEVEL_SIZE = 5;
// Only assign during setup()
bool		HAS_LEVEL[LEVEL_SIZE];
int			VERBOSE_LEVEL = 0;
ds::BitMask HAS_MODULE	  = ds::BitMask::newFilled();
bool		HAS_ASYNC	  = true;
std::string LOG_FILE;

Poco::Semaphore BLOCK_SEM(0, 1);

// Maintain the modules associated with names so I can let the user know what's available
std::map<int, std::string>* MODULE_MAP = nullptr;
} // namespace

/* DS::LOGGER static
 ******************************************************************/
static void setup_level(const std::string& level) {
	std::string s = Poco::trim(level);
	Poco::toLowerInPlace(s);
	if (s == "all") {
		for (int k = 0; k < LEVEL_SIZE; ++k)
			HAS_LEVEL[k] = true;
	} else if (s == "info")
		HAS_LEVEL[ds::Logger::LOG_INFO] = true;
	else if (s == "warning")
		HAS_LEVEL[ds::Logger::LOG_WARNING] = true;
	else if (s == "error")
		HAS_LEVEL[ds::Logger::LOG_ERROR] = true;
	else if (s == "fatal")
		HAS_LEVEL[ds::Logger::LOG_FATAL] = true;
	else if (s == "metric")
		HAS_LEVEL[ds::Logger::LOG_METRIC] = true;
}

static void setup_module(const std::string& module) {
	const std::string s = Poco::trim(module);
	int				  v;
	if (s == "all")
		HAS_MODULE = ds::BitMask::newFilled();
	else if (ds::string_to_value(s, v))
		HAS_MODULE |= ds::BitMask(v);
}

static const std::string& level_name(const int level) {
	static const std::string INFO("info   "), WARNING("warning"), ERROR_("error  "), FATAL("fatal  "),
		METRIC("metric "), STARTUP("startup"), UNKNOWN("       ");
	if (level == ds::Logger::LOG_INFO) return INFO;
	if (level == ds::Logger::LOG_WARNING) return WARNING;
	if (level == ds::Logger::LOG_ERROR) return ERROR_;
	if (level == ds::Logger::LOG_FATAL) return FATAL;
	if (level == ds::Logger::LOG_METRIC) return METRIC;
	if (level == ds::Logger::LOG_STARTUP) return STARTUP;
	return UNKNOWN;
}

static bool ends_in_separator(const std::string& file) {
	if (file.empty()) return false;
	const char end = file[file.length() - 1];
	if (end == Poco::Path::separator()) return true;
	// take either slash or backslash, since every tends to use them interchangeably
	if (end == '\\') return true;
	if (end == '/') return true;
	return false;
}

void ds::Logger::setup(ds::cfg::Settings& settings) {
	for (int k = 0; k < LEVEL_SIZE; ++k)
		HAS_LEVEL[k] = false;

	std::string level = settings.getString("logger:level"), module = settings.getString("logger:module"),
				async = settings.getString("logger:async"), file = settings.getString("logger:file");

	VERBOSE_LEVEL = settings.getInt("logger:verbose_level");
	if (VERBOSE_LEVEL < 0) VERBOSE_LEVEL = 0;
	if (VERBOSE_LEVEL > 9) VERBOSE_LEVEL = 9;
	if (VERBOSE_LEVEL > 0) {
		std::cout << "Log verbosity is now " << std::to_string(VERBOSE_LEVEL) << std::endl;
	}

	ds::tokenize(level, ',', [](const std::string& s) { setup_level(s); });
	if (!module.empty()) {
		HAS_MODULE = ds::BitMask::newEmpty();
		ds::tokenize(module, ',', [](const std::string& s) { setup_module(s); });
	}

	Poco::trimInPlace(async);
	Poco::toLowerInPlace(async);
	if (async == "false") HAS_ASYNC = false;

	// If I wasn't supplied a filename, try and find a logs folder
	if (file.empty()) {
		file = "%LOCAL%/logs/";
	}
	if (!file.empty()) {
		Poco::Path					   path(ds::Environment::expand(file));
		const Poco::Timestamp::TimeVal t = Poco::Timestamp().epochMicroseconds();
		static const std::string	   DATE_FORMAT("%Y-%m-%d");
		std::string					   fn;
		fn.append(settings.getString("platform:guid"));
		ds::replace(fn, "?", "");
		ds::replace(fn, "<", "");
		ds::replace(fn, ">", "");
		ds::replace(fn, ":", "");
		ds::replace(fn, "\"", "");
		ds::replace(fn, "|", "");
		ds::replace(fn, "*", "");
		fn.append("-");
		// If an actual file name was supplied, then do something to separate the date stamp
		// XXX -- not currently supported, assume the default log name
		// if (!file.empty() && !ends_in_separator(file)) file.append(" ");
		fn.append(Poco::DateTimeFormatter::format(Poco::LocalDateTime(), DATE_FORMAT));
		fn.append(".log.txt");
		path.append(fn);
		LOG_FILE = path.toString();

		std::cout << "Logging to file " << LOG_FILE << std::endl;
		// Verify the directory exists
		try {
			path.makeAbsolute();
			path.makeParent();
			Poco::File f(path);
			f.createDirectories();
			if (!f.exists())
				std::cout << "WARNING:  Log directory does not exist.  No log will be created." << std::endl
						  << "\t" << path.toString() << std::endl;
		} catch (std::exception& e) {
			std::cout << "Exception setting up logger directory: " << e.what() << std::endl;
		}
	}

	// Inform user of what modules are active (and available)
	if (MODULE_MAP) {
		for (auto it = MODULE_MAP->begin(), end = MODULE_MAP->end(); it != end; ++it) {
			std::cout << "Logger module " << it->first << " (" << it->second << ")";
			if (HAS_MODULE & (ds::BitMask(it->first))) std::cout << " is ON";
			std::cout << std::endl;
		}
		std::cout << "logger level is " << level << std::endl;
	}
}

ds::BitMask Logger::newModule(const std::string& name) {
	static std::map<int, std::string> MAP;
	static int						  NEXT_DS = 0;
	const ds::BitMask				  ans	  = ds::BitMask(NEXT_DS++);
	if (!MODULE_MAP) MODULE_MAP = &MAP;
	MAP[ans.getFirstIndex()] = name;
	return ans;
}

bool Logger::hasLevel(const int level) {
	if (level == LOG_STARTUP) return true;
	if (level < 0 || level >= LEVEL_SIZE) return false;
	return HAS_LEVEL[level];
}

bool ds::Logger::hasVerboseLevel(const int verboseLevel) {
	return verboseLevel <= VERBOSE_LEVEL;
}

void ds::Logger::setVerboseLevel(const int newVerboseLevel) {
	VERBOSE_LEVEL = newVerboseLevel;
	ds::getLogger().log(LOG_INFO, "Log verbosity is now " + std::to_string(VERBOSE_LEVEL));
}

const int ds::Logger::getVerboseLevel() {
	return VERBOSE_LEVEL;
}

void ds::Logger::incrementVerboseLevel() {
	VERBOSE_LEVEL++;
	if (VERBOSE_LEVEL > 9) VERBOSE_LEVEL = 0;
	ds::getLogger().log(LOG_INFO, "Log verbosity is now " + std::to_string(VERBOSE_LEVEL));
}

void ds::Logger::decrementVerboseLevel() {
	VERBOSE_LEVEL--;
	if (VERBOSE_LEVEL < 0) VERBOSE_LEVEL = 9;
	ds::getLogger().log(LOG_INFO, "Log verbosity is now " + std::to_string(VERBOSE_LEVEL));
}

bool Logger::hasModule(const ds::BitMask& m) {
	return HAS_MODULE.has(m);
}

void Logger::toggleModule(const ds::BitMask& module, const bool on) {
	if (on)
		HAS_MODULE |= module;
	else
		HAS_MODULE &= (~module);
}

/* DS::LOGGER
 ******************************************************************/
Logger::Logger() {
	if (HAS_ASYNC) {
		mThread.start(mLoop);
		// We use the HAS_ASYNC flag to determine if we're running,
		// so make sure it's accurate
		if (!mThread.isRunning()) HAS_ASYNC = false;
	}
}

Logger::~Logger() {
	shutDown();
}

void Logger::log(const int level, const std::string& str) {
	mLoop.log(level, str);
}

void ds::Logger::log(const int level, const std::wstring& str) {
	log(level, ds::utf8_from_wstr(str));
}

void Logger::blockUntilReady() {
	// Only matters if I'm running async
	if (!HAS_ASYNC) return;

	mLoop.log(LOG_LEVEL_BLOCK_CODE, "");
	BLOCK_SEM.wait();
}

void Logger::shutDown() {
	if (!mThread.isRunning()) return;

	{
		Poco::Mutex::ScopedLock l(mLoop.mMutex);
		mLoop.mAbort = true;
		mLoop.mCondition.signal();
	}

	try {
		mThread.join();
	} catch (std::exception&) {}
}

/* DS::LOGGER::LOOP
 ******************************************************************/
Logger::Loop::Loop()
  : mAbort(false) {
	mInput.reserve(128);
}

void Logger::Loop::log(const int level, const std::string& str) {
	Poco::Mutex::ScopedLock l(mMutex);
	try {
		mInput.push_back(entry());
		entry& e = mInput.back();
		e.mMsg	 = str;
		e.mLevel = level;
		e.mTime	 = Poco::LocalDateTime().timestamp().epochMicroseconds();
	} catch (std::exception&) {
		return;
	}
	if (HAS_ASYNC) {
		mCondition.signal();
	} else {
		consume(mInput);
	}
}

void ds::Logger::Loop::log(const int level, const std::wstring& str) {
	log(level, ds::utf8_from_wstr(str));
}

void Logger::Loop::run() {
	std::vector<entry> ins;
	ins.reserve(128);

	while (true) {
		// Pop the inputs
		{
			Poco::Mutex::ScopedLock l(mMutex);
			mInput.swap(ins);
		}

		// Perform each input
		consume(ins);

		{
			Poco::Mutex::ScopedLock l(mMutex);
			if (mAbort) break;
		}

		// If more input came in during the time I've been
		// processing keep going, otherwise wait.
		mMutex.lock();
		if (!mAbort && mInput.size() < 1) mCondition.wait(mMutex);
		if (mAbort) break;
		mMutex.unlock();
	}
}

void Logger::Loop::consume(std::vector<entry>& ins) {
	for (int k = 0; k < ins.size(); k++) {
		const entry& e = ins[k];
		if (e.mLevel == LOG_LEVEL_BLOCK_CODE) BLOCK_SEM.set();
		if (e.mMsg.empty()) continue;

		mBuf.str("");
		// time stamp
		static const std::string DATE_FORMAT("%Y/%m/%d %H:%M:%s");
		mBuf << Poco::DateTimeFormatter::format(Poco::Timestamp(e.mTime), DATE_FORMAT);
		mBuf << " ";
		// level
		mBuf << level_name(e.mLevel);
		mBuf << " ";
		// message
		mBuf << e.mMsg;
		mBuf << std::endl;

		logToConsole(e, mBuf.str());
		logToFile(e, mBuf.str());

		if (e.mLevel == ds::Logger::LOG_FATAL) {
			std::string msg = mBuf.str();
			Poco::Thread::sleep(4 * 1000);
			std::terminate();
		}
	}
	ins.clear();
}

void Logger::Loop::logToConsole(const entry& e, const std::string& formattedMsg) {
	std::cout << formattedMsg;
}

void Logger::Loop::logToFile(const entry& e, const std::string& formattedMsg) {
	if (LOG_FILE.empty()) return;

	std::ofstream outFile;
	outFile.open(LOG_FILE.c_str(), std::ios_base::app);
	outFile << formattedMsg;
	outFile.close();
}

void ds::Logger::Loop::logToConsole(const entry& e, const std::wstring& formattedMsg) {
	logToConsole(e, ds::utf8_from_wstr(formattedMsg));
}

void ds::Logger::Loop::logToFile(const entry& e, const std::wstring& formattedMsg) {
	logToFile(e, ds::utf8_from_wstr(formattedMsg));
}

/* DS::LOGGER singleton
 ******************************************************************/
namespace {
Poco::Mutex LOGGER_MUTEX;
}

extern Logger& ds::getLogger() {
	// The logger is in a multithreaded environment, so
	// control access to the static construction.
	Poco::Mutex::ScopedLock l(LOGGER_MUTEX);
	static Logger			LOGGER;
	return LOGGER;
}
