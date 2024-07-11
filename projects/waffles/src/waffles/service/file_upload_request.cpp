#include "stdafx.h"

#include "file_upload_request.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <sstream>

#include <cinder/ImageIo.h>
#include <cinder/Json.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>

#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include <ds/network/curl/curl.h>

#include "app/app_defs.h"

namespace waffles {

FileUploadRequest::FileUploadRequest(ds::ui::SpriteEngine& g)
	: mEngine(g)
	, mCreateRecordRequest(g)
	, mGetUploadUrlRequest(g)
	, mUploadRequest(g)
	, mAddUploadToRecordRequest(g) {
}

size_t file_write_callback(char* contents, size_t size, size_t nmemb, void* userdata) {
	size_t realsize = size * nmemb;
	((std::string*)userdata)->append((char*)contents, size * nmemb);
	return realsize;
}

void FileUploadRequest::run() {
	if (mCmsLocation.empty()) {
		DS_LOG_WARNING("No CMS location specified in file upload request!");
		return;
	}

	Poco::Path				 p(ds::Environment::expand("%DOCUMENTS%"));
	Poco::Timestamp::TimeVal t = Poco::Timestamp().epochMicroseconds();
	std::stringstream		 filepath;
	filepath << "Annotation-" << t << ".png";
	mSaveName = filepath.str();
	p.append("ds_screenshots/").append(filepath.str());
	mFullFilePath = Poco::Path::expand(p.toString());

	ci::writeImage(mFullFilePath, mSurface);
	mSurface = ci::Surface();

	// Send create record request to active pinboard
	createRecord();
}

std::vector<std::string> FileUploadRequest::getHeaders() {
	std::string authHash = mEngine.mContent.getChildByName("server").getPropertyString("auth");
	if (authHash.empty()) {
		authHash = mEngine.getAppSettings().getString("cms:auth_hash", 0, "");
	}
	std::vector<std::string> headers;
	headers.emplace_back("Authorization: Bearer " + authHash);
	headers.emplace_back("Content-Type: application/json");
	headers.emplace_back("Accept: application/json");
	return headers;
}

void FileUploadRequest::createRecord() {
	// NOTE: Hardcoded slot_uids here! Should be replaced with an appsetting or something
	std::string thePayload =
		R"JSON( {"type": "JfDgLbj9vxT8","parent": {"variant": "RECORD","record": "%RECORD%","slot": "vRvbnAxGIDJ7"},"name": "%NAME%"} )JSON";

	ds::replace(thePayload, "%RECORD%", mParentUid);
	ds::replace(thePayload, "%NAME%", mSaveName);

	mCreateRecordRequest.setReplyFunction(
		[this, thePayload](const bool erroed, const std::string& reply, long httpCode) {
			if (httpCode == 200 && !erroed) {
				std::string toRecordUid;
				auto		replyJson = ci::JsonTree(reply);
				if (replyJson.hasChild("data") && replyJson.getChild("data").hasChild("id")) {
					mRecordUid = replyJson.getChild("data").getChild("id").getValue();
					getUploadUrl();
				} else {

					mError		  = true;
					mErrorMessage = reply;
					mHttpStatus	  = httpCode;
					DS_LOG_WARNING("Unable to find record UID for new pinboard record! Contents: " << reply);
				}
			} else {
				mError		  = true;
				mErrorMessage = reply;
				mHttpStatus	  = httpCode;
				DS_LOG_WARNING("Unable to create pinboard record! " << reply);
				DS_LOG_WARNING("Payload: " << thePayload);
			}
		});

	// auto endpoint = "https://app.tetrapak.bridge.downstream.com/editing/record/create";
	auto endpoint = mEngine.getCmsURL() + "/editing/record/create";
	mCreateRecordRequest.makeSyncPostRequest(endpoint, thePayload, false, false, "POST", getHeaders());
}

void FileUploadRequest::getUploadUrl() {
	std::string thePayload =
		R"JSON(
		{"filename": "%FILENAME%", "contentType": "image/png"}
		)JSON";

	ds::replace(thePayload, "%FILENAME%", mSaveName);

	mCreateRecordRequest.setReplyFunction([this](const bool erroed, const std::string& reply, long httpCode) {
		if (httpCode == 200 && !erroed) {
			auto replyJson = ci::JsonTree(reply);
			if (replyJson.hasChild("data") && replyJson.getChild("data").hasChild("upload") &&
				replyJson.getChild("data").hasChild("download")) {
				mUploadUrl	 = replyJson.getChild("data").getChild("upload").getValue();
				mDownloadUrl = replyJson.getChild("data").getChild("download").getValue();
				DS_LOG_INFO("Upload URL: " << mUploadUrl);
				DS_LOG_INFO("Download URL: " << mDownloadUrl);
				uploadToS3();
			} else {
				DS_LOG_WARNING("Unable to parse getTemporaryUploadUrls reply! Contents: " << reply);
				mError		  = true;
				mErrorMessage = reply;
				mHttpStatus	  = httpCode;
			}
		} else {
			DS_LOG_WARNING("Unable to parse getTemporaryUploadUrls reply! Contents: " << reply);
			mError		  = true;
			mErrorMessage = reply;
			mHttpStatus	  = httpCode;
		}
	});

