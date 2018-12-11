#ifndef _VIDEO_CONVERTER_APP_APPEVENTS_H_
#define _VIDEO_CONVERTER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

struct RequestConvertFiles : public ds::RegisteredEvent<RequestConvertFiles>{
	RequestConvertFiles(std::vector<std::string> paths) : mPaths(paths) {}
	std::vector<std::string> mPaths;
};

struct PlayVideoRequest : public ds::RegisteredEvent<PlayVideoRequest> {
	PlayVideoRequest(const std::string thePath) : mPath(thePath) {}
	std::string mPath;
};

} // !namespace downstream

#endif // !_VIDEO_CONVERTER_APP_APPEVENTS_H_
