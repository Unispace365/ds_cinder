#pragma once
#ifndef DS_DEBUG_LOGGER_H_
#define DS_DEBUG_LOGGER_H_

// Unfortunately due to some weird include issue I need to make sure to
// include cinder/ChanTraits.h before something in presumably the C++ libs.
#include <cinder/Color.h>
#include <sstream>
#include <string>
#include <vector>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include <Poco/Timestamp.h>
#include "ds/util/bit_mask.h"

namespace ds {

namespace cfg {
class Settings;
} // namespace cfg

// Some common modules, for lack of a better place
extern const ds::BitMask	GENERAL_LOG;
extern const ds::BitMask	IMAGE_LOG;
extern const ds::BitMask	VIDEO_LOG;

/**
 * \class ds::Logger
 * \brief Standard logging behaviour.
 * \description A run-time configurable logger that supports clients
 * supplying log level (INFO, WARN etc) and application-defined modules.
 * The logger is initially configured from a settings file, and then at
 * runtime, modules can be turned on and off.  All logging ideally happens
 * through the DS_LOG_* convenience macros.
 */
class Logger
{
  public:
    /**
     * \brief Initialize the logger.  Settings files are in the standard ds::cfg::Settings format.  Here's what's available:
     *	"logger:level" string -- all,none,info,warning,error,fatal (can be specific levels, like "error,fatal") DEFAULT=none
     *  "logger:module" string -- all,none, or numbers (i.e. "0,1,2,3").  applications map the numbers to specific modules DEFAULT=all
     *  "logger:file" string -- filename (and location).  a date stamp is appended.  DEFAULT=../logs/
     *  "logger:async" text -- (true,false) If this is false, then logging is synchronous.  DEFAULT=true
     */
    static void						  setup(const ds::cfg::Settings&);

    /**
     * LEVELS
     */
    static const int				LOG_INFO = 0;
    static const int				LOG_WARNING = 1;
    static const int				LOG_ERROR = 2;
    static const int				LOG_FATAL = 3;
    // Verification that the given parameter is valid to log.
    static bool						  hasLevel(const int level);

    /**
     * MODULES
     */
    /**
     * \brief Create a new, globally unique module.  This is meant
     * to be run statically or in the main thread.  The module names and indexes will
     * be printed on app startup.  Typical usage might look like:
     * namespace {
     * const ds::BitMask	QUERY_MODULE = ds::Logger::newModule();
     * }
     */
    static ds::BitMask      newModule(const std::string& name);

    // Verification that the given parameter is valid to log.
    static bool             hasModule(const ds::BitMask&);

    // A run-time switch to toggle specific modules on and off.  This isn't
    // 100% safe but the consequences aren't exactly dire -- extra logging or
    // missing logging for a fraction of a second.
    static void             toggleModule(const ds::BitMask& module, const bool on);

  public:
    Logger();
    ~Logger();

    void                    log(const int level, const std::string&);

    // Block until all current inputs have finished writing
    void                    blockUntilReady();

    // called by the app to make sure I'm shut down.
    void                    shutDown();

  private:
    struct entry {
      Poco::Timestamp::TimeVal
                            mTime;
      int                   mLevel;
      std::string           mMsg;
    };

  private:
    class Loop : public Poco::Runnable {
      public:
        Poco::Mutex         mMutex;
        Poco::Condition     mCondition;
        bool                mAbort;
        std::vector<entry>  mInput;

      public:
        Loop();

        void                log(const int level, const std::string&);

        virtual void        run();

      private:
        std::string         mFileName;
        std::stringstream   mBuf;

        void                consume(std::vector<entry>&);
        void                logToConsole(const entry&, const std::string& formattedMsg);
        void                logToFile(const entry&, const std::string& formattedMsg);
    };

  private:
    Loop                    mLoop;
    Poco::Thread            mThread;
};


// Singleton access
Logger&                     getLogger();

} // namespace ds

// example: DS_LOG(ds::Logger::LOG_INFO, "I have " << numberArg << " info items to report" << endl, ds::BitMask::newFilled());
#define DS_LOG(level, streamExp, module)	{ if (ds::Logger::hasLevel(level) && ds::Logger::hasModule(module)) { std::stringstream	buf;	buf << streamExp; 	ds::getLogger().log(level, buf.str()); } }

// Logging convenience
#define DS_LOG_INFO(streamExp)				DS_LOG(ds::Logger::LOG_INFO,	streamExp, ds::BitMask::newFilled())
#define DS_LOG_INFO_M(streamExp, module)	DS_LOG(ds::Logger::LOG_INFO,	streamExp, module)
#define DS_LOG_WARNING(streamExp)			DS_LOG(ds::Logger::LOG_WARNING, streamExp, ds::BitMask::newFilled())
#define DS_LOG_WARNING_M(streamExp, module)	DS_LOG(ds::Logger::LOG_WARNING, streamExp, module)
#define DS_LOG_ERROR(streamExp)				DS_LOG(ds::Logger::LOG_ERROR,	streamExp, ds::BitMask::newFilled())
#define DS_LOG_ERROR_M(streamExp, module)	DS_LOG(ds::Logger::LOG_ERROR,	streamExp, module)
#define DS_LOG_FATAL(streamExp)				DS_LOG(ds::Logger::LOG_FATAL,	streamExp, ds::BitMask::newFilled())
#define DS_LOG_FATAL_M(streamExp, module)	DS_LOG(ds::Logger::LOG_FATAL,	streamExp, module)

// Utility for logging a fatal error then ending the app, to maintain compatibility
// with the previous logger, which had this functionality.  Probably shouldn't
// be here in the logging stuff, but I don't think we have a place for things
// like this right now.
#define DS_FATAL_ERROR(streamExp)			{ std::stringstream	buf;	buf << streamExp; 	ds::getLogger().log(ds::Logger::LOG_FATAL, buf.str()); ds::getLogger().blockUntilReady(); Poco::Thread::sleep(4000); std::exit(-1); }

#endif // LOGGER_DS_H
