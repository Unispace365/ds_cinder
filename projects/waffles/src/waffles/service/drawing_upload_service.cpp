#include "stdafx.h"

#include "drawing_upload_service.h"

#include <fstream>

#include <Poco/DateTimeFormatter.h>
#include <Poco/LocalDateTime.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

//#include "app/helpers.h"
#include "waffles/waffles_events.h"
//using namespace downstream;

namespace waffles {

DrawingUploadService::DrawingUploadService(ds::ui::SpriteEngine& g)
	: mEngine(g)
	, mEventClient(g)
	, mUploadRequests(g, [&g] { return new FileUploadRequest(g); })
	, mSaveRequests(g, [] { return new FileSaveRequest(); }) {

	mUploadRequests.setReplyHandler([this](FileUploadRequest& ur) {
		mEngine.getNotifier().notify(DrawingSaveComplete(ur.mRequestId, ur.mError, ur.mErrorMessage));
	});

	mSaveRequests.setReplyHandler([this](FileSaveRequest& ur) {
		mEngine.getNotifier().notify(DrawingSaveComplete(ur.mRequestId, ur.mError, ur.mErrorMessage));
	});

	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) {
		auto helper = WafflesHelperFactory::getDefault();
		ci::Surface theSurface = e.mSurface;
		int			requestId  = e.mRequestId;

		bool allowCmsStuff = mEngine.getAppSettings().getBool("cms_files:allow", 0, true);
		if (allowCmsStuff) {

			auto parentUid = helper->getAnnotationFolder().getPropertyString("uid");
			if (parentUid.empty()) {
				DS_LOG_WARNING("Trying to save drawing with no Annotation Folder. Aborting");
				return;
			};

			std::string cmsUrl = mEngine.getCmsURL();

			std::string authHash = mEngine.mContent.getChildByName("server").getPropertyString("auth");
			if (authHash.empty()) {
				authHash = mEngine.getAppSettings().getString("cms:auth_hash", 0, "ff779ee219d7be0549c971d6ba2311d5");
			}
			if (cmsUrl.empty()) {
				DS_LOG_WARNING("Set a cms:url in app settings to be able to save whiteboards!");
				return;
			}

			cmsUrl.append("/editing/record/create");

			auto dateName	= mEngine.getAppSettings().getString("drawings:save_name", 0, "Saved Drawing ");
			auto dateFormat = mEngine.getAppSettings().getString("drawings:date_time_format", 0, "%B %e, %Y %h:%M %a");

			Poco::LocalDateTime ldt;
			dateName.append(Poco::DateTimeFormatter().format(ldt, dateFormat));

			mUploadRequests.start(
				[this, theSurface, authHash, requestId, cmsUrl, dateName, parentUid](FileUploadRequest& ur) {
					ur.setInput(cmsUrl, authHash, theSurface, requestId, dateName, parentUid);
				});
		} else {
			std::string localPath =
				mEngine.getAppSettings().getString("drawings:drawings:can_save_as", 0, "%DOCUMENTS%/ds_screenshots/");
			bool isFullPath = false;
			if (!e.mLocalPath.empty()) {
				localPath  = e.mLocalPath;
				isFullPath = true;
			}
			mSaveRequests.start([this, theSurface, isFullPath, localPath, requestId](FileSaveRequest& ur) {
				ur.setInput(localPath, isFullPath, theSurface, requestId);
			});
		}
	});
}

} // namespace waffles
