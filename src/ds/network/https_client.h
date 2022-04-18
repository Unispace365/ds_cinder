#pragma once
#ifndef ESSENTIALS_DS_NETWORLD_HTTPS_CLIENT
#define ESSENTIALS_DS_NETWORLD_HTTPS_CLIENT

#include <string>
#include <functional>
#include <vector>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/thread/parallel_runnable.h>
#include <Poco/Runnable.h>

namespace ds {
namespace net {
/**
* \class HttpsRequest
* Make very simple https requests
* This uses Curl on the backend, whereas HttpRequest uses Poco (which doesn't support SSL)
*/

class HttpsRequest {

public:
	HttpsRequest(ds::ui::SpriteEngine& eng);

	/// The url is the full request url
	/// verifyPeers if false will use try to connect even if the certificate is self-signed (Much less secure)
	/// verifyHosts if false will use try to connect even if the certificate doesn't match the domain (Much less secure)
	void					makeGetRequest(const std::string& url, const bool verifyPeers = true, const bool verifyHosts = true, const bool isDownloadMedia = false, const std::string& downloadfile = "");
	void					makeGetRequest(const std::string& url, std::vector<std::string> headers, const bool verifyPeers = true, const bool verifyHosts = true, const bool isDownloadMedia = false, const std::string& downloadfile = "");
	/// Same as makeGetRequest, but runs syncronously
	void					makeSyncGetRequest(const std::string& url, const bool verifyPeers = true, const bool verifyHosts = true, const bool isDownloadMedia = false, const std::string& downloadfile = "");

	/// The url is the full request url
	/// The postData is something like name=jeeves&project=ds_cinder
	/// verifyPeers if false will use try to connect even if the certificate is self-signed (Much less secure)
	/// verifyHosts if false will use try to connect even if the certificate doesn't match the domain (Much less secure)
	void					makePostRequest(const std::string& url, const std::string& postData, const bool verifyPeers = true, const bool verifyHosts = true, const std::string& customrequest = "", std::vector<std::string> headers = std::vector<std::string>(), const bool isDownloadMedia = false, const std::string& downloadfile = "");
	
	/// Same as makePostRequest, but runs syncronously
	void					makeSyncPostRequest(const std::string& url, const std::string& postData, const bool verifyPeers = true, const bool verifyHosts = true, const std::string& customrequest = "", std::vector<std::string> headers = std::vector<std::string>(), const bool isDownloadMedia = false, const std::string& downloadfile = "");

	/// If errored == true, then something went wrong and the reply will have the error message
	/// Otherwise it will be whatever was returned from the server
	/// HTTP status is the normal return code (100 = continue, 200 = ok, 404 = not found, 500 = server error, etc)
	void					setReplyFunction(std::function<void(const bool errored, const std::string& reply, const long httpCode)> func){ mReplyFunction = func; }

	void					setVerboseOutput(const bool verbose);
	const std::string&		getLastRequestUrl() const { return mLastRequestUrl; }

private:
	class IndividualRequest : public Poco::Runnable {
	public:
		IndividualRequest();

		void				setInput(std::string url);

		virtual void		run();

		bool				mError;
		std::string			mErrorMessage;
		std::string			mOutput;
		std::FILE*			mFp;
		long				mHttpStatus;
		std::string			mInput;
		bool				mVerifyPeers;
		bool				mVerifyHost;
		bool				mIsGet;
		std::string			mPostData;
		std::string			mCustomRequest;
		std::vector<std::string>	mHeaders;
		bool				mVerboseOutput;
		bool				mIsDownloadMedia;
		std::string			mDownloadFile;
	};

	void									onRequestComplete(IndividualRequest&);
	bool									mVerbose;
	ds::ParallelRunnable<IndividualRequest>	mRequests;
	std::function<void(const bool errored, const std::string&, const long)>	mReplyFunction;
	std::string								mLastRequestUrl;
};
} // namespace net
} // namespace ds

#endif 
