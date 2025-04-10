#include "stdafx.h"

#include <filesystem>

#include "ds/util/file_meta_data.h"
#include "framed_viewer_controller.h"
#include "waffles/common/ui_utils.h"
#include "waffles/viewers/framed_media_viewer.h"
#include "waffles/waffles_events.h"

#include "waffles/util/framed_waffles_helper.h"
#include "waffles/viewers/fullscreen_controller/framed_fullscreen_controller.h"
#include "waffles/viewers/fullscreen_controller/fullscreen_controller.h"

namespace waffles {
FramedViewerController::FramedViewerController(ds::ui::SpriteEngine& g, ci::vec2 size, std::string channel)
  : ViewerController(g, size, channel) {}

void FramedViewerController::initCreators() {
	ViewerController::initCreators();

	setCreator(VIEW_TYPE_TITLED_MEDIA_VIEWER,
			   [this](const ViewerCreationArgs args) -> std::tuple<BaseElement*, CreationError> {
				   auto helper			 = ds::model::ContentHelperFactory::getDefault<FramedWafflesHelper>();
				   auto mediaPropertyKey = helper->getMediaPropertyKey(args.mMediaRef);
				   auto theResource		 = args.mMediaRef.getPropertyResource(mediaPropertyKey);
				   auto isStream = helper->isValidStream(args.mMediaRef, ds::model::ContentHelper::WAFFLESCATEGORY);
				   auto isStreamSource =
					   helper->isValidStreamSource(args.mMediaRef, ds::model::ContentHelper::WAFFLESCATEGORY);

				   if (!isStream && !isStreamSource && args.mMediaRef.getPropertyString("type") != MEDIA_TYPE_CAPTURE &&
					   theResource.getType() != ds::Resource::WEB_TYPE &&
					   theResource.getType() != ds::Resource::YOUTUBE_TYPE &&
					   theResource.getType() != ds::Resource::VIDEO_STREAM_TYPE) {

					   if (ds::safeFileExistsCheck(theResource.getAbsoluteFilePath())) {
						   return {new FramedMediaViewer(mEngine, getChannelName()), CreationError::OK};
					   } else {

						   if (args.mIsDetached) {
							   ds::model::ContentModelRef errorModel;
							   std::string				  errorMessage =
								   "We couldn't load this piece of media because the file couldn't be found.";

							   errorModel.setProperty("name", std::string("Sorry!"));
							   errorModel.setProperty("error", errorMessage);
							   errorModel.setPropertyResource(mediaPropertyKey, theResource); // TODO
							   errorModel.setProperty("media_path", theResource.getAbsoluteFilePath());
							   errorModel.setProperty("media_name", args.mMediaRef.getPropertyString("name"));
							   auto eArgs = ViewerCreationArgs(errorModel, VIEW_TYPE_ERROR, args.mLocation,
															   ViewerCreationArgs::kViewLayerTop, 0, args.mFromCenter);


							   mChannelClient.notify(RequestViewerLaunchEvent(eArgs));
						   }
						   return {nullptr, CreationError::INVALID_MEDIA};
					   }
				   } else {
					   return {new FramedMediaViewer(mEngine, getChannelName()), CreationError::OK};
				   }
			   });
	setCreator(VIEW_TYPE_FULLSCREEN_CONTROLLER,
			   [this](const ViewerCreationArgs args) -> std::tuple<BaseElement*, CreationError> {
				   return {new FramedFullscreenController(mEngine, "waffles/viewer/framed_fsc.xml"), CreationError::OK};
			   });
}
void FramedViewerController::setupFullscreenDarkener(waffles::BaseElement*& viewer, bool& retFlag) {
	ViewerController::setupFullscreenDarkener(viewer, retFlag);
	mFullscreenDarkeners[viewer]->setTapCallback([this, viewer](ds::ui::Sprite*, const ci::vec3& pos) {
		if (viewer->getIsFullscreen()) {
			mEventClient.notify(RequestCollapseAndMoveFullscreenController(true, pos));
		}
	});
}
} // namespace waffles
