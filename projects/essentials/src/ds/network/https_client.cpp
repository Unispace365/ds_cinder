#include "https_client.h"

#include "ds/network/curl/curl.h"

namespace ds {
namespace net {
HttpsRequest::HttpsRequest(ds::ui::SpriteEngine& eng)
	: mRequests(eng)
{
	mRequests.setReplyHandler([this](HttpsRequest::IndividualRequest& q){onRequestComplete(q); });
}


void HttpsRequest::makeGetRequest(const std::string& url, const bool peerVerify, const bool hostVerify){
	mRequests.start([this, url, peerVerify, hostVerify](IndividualRequest& q){ q.mInput = url; q.mVerifyHost = hostVerify; q.mVerifyPeers = peerVerify; });
}

void HttpsRequest::onRequestComplete(IndividualRequest& q){
	if(mReplyFunction){
		if(q.mError){
			mReplyFunction(true, q.mErrorMessage);
		} else {
			mReplyFunction(false, q.mOutput);
		}
	}
}


HttpsRequest::IndividualRequest::IndividualRequest()
	: mError(false)
	, mVerifyPeers(true)
	, mVerifyHost(true)
{}

void HttpsRequest::IndividualRequest::setInput(std::string url){
	mInput = url;
}

size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata){
	size_t realsize = size * nmemb;
	((std::string*)userdata)->append((char*)contents, size * nmemb);
	return realsize;
}
void HttpsRequest::IndividualRequest::run(){
	mError = false;
	mErrorMessage = "";
	mOutput = "";

	if(mInput.empty()){
		mError = true;
		mErrorMessage = "No input specified";
		return;
	}
	CURL *curl = curl_easy_init();
	if(curl) {
		CURLcode res;
		curl_easy_setopt(curl, CURLOPT_URL, mInput.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mOutput);
		/*
		* If you want to connect to a site who isn't using a certificate that is
		* signed by one of the certs in the CA bundle you have, you can skip the
		* verification of the server's certificate. This makes the connection
		* A LOT LESS SECURE.
		*
		* If you have a CA cert for the server stored someplace else than in the
		* default bundle, then the CURLOPT_CAPATH option might come handy for
		* you.
		*/
		if(!mVerifyPeers){
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		}

		/*
		* If the site you're connecting to uses a different host name that what
		* they have mentioned in their server certificate's commonName (or
		* subjectAltName) fields, libcurl will refuse to connect. You can skip
		* this check, but this will make the connection less secure.
		*/
		if(!mVerifyHost){
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		}

		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			mError = true;
			mErrorMessage = curl_easy_strerror(res);
		} else {
			// something?
		}
		curl_easy_cleanup(curl);
	}
}

} // namespace net
} // namespace ds
