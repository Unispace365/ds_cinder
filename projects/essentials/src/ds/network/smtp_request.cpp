#include "smtp_request.h"

#include "ds/network/curl/curl.h"

#include <ds/debug/logger.h>

std::vector<std::string> payload_textS;

struct upload_status {
	int lines_read;
};

namespace ds {
namespace net {
SMTPRequest::SMTPRequest(ds::ui::SpriteEngine& eng)
	: mRequests(eng)
{
	mRequests.setReplyHandler([this](SMTPRequest::IndividualRequest& q){onRequestComplete(q); });
}

void SMTPRequest::makePostRequest(const std::string serverUrl, const std::vector<std::string> userInfo, std::vector<std::string> sInput, std::string userpwd)
{
	if (serverUrl.empty()){
		DS_LOG_WARNING("Couldn't make a post request in SMTPRequest because the url is empty");
		return;
	}
	if (userInfo.size() < 2)
	{
		DS_LOG_WARNING("Couldn't make a post request in SMTPRequest because of lacking the basic email addresss");
		return;
	}


	mRequests.start([this, serverUrl, userInfo, sInput, userpwd](IndividualRequest& q){
		q.mSMTPServer = serverUrl;
		q.mFrom = userInfo[0];
		q.mTo = userInfo[1];
		payload_textS = sInput;
		q.mUserPwd = userpwd;
		for (auto i = 2; i < userInfo.size(); i++)
		{
			q.mCC.push_back(userInfo[i]);
		}
	});
}

void SMTPRequest::onRequestComplete(IndividualRequest& q){
	if(mReplyFunction){
		if(q.mError){
			mReplyFunction(true, q.mErrorMessage);
		} else {
			mReplyFunction(false, q.mOutput);
		}
	}
}


SMTPRequest::IndividualRequest::IndividualRequest()
	: mError(false)
{}

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;
	const char *data;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1) || upload_ctx->lines_read >= payload_textS.size()) {
		return 0;
	}
	data = payload_textS[upload_ctx->lines_read].c_str();

	if (data) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_ctx->lines_read++;

		return len;
	}

	return 0;
}

void SMTPRequest::IndividualRequest::run(){
	mError = false;
	mErrorMessage = "";
	mOutput = "";

	CURL *curl = curl_easy_init();

	struct upload_status upload_ctx;
	upload_ctx.lines_read = 0;

	if(curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, mSMTPServer.c_str());
			curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mFrom.c_str());
			struct curl_slist *recipients = NULL;
			recipients = curl_slist_append(recipients, mTo.c_str());
			if (!mCC.empty())
			{
				for (auto it : mCC)
					recipients = curl_slist_append(recipients, it.c_str());
			}
			curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
			curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			if (mUserPwd != "")
				curl_easy_setopt(curl, CURLOPT_USERPWD, mUserPwd.c_str());
			res = curl_easy_perform(curl);

			if(res != CURLE_OK){
				mError = true;
				mErrorMessage = curl_easy_strerror(res);
				DS_LOG_WARNING(mErrorMessage);
			} else {
				// something?
			}
		curl_easy_cleanup(curl);
	}
}

} // namespace net
} // namespace ds
