#pragma once
#ifndef PRIVATE_CEF_WEB_CALLBACKS_H
#define PRIVATE_CEF_WEB_CALLBACKS_H

#include <functional>

namespace ds {
class Engine;

namespace web {

/**
* \class ds::web::WebCefCallbacks
* \brief A wrapper object 
*/
struct WebCefCallbacks  {
public:
	WebCefCallbacks(){};

	// Gets called when the browser sends new paint info, aka new buffers
	std::function<void(const void *, const int, const int)> mPaintCallback;
	// The loading state has changed (started or finished loading.
	std::function<void(const bool, const bool, const bool)> mLoadChangeCallback;
	// The title of the page has changed
	std::function<void(const std::wstring&)>				mTitleChangeCallback;



};

} // namespace web
} // namespace ds

#endif // PRIVATE_CEF_WEB_CALLBACKS_H