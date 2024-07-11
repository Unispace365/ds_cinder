#pragma once

#include <string>

#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/network/https_client.h>

namespace waffles {

/**
 * \class waffles::FileUploadRequest
 *			 Send a file to a cms
 */
class FileUploadRequest : public Poco::Runnable {
  public:
	FileUploadRequest(ds::ui::SpriteEngine& g);

	virtual void run();
	void		 setInput(const std::string& cmsLocation, const std::string& authHash, ci::Surface theSurface,
						  const int requestId, const std::string& saveName, const std::string& parentUid);

	std::vector<std::string> getHeaders();
	void createRecord();
	void getUploadUrl();
	void uploadToS3();
	void setMediaOnRecord();

	bool		mError;
	std::string mErrorMessage;
	int			mRequestId;

  private:
	ds::ui::SpriteEngine& mEngine;

	std::string mCmsLocation;
	std::string mAuthHash;
	std::string mSaveName;
	std::string mFullFilePath;
	std::string mParentUid;
	std::string mRecordUid;

	std::string mUploadUrl;
	std::string mDownloadUrl;

	ci::Surface mSurface;
	std::string mOutput;

	long		mHttpStatus;

	ds::net::HttpsRequest mCreateRecordRequest;
	ds::net::HttpsRequest mGetUploadUrlRequest;
	ds::net::HttpsRequest mUploadRequest;
	ds::net::HttpsRequest mAddUploadToRecordRequest;
};


} // namespace waffles
