#include "ds/network/http_client.h"

#include <iostream>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/thread/work_manager.h"
#include "ds/util/memory_ds.h"
#include "ds/util/string_util.h"

using namespace std;

// FOR TESTING HTTP:
// Use http://requestb.in

namespace ds {

namespace {
static const int		  HTTP_GET_OPT = (1<<0);
static const int		  HTTP_POST_OPT = (1<<1);

static const string		EMPTY_SZ("");

const ds::BitMask		  HTTP_LOG = ds::Logger::newModule("http");
}

/* HTTP-CLIENT static
 ******************************************************************/
bool HttpClient::httpGetAndReply(const std::wstring& url, ds::HttpReply* ans)
{
	return httpAndReply(HTTP_GET_OPT, url, EMPTY_SZ, nullptr, ans);
}

bool HttpClient::httpPostAndReply(const std::wstring& url, const std::string& body, ds::HttpReply* ans)
{
	return httpAndReply(HTTP_POST_OPT, url, body, nullptr, ans);
}

bool HttpClient::httpPostAndReply(const std::wstring& url, const std::function<void(Poco::Net::HTMLForm&)>& postFn, ds::HttpReply* ans)
{
	return httpAndReply(HTTP_POST_OPT, url, EMPTY_SZ, postFn, ans);
}

/* HTTP-CLIENT
 ******************************************************************/
HttpClient::HttpClient(ui::SpriteEngine& e, const std::function<void(const HttpReply&)>& h)
	: inherited(e)
	, mCache(this)
	, mResultHandler(h)
{
}

void HttpClient::setResultHandler(const std::function<void(const HttpReply&)>& h)
{
	mResultHandler = h;
}

bool HttpClient::httpGet(const std::wstring& url)
{
	return sendHttp(HTTP_GET_OPT, url, EMPTY_SZ, nullptr);
}

bool HttpClient::httpPost(const std::wstring& url, const std::string& body)
{
	return sendHttp(HTTP_POST_OPT, url, body, nullptr);
}

bool HttpClient::httpPost(const std::wstring& url, const std::function<void(Poco::Net::HTMLForm&)>& postFn)
{
	return sendHttp(HTTP_POST_OPT, url, EMPTY_SZ, postFn);
}

void HttpClient::handleResult(std::unique_ptr<WorkRequest>& wr)
{
	std::unique_ptr<Request>		r(ds::unique_dynamic_cast<Request, WorkRequest>(wr));
	if (!r) return;

	if (mResultHandler) mResultHandler(r->mReply);
	// Recycle the request.
	mCache.push(r);
}

bool HttpClient::sendHttp(const int opt, const std::wstring& url, const std::string& body,
                          const std::function<void(Poco::Net::HTMLForm&)>& postFn)
{
	if (url.empty()) {
		DS_DBG_CODE(std::cout << "ERROR ds::HttpClient() empty url" << std::endl);
		return false;
	}

	std::unique_ptr<Request>		r(std::move(mCache.next()));
	if (!r) return false;

	r->mOpt = opt;
	r->mUrl = url;
	r->mBody = body;
	r->mPostFn = postFn;
	r->mReply.clear();
	return mManager.sendRequest(ds::unique_dynamic_cast<WorkRequest, Request>(r));
}

bool HttpClient::httpAndReply(const int opt, const std::wstring& url, const std::string& body,
                              const std::function<void(Poco::Net::HTMLForm&)>& postFn,
                              ds::HttpReply* ans)
{
	Request				r(nullptr);
	r.mOpt = opt;
	r.mUrl = url;
	r.mBody = body;
	r.mPostFn = postFn;
	r.run();
	if (ans) (*ans) = r.mReply;
	return true;
}

/* HTTP-CLIENT::HTTP-REPLY
 ******************************************************************/
ds::HttpReply::HttpReply()
{
	clear();
}

void ds::HttpReply::clear()
{
	mMsg.clear();
	mStatus = REPLY_UNKNOWN_ERROR;
}

/* HTTP-CLIENT::REQUEST
 ******************************************************************/
HttpClient::Request::Request(const void* clientId)
	: WorkRequest(clientId)
	, mOpt(HTTP_GET_OPT)
{
}

void HttpClient::Request::run() {
	mReply.clear();
	const string						url8 = ds::utf8_from_wstr(mUrl);
	if (url8.empty()) return;

	try {
		Poco::URI						uri(url8);
		Poco::Net::HTTPClientSession	s;
		// I think POCO is a little tricky -- if the URL does NOT have a server part, then this will automatically
		// URL encode the string.  If it does, it will leave it alone.  Groan!
		std::string						path(uri.getPathAndQuery());
		if (path.empty()) path = "/";
		// This seems insane, but some requests will complain unless we explicitly set the host name.
		// What's going on -- seen in the google weather API -- is that the SocketAddress translates the
		// domain name into an IP address, and then running the request redirects you back to the domain name.
		// So always set the host and port this way, rather then going through a SocketAddress on the constructor.
		s.setHost(uri.getHost());
		s.setPort(uri.getPort());

		if ((mOpt&HTTP_POST_OPT) != 0) {
			Poco::Net::HTTPRequest		request(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
			Poco::Net::HTMLForm		    form(request);
			form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
			if (!mBody.empty()) {
				request.setKeepAlive(true);
				// XXX Obviously we need to provide more parameters to use this properly.
				// Clients should use the postFn... probably should obsolete this, or make it URL-encoded only.
				Poco::Net::StringPartSource*  ps = new Poco::Net::StringPartSource(mBody, "binary/octet-stream", "unknown_file");
				if (ps) form.addPart("file", ps);
			} else if (mPostFn != nullptr) {
				request.setKeepAlive(true);
				mPostFn(form);
			}
			form.prepareSubmit(request);

			std::ostream&   ostr = s.sendRequest(request); // << mBody;
			form.write(ostr);
		} else {
			Poco::Net::HTTPRequest		request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
			s.sendRequest(request);
		}

		Poco::Net::HTTPResponse			response;
		std::istream&					rs = s.receiveResponse(response);
		if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
			std::string					str;
			Poco::StreamCopier::copyToString(rs, str);
			if (!str.empty()) mReply.mMsg = ds::wstr_from_utf8(str);
			mReply.mStatus = ds::HttpReply::REPLY_OK;
#ifdef _DEBUG
//			wcout << "DBG HttpClient response OK msg=" << mReply.mMsg << endl;
#endif
		} else {
#ifdef _DEBUG
			std::cout << "DBG Http request failed on " << uri.toString() << " response=" << response.getStatus() << " reason=" << response.getReason() << std::endl;
			std::string					str;
			Poco::StreamCopier::copyToString(rs, str);
			if (!str.empty()) std::cout << "response=" << str << std::endl;
#endif
		}
	} catch (Poco::Net::ConnectionRefusedException&) {
		cout << "HttpClient connection refused" << endl;
		mReply.mStatus = ds::HttpReply::REPLY_CONNECTION_ERROR;
	} catch (std::exception& ex) {
		cout << "HttpClient exception=" << ex.what() << endl;
	}
}

} // namespace ds