	auto endpoint = mEngine.getCmsURL() + "/editing/value/media/getTemporaryUploadUrls";
	mCreateRecordRequest.makeSyncPostRequest(endpoint, thePayload, false, false, "POST", getHeaders());
}
namespace {
	size_t read_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
		FILE*	   readhere = (FILE*)userdata;
		curl_off_t nread;

		/* copy as much data as possible into the 'ptr' buffer, but no more than
		   'size' * 'nmemb' bytes! */
		size_t retcode = fread(ptr, size, nmemb, readhere);

		nread = (curl_off_t)retcode;

		fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
		return retcode;
	}
} // namespace

void FileUploadRequest::uploadToS3() {
	CURL* curl = curl_easy_init();
	if (curl) {

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mOutput);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		struct curl_slist* headers = NULL;
		headers					   = curl_slist_append(headers, "Content-Type: image/png");
		headers					   = curl_slist_append(headers, "X-Amz-Tagging: Bridge-Temp=1");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, mUploadUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		struct curl_httppost* post = NULL;
		struct curl_httppost* last = NULL;

		FILE* file = fopen(mFullFilePath.c_str(), "rb");
		fseek(file, 0L, SEEK_END);
		auto fz = ftell(file);
		fseek(file, 0L, SEEK_SET);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, (void*)file);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fz);

		// curl_formadd(&post, &last, CURLOPT_READDATA, "file", CURLFORM_FILE, mFullFilePath.c_str(), CURLFORM_END);
		// curl_easy_setopt(curl, CURLOPT_PUT, post);

		// For verbose output
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		CURLcode res = curl_easy_perform(curl);


		curl_formfree(post);

		if (res != CURLE_OK) {
			mErrorMessage = curl_easy_strerror(res);
			DS_LOG_WARNING("FileUploadRequest, Curl response: " << mErrorMessage);
			// used to set the error here, but curl can report an error but the request actually succeeeded
			// so now we use the http status code
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &mHttpStatus);

		DS_LOG_INFO("Curl finished, response code: " << mHttpStatus << std::endl << mOutput);
		if (mHttpStatus != 200) {
			mError		  = true;
			mErrorMessage = mOutput;
			DS_LOG_WARNING("Bad status while trying to upload file to S3! Status: " << mHttpStatus);
		}

		curl_slist_free_all(headers);

		mSurface = ci::Surface();
	}

	curl_easy_cleanup(curl);

	if (mHttpStatus == 200) {
		setMediaOnRecord();
	}
}

void FileUploadRequest::setMediaOnRecord() {
	std::string thePayload =
		R"JSON(
		{"field": "S2ex3AuxOtS3", "record": "%RECORDUID%", "url": "%DOWNLOADURL%", "filename": "%FILENAME%"}
		)JSON";

	ds::replace(thePayload, "%RECORDUID%", mRecordUid);
	ds::replace(thePayload, "%DOWNLOADURL%", mDownloadUrl);
	ds::replace(thePayload, "%FILENAME%", mSaveName);

	mAddUploadToRecordRequest.setReplyFunction([this](const bool erroed, const std::string& reply, long httpCode) {
		if (httpCode == 200 && !erroed) {
			return;
		} else {
			mError		  = true;
			mErrorMessage = reply;
			mHttpStatus	  = httpCode;
			DS_LOG_WARNING("Unable to parse getTemporaryUploadUrls reply! Contents: " << reply);
		}
	});

	auto endpoint = mEngine.getCmsURL() + "/editing/value/media/downloadFile";
	mAddUploadToRecordRequest.makeSyncPostRequest(endpoint, thePayload, false, false, "POST", getHeaders());
}

void FileUploadRequest::setInput(const std::string& cmsLocation, const std::string& authHash, ci::Surface surf,
								 const int requestId, const std::string& saveName, const std::string& parentUid) {
	mCmsLocation = cmsLocation;
	mAuthHash	 = authHash;
	mSurface	 = surf;
	mRequestId	 = requestId;
	mError		 = false;
	mSaveName	 = saveName;
	mParentUid	 = parentUid;
}

} // namespace waffles
