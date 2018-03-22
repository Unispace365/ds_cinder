#include "stdafx.h"

#include "https_client.h"

#define CURL_STATICLIB
#include "ds/network/curl/curl.h"

#include <ds/debug/logger.h>

namespace ds {
namespace net {
HttpsRequest::HttpsRequest(ds::ui::SpriteEngine& eng)
	: mRequests(eng)
	, mVerbose(false)
{
	mRequests.setReplyHandler([this](HttpsRequest::IndividualRequest& q){onRequestComplete(q); });
}


void HttpsRequest::makeGetRequest(const std::string& url, const bool peerVerify, const bool hostVerify, const bool isDownloadMedia, const std::string& downloadfile){
	if(url.empty()){
		DS_LOG_WARNING("Couldn't make a get request in HttpsRequest because the url is empty");
		return;
	}

	DS_LOG_VERBOSE(1, "HttpsRequest::makeGetRequest url=" << url << " peer=" << peerVerify << " host=" << hostVerify << " isDownload=" << isDownloadMedia << " downloadFile=" << downloadfile);

	mRequests.start([this, url, peerVerify, hostVerify, isDownloadMedia, downloadfile](IndividualRequest& q){
		q.mInput = url;
		q.mVerifyHost = hostVerify;
		q.mVerifyPeers = peerVerify;
		q.mIsGet = true;
		q.mVerboseOutput = mVerbose; 
		q.mIsDownloadMedia = isDownloadMedia;
		q.mDownloadFile = downloadfile;
	});
}


void HttpsRequest::makePostRequest(const std::string& url, const std::string& postData, const bool peerVerify /*= true*/, const bool hostVerify /*= true*/, const std::string& customRequest, std::vector<std::string> headers, const bool isDownloadMedia, const std::string& downloadfile){
	if(url.empty()){
		DS_LOG_WARNING("Couldn't make a post request in HttpsRequest because the url is empty");
		return;
	}

	DS_LOG_VERBOSE(1, "HttpsRequest::makePostRequest url=" << url << " postData=" << postData <<  " peer=" << peerVerify << " host=" << hostVerify << " isDownload=" << isDownloadMedia << " downloadFile=" << downloadfile);

	mRequests.start([this, url, postData, peerVerify, hostVerify, customRequest, headers, isDownloadMedia, downloadfile](IndividualRequest& q){
		q.mInput = url;
		q.mPostData = postData;
		q.mVerifyHost = hostVerify;
		q.mVerifyPeers = peerVerify;
		q.mIsGet = false; 
		q.mCustomRequest = customRequest; 
		q.mHeaders = headers;
		q.mVerboseOutput = mVerbose;
		q.mIsDownloadMedia = isDownloadMedia;
		q.mDownloadFile = downloadfile; });

}


void HttpsRequest::setVerboseOutput(const bool verbose){
	mVerbose = verbose;
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
	, mVerboseOutput(false)
	, mIsDownloadMedia(false)
	, mDownloadFile("")
{}

void HttpsRequest::IndividualRequest::setInput(std::string url){
	mInput = url;
}

namespace {
size_t this_write_callback(char *contents, size_t size, size_t nmemb, void *userdata){
	size_t realsize = size * nmemb;
	((std::string*)userdata)->append((char*)contents, realsize);
	return realsize;
}

size_t this_write_file_callback(char *contents, size_t size, size_t nmemb, void *userdata){
	FILE* stream = (FILE*)userdata;
	if (!stream)
	{
		printf("!!! No stream\n");
		return 0;
	}

	size_t written = fwrite((FILE*)contents, size, nmemb, stream);
	return written;
}
}

void HttpsRequest::IndividualRequest::run(){
	mError = false;
	mErrorMessage = "";
	mOutput = "";
	if (mIsDownloadMedia)
	{
		mFp = fopen(mDownloadFile.c_str(), "wb");
		if (!mFp)
		{
			printf("!!! Failed to create file on the disk\n");
		}
	}
	mHttpStatus = 400;

	if(mInput.empty()){
		mError = true;
		mErrorMessage = "No input specified";
		return;
	}
	CURL *curl = curl_easy_init();
	if(curl) {

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

		if(mVerboseOutput){
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curl_easy_setopt(curl, CURLOPT_URL, mInput.c_str());
		if (!mIsDownloadMedia)
		{
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this_write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mOutput);
		}
		else
		{
			curl_easy_setopt(curl, CURLoption::CURLOPT_FOLLOWLOCATION);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this_write_file_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, mFp);
		}


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

		struct curl_slist *headers = NULL;
		CURLcode res;
		if(!mIsGet){
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

		DS_LOG_VERBOSE(1, "Curl request completed with result=" << res << " with httpsStatus=" << mHttpStatus << " for url=" << mInput);

		if(headers){
			curl_slist_free_all(headers);
		}
		curl_easy_cleanup(curl);
		if (mIsDownloadMedia)
		{
			fclose(mFp);
		}
	}
}

} // namespace net
} // namespace ds
