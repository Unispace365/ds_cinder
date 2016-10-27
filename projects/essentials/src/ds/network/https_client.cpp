#include "https_client.h"

#include "ds/network/curl/curl.h"

#include <ds/debug/logger.h>

namespace ds {
namespace net {
HttpsRequest::HttpsRequest(ds::ui::SpriteEngine& eng)
	: mRequests(eng)
{
	mRequests.setReplyHandler([this](HttpsRequest::IndividualRequest& q){onRequestComplete(q); });
}


void HttpsRequest::makeGetRequest(const std::string& url, const bool peerVerify, const bool hostVerify){
	if(url.empty()){
		DS_LOG_WARNING("Couldn't make a get request in HttpsRequest because the url is empty");
		return;
	}
	mRequests.start([this, url, peerVerify, hostVerify](IndividualRequest& q){
		q.mInput = url;
		q.mVerifyHost = hostVerify;
		q.mVerifyPeers = peerVerify;
		q.mIsGet = true; });
}


void HttpsRequest::makePostRequest(const std::string& url, const std::string& postData, const bool peerVerify /*= true*/, const bool hostVerify /*= true*/, const std::string& customRequest, std::vector<std::string> headers){
	if(url.empty()){
		DS_LOG_WARNING("Couldn't make a post request in HttpsRequest because the url is empty");
		return;
	}

	mRequests.start([this, url, postData, peerVerify, hostVerify, customRequest, headers](IndividualRequest& q){
		q.mInput = url;
		q.mPostData = postData;
		q.mVerifyHost = hostVerify;
		q.mVerifyPeers = peerVerify;
		q.mIsGet = false; 
		q.mCustomRequest = customRequest; 
		q.mHeaders = headers; });

}

void HttpsRequest::onRequestComplete(IndividualRequest& q){
	if(mReplyFunction){
		if(q.mError){
			mReplyFunction(true, q.mErrorMessage, q.mHttpStatus);
		} else {
			mReplyFunction(false, q.mOutput, q.mHttpStatus);
		}
	}
}


HttpsRequest::IndividualRequest::IndividualRequest()
	: mError(false)
	, mVerifyPeers(true)
	, mVerifyHost(true)
	, mIsGet(true)
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
	mHttpStatus = 400;

	if(mInput.empty()){
		mError = true;
		mErrorMessage = "No input specified";
		return;
	}
	CURL *curl = curl_easy_init();
	if(curl) {

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

		struct curl_slist *headers = NULL;
		if(mIsGet){
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
				DS_LOG_WARNING(mErrorMessage);
			} else {
				// success
			}

			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &mHttpStatus);
		} else {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, mInput.c_str());

			if(!mHeaders.empty()){
				for(auto it : mHeaders){
					headers = curl_slist_append(headers, it.c_str());
				}
				auto optSet = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			}

			/* Allows custom request types, like DELETE*/
			if(!mCustomRequest.empty()){
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, mCustomRequest.c_str());
			}

			if(!mPostData.empty()){
				/* Now specify the POST data */
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mPostData.c_str());
			}

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

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if(res != CURLE_OK){
				mError = true;
				mErrorMessage = curl_easy_strerror(res);
				DS_LOG_WARNING(mErrorMessage);
			} 
			
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &mHttpStatus);
		}

		if(headers){
			curl_slist_free_all(headers);
		}
		curl_easy_cleanup(curl);
	}
}

} // namespace net
} // namespace ds
