#include "stdafx.h"

#include "file_save_request.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <sstream>

#include <cinder/ImageIo.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"


namespace waffles {

FileSaveRequest::FileSaveRequest() {
}

void FileSaveRequest::run() {

	mError = true;

	try {
		Poco::Path				 p(mFileLocation);
		Poco::Timestamp::TimeVal t = Poco::Timestamp().epochMicroseconds();
		std::stringstream		 filepath;
		filepath << "ds_cinder.screenshot." << t << ".png";
		p.append(filepath.str());
		std::string fullFilePath = Poco::Path::expand(p.toString());

		/// This is if someone clicked "save as..." when saving a drawing
		if (mIsFullFilePath) fullFilePath = mFileLocation;

		ci::writeImage(fullFilePath, mSurface);

		mSurface = ci::Surface();
		mError	 = false;
	} catch (std::exception& e) {
		DS_LOG_WARNING("Error saving a local drawing: " << e.what());
	}
}

void FileSaveRequest::setInput(const std::string& fileLocation, const bool isFullPath, ci::Surface surf,
							   const int requestId,const std::string& eventChannel) {
	mFileLocation	= fileLocation;
	mSurface		= surf;
	mRequestId		= requestId;
	mError			= false;
	mIsFullFilePath = isFullPath;
	mEventChannel = eventChannel;
}

} // namespace waffles
