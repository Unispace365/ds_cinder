#pragma once
#ifndef DS_NETWORK_HTTPCLIENT_H_
#define DS_NETWORK_HTTPCLIENT_H_

#include "ds/thread/work_client.h"
#include "ds/thread/work_request_list.h"
#include <functional>
#include <string>

namespace Poco { namespace Net {
	class HTMLForm;
	class HTTPRequest;
}} // namespace Poco::Net

namespace ds {

/* Generic HTTP reply info
 ******************************************************************/
class HttpReply {
  public:
	static const int REPLY_OK				= 0;
	static const int REPLY_UNKNOWN_ERROR	= -1;
	static const int REPLY_CONNECTION_ERROR = -2;

  public:
	std::wstring mMsg;
	int			 mStatus;

	HttpReply();

	void clear();
};

/* HTTP-CLIENT
 * Perform an HTTP GET operation asynchronously.  Note that
 * HTTPS is not supported -- Use HttpsClient for that.
 * This uses Poco to make requests. Recommend using HttpsClient (or use curl directly) instead
 ******************************************************************/
class HttpClient : public ds::WorkClient {
  public:
	/// If the URL contains a query part it needs to be URL encoded, that won't happen automatically.
	static bool httpGetAndReply(const std::wstring& url, ds::HttpReply*);
	static bool httpPostAndReply(const std::wstring& url, const std::string& body, ds::HttpReply*);
	/// Do a post with complete control over what goes in the form. Poco supplies utilities for
	/// adding strings and files. An example of posting a file into the form would be this:
	/// const std::string  filename("c:\\tempfile.png")'
	/// auto postFn = [filename](Poco::Net::HTMLForm& f) {
	///    Poco::Net::FilePartSource*  ps = new Poco::Net::FilePartSource(filename, "temp.png", "binary/octet-stream");
	///    if (ps) f.addPart("file", ps);
	/// };
	static bool httpPostAndReply(const std::wstring& url, const std::function<void(Poco::Net::HTMLForm&)>&,
								 ds::HttpReply*);

  public:
	HttpClient(ui::SpriteEngine&, const std::function<void(const HttpReply&)>& = nullptr);

	void setResultHandler(const std::function<void(const HttpReply&)>&);

	bool httpGet(const std::wstring& url, const int timeout = 30);
	bool httpPost(const std::wstring& url, const std::string& body, const int timeout = 30);
	bool httpPost(const std::wstring& url, const std::function<void(Poco::Net::HTMLForm&)>& postFn,
				  const int timeout = 30);
	bool http(const std::string& verb, const std::string& url, const std::string& body,
			  const std::function<void(Poco::Net::HTTPRequest&)>& postFn, const int timeout = 30);

  protected:
	virtual void handleResult(std::unique_ptr<WorkRequest>&);

  private:
	typedef ds::WorkClient inherited;

	class Request : public ds::WorkRequest {
	  public:
		Request(const void* clientId);

		/// input
		int			 mOpt;
		std::string	 mVerb;
		std::wstring mUrl;
		std::string	 mBody;
		int			 mTimeout;
		/// Utility to write multipart form data in a post
		std::function<void(Poco::Net::HTMLForm&)> mPostFn;
		/// Utility to write to the message
		std::function<void(Poco::Net::HTTPRequest&)> mRequestFn;

		/// output
		ds::HttpReply mReply;

		virtual void run();
	};

	ds::WorkRequestList<Request>		  mCache;
	std::function<void(const HttpReply&)> mResultHandler;

  private:
	bool		sendHttp(const int opt, const std::string& verb, const std::wstring& url, const std::string& body,
						 const std::function<void(Poco::Net::HTMLForm&)>&	 postFn,
						 const std::function<void(Poco::Net::HTTPRequest&)>& requestFn = nullptr,
						 const int timeout = 30);
	static bool httpAndReply(const int opt, const std::wstring& url, const std::string& body,
							 const std::function<void(Poco::Net::HTMLForm&)>& postFn, ds::HttpReply*);
};

} // namespace ds

#endif // DS_NETWORK_HTTPCLIENT_H_
