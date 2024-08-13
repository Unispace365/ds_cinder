#include "stdafx.h"

#include "asset_layer.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/media/player/web_player.h>
#include <ds/ui/menu/touch_menu.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/util/clip_plane.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/sprite/web.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/viewers/base_element.h"
#include "waffles/viewers/drawing/drawing_tools.h"
#include "waffles/viewers/titled_media_viewer.h"
#include "waffles/viewers/viewer_controller.h"

namespace waffles {

AssetLayer::AssetLayer(ds::ui::SpriteEngine& eng, bool isReceiver)
	: ds::ui::SmartLayout(eng, "waffles/layer/asset_layer.xml")
	, mAmReceiver(isReceiver) {

	// setTransparent(false);

	if (mEngine.getAppSettings().getString("app:mode", 0, "single") == "multi") {
		mAmMulti = true;

		if (mAmReceiver) {
			listenToEvents<waffles::ViewerControllerStarted>([this](const auto& ev) {
				auto bounds			   = ci::Rectf(0.f, 0.f, getWidth(), getHeight());
				auto transmitterBounds = mEngine.getAppSettings().getRect("app:screen:0", 0, bounds);

				auto fitBounds = bounds.getCenteredFit(transmitterBounds, true);
				mReceiverScale = bounds.getHeight() / fitBounds.getHeight();

				mViewerController = ev.mController;
			});
		}
	}
}

void AssetLayer::drawClient(const ci::mat4& transform, const ds::DrawParams& dp) {
	if (mAmMulti && mAmReceiver && mViewerController) {
		buildTransform();
		ci::mat4 totalTransformation =
			getInverseTransform() * glm::scale(ci::vec3(mReceiverScale, mReceiverScale, 1.f));

		// This is a big ole hack!
		// We're drawing the viewers a second time on the 'receiver' layer
		// Thus we need to set clipping planes before calling
		waffles::TitledMediaViewer* fullscreenViewer = nullptr;
		for (auto viewer : mViewerController->getViewers()) {
			if (auto mv = dynamic_cast<waffles::TitledMediaViewer*>(viewer)) {
				if (mv->getIsFullscreen()) {
					fullscreenViewer = mv;
				} else {
					ds::ui::clip_plane::enableClipping(0.f, 0.f, getWidth(), getHeight());
					if (auto mp = mv->getMediaPlayer()) {
						auto iface		   = mp->getMediaInterface();
						bool hideInterface = (iface != nullptr);
						if (hideInterface) {
							iface->hide();
						}
						mp->drawClient(totalTransformation * mv->getTransform(), dp);
						if (hideInterface) {
							iface->show();
						}
						// mv->drawClient(totalTransformation, dp);
					}
					if (mv->getIsDrawingMode()) {
						if (auto drawingArea = mv->getDrawingArea()) {
							drawingArea->getDrawingTools()->hide();
							drawingArea->drawClient(totalTransformation * mv->getTransform(), dp);
							drawingArea->getDrawingTools()->show();
						}
					}
					ds::ui::clip_plane::disableClipping();
				}
			}
			for (auto [viewer, darkener] : mViewerController->getFullscreenDarkeners()) {
				ds::ui::clip_plane::enableClipping(0.f, 0.f, getWidth(), getHeight());
				auto newDp = dp;
				newDp.mParentOpacity = 0.4f;
				darkener->drawClient(totalTransformation, newDp);
				ds::ui::clip_plane::disableClipping();
			}

			if (fullscreenViewer) {

				ds::ui::clip_plane::enableClipping(0.f, 0.f, getWidth(), getHeight());
				if (auto mp = fullscreenViewer->getMediaPlayer()) {
					auto iface		   = mp->getMediaInterface();
					bool hideInterface = (iface != nullptr);
					if (hideInterface) {
						iface->hide();
					}
					mp->drawClient(totalTransformation * fullscreenViewer->getTransform(), dp);
					if (hideInterface) {
						iface->show();
					}
					// fullscreenViewer->drawClient(totalTransformation, dp);
				}
				if (fullscreenViewer->getIsDrawingMode()) {
					if (auto drawingArea = fullscreenViewer->getDrawingArea()) {
						drawingArea->getDrawingTools()->hide();
						drawingArea->drawClient(totalTransformation * fullscreenViewer->getTransform(), dp);
						drawingArea->getDrawingTools()->show();
					}
				}
				ds::ui::clip_plane::disableClipping();
			}
		}
	} else {
		ds::ui::SmartLayout::drawClient(transform, dp);
	}
}

void AssetLayer::onSizeChanged() {
	clearChildren();

	if (mAmMulti) {
		// Add the main waffles controller sprite
		if (!mAmReceiver) {
			// Wall gets only reciever version
			mViewerController = new waffles::ViewerController(mEngine, ci::vec2(getSize()));
			addChildPtr(mViewerController);
			setSize(ci::vec2(getSize()));
			runLayout();

			auto bounds			= ci::Rectf(0.f, 0.f, getWidth(), getHeight());
			auto receiverBounds = mEngine.getAppSettings().getRect("app:screen:1", 0, bounds);

			auto fitBounds = receiverBounds.getCenteredFit(bounds, true);

			mViewerController->setLayerBounds(waffles::ViewerCreationArgs::kViewLayerBackground, fitBounds);
			mViewerController->setLayerBounds(waffles::ViewerCreationArgs::kViewLayerNormal, fitBounds);

			callAfterDelay([this] { mEngine.getNotifier().notify(waffles::ViewerControllerStarted(mViewerController)); },
						   0.15f);
		}
	} else {
		mViewerController = new waffles::ViewerController(mEngine, ci::vec2(getSize()));
		addChildPtr(mViewerController);
	}
}


} // namespace waffles
