#pragma once
#ifndef ESSENTIALS_DS_NETWORLD_SMTP_REQUEST
#define ESSENTIALS_DS_NETWORLD_SMTP_REQUEST

#include <string>
#include <functional>
#include <vector>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/thread/parallel_runnable.h>
#include <Poco/Runnable.h>

namespace ds {
namespace net {
/**
* \class SMTPRequest
* Make SMTP requests to send an email. This uses Curl behind the scenes
*/

class SMTPRequest {
public:
	SMTPRequest(ds::ui::SpriteEngine& eng);


	/// The serverUrl is the SMTP SERVER url. format is smtp://mail.smtp.com:25 
	/// userInfo, first is email address for sender, second is email address for receiver, and following is email address for cc. format is "<example@email.com>"
	/// sInput is the content for the email, include from,to,cc,subject,content body. details see SMTP sample
	/// userpwd is the username and password for SMTP account. format is "user:pwd"


	void					makePostRequest(const std::string serverUrl, const std::vector<std::string> userInfo, std::vector<std::string> sInput, std::string userpwd = "");

	/// If errored == true, then something went wrong and the reply will have the error message
	/// Otherwise it will be whatever was returned from the server
	void					setReplyFunction(std::function<void(const bool errored, const std::string& reply)> func){ mReplyFunction = func; }

private:
	class IndividualRequest : public Poco::Runnable {
	public:
		IndividualRequest();

		virtual void		run();

		bool				mError;
		std::string			mErrorMessage;

		std::string			mOutput;

		std::string			mFrom;
		std::string			mTo;
		std::vector<std::string>
							mCC;
		std::string			mUserPwd;
		std::string			mSMTPServer;
	};

	void									onRequestComplete(IndividualRequest&);
	ds::ParallelRunnable<IndividualRequest>	mRequests;
	std::function<void(const bool errored, const std::string&)>	mReplyFunction;
};
} // namespace net
} // namespace ds

#endif 
