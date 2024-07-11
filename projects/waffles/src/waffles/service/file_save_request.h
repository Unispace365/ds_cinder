#pragma once

#include <ds/app/event_client.h>
#include <ds/network/tcp_server.h>
#include <ds/thread/serial_runnable.h>
#include <map>
#include <string>

namespace waffles {

/**
 * \class waffles::FileSaveRequest
 *			 Save a file locally (and by file I mean image)
 */
class FileSaveRequest : public Poco::Runnable {
  public:
	FileSaveRequest();

	virtual void run();
	void setInput(const std::string& fileLocation, bool isFullFilePath, ci::Surface theSurface, const int requestId);

	bool		mError;
	std::string mErrorMessage;
	int			mRequestId;

  private:
	bool		mIsFullFilePath;
	std::string mFileLocation;
	ci::Surface mSurface;
	std::string mOutput;
	long		mHttpStatus;
};


} // namespace waffles
