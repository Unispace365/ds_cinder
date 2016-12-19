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
* \class ds::net::HttpsRequest
* Make very simple https requests
*/

class HttpsRequest {

public:
	HttpsRequest(ds::ui::SpriteEngine& eng);

	/// The url is the full request url
	/// verifyPeers if false will use try to connect even if the certificate is self-signed (Much less secure)
	/// verifyHosts if false will use try to connect even if the certificate doesn't match the domain (Much less secure)
	void					makeGetRequest(const std::string& url, const bool verifyPeers = true, const bool verifyHosts = true);

	/// The url is the full request url
	/// The postData is something like name=jeeves&project=ds_cinder
	/// verifyPeers if false will use try to connect even if the certificate is self-signed (Much less secure)
	/// verifyHosts if false will use try to connect even if the certificate doesn't match the domain (Much less secure)
	void					makePostRequest(const std::string& url, const std::string& postData, const bool verifyPeers = true, const bool verifyHosts = true, const std::string& customrequest = "", std::vector<std::string> headers = std::vector<std::string>());

	/// If errored == true, then something went wrong and the reply will have the error message
	/// Otherwise it will be whatever was returned from the server
	/// HTTP status is the normal return code (100 = continue, 200 = ok, 404 = not found, 500 = server error, etc)
	void					setReplyFunction(std::function<void(const bool errored, const std::string& reply, const long httpCode)> func){ mReplyFunction = func; }

	void					setVerboseOutput(const bool verbose);

private:
	class IndividualRequest : public Poco::Runnable {
	public:
		IndividualRequest();

		void				setInput(std::string url);

		virtual void		run();

		bool				mError;
		std::string			mErrorMessage;
		std::string			mOutput;
		long				mHttpStatus;
		std::string			mInput;
		bool				mVerifyPeers;
		bool				mVerifyHost;
		bool				mIsGet;
		std::string			mPostData;
		std::string			mCustomRequest;
		std::vector<std::string>	mHeaders;
		bool				mVerboseOutput;
	};

	void									onRequestComplete(IndividualRequest&);
	bool									mVerbose;
	ds::ParallelRunnable<IndividualRequest>	mRequests;
	std::function<void(const bool errored, const std::string&, const long)>	mReplyFunction;
};
} // namespace net
} // namespace ds

#endif 