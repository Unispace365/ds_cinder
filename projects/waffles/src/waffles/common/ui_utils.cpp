#include "stdafx.h"

#include "ui_utils.h"

#include <ds/ui/button/image_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <utility>

#include "app/waffles_app_defs.h"

#include "waffles/util/waffles_helper.h"
#include "waffles/waffles_events.h"

namespace waffles {

ContentUtils::ContentUtils(ds::ui::SpriteEngine& g)
  : Sprite(g) {
	// read in acceptable content from waffles settings
}


ContentUtils* ContentUtils::getDefault(ds::ui::SpriteEngine& g) {
	static ContentUtils* sDefault = nullptr;
	if (!sDefault) {
		sDefault = new ContentUtils(g);
	}
	return sDefault;
}

bool ContentUtils::isFolder(ds::model::ContentModelRef model) const {
	auto content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	return content->isValidFolder(std::move(model), ds::model::ContentHelper::WAFFLESCATEGORY);
}

bool ContentUtils::isMedia(ds::model::ContentModelRef model) const {
	auto content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	return content->isValidMedia(std::move(model), ds::model::ContentHelper::WAFFLESCATEGORY);
}

bool ContentUtils::isPresentation(ds::model::ContentModelRef model) const {
	auto content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	return content->isValidPlaylist(std::move(model), ds::model::ContentHelper::PRESENTATIONCATEGORY);
}

bool ContentUtils::isAmbientPlaylist(ds::model::ContentModelRef model) const {
	auto content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	return content->isValidPlaylist(std::move(model), ds::model::ContentHelper::AMBIENTCATEGORY);
}

std::string ContentUtils::getMediaPropertyKey(ds::model::ContentModelRef model) const {
	auto content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	return content->getMediaPropertyKey(std::move(model), ds::model::ContentHelper::WAFFLESCATEGORY);
}

void ContentUtils::configureListItem(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, const ci::vec2& size,
									 bool isSelectable) {
	auto		content = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	std::string thumbPath;
	auto		theModel		 = item->getContentModel();
	auto		theType			 = item->getContentModel().getPropertyString("type_key");
	auto		theTypeLabel	 = "UNKNOWN";
	auto		theTypeUid		 = item->getContentModel().getPropertyString("type_uid");
	auto		mediaPropertyKey = content->getMediaPropertyKey(item->getContentModel());
	mediaPropertyKey			 = mediaPropertyKey.empty() ? "media" : mediaPropertyKey;
	auto mediaType				 = item->getContentModel().getPropertyResource(mediaPropertyKey).getType();
	bool useThumbnails			 = engine.getWafflesSettings().getBool("launcher:items:thumbnails_as_icons", 0, true);

	bool showArrow	= false;
	bool showSelect = false;
	bool validy		= true;

	if (theType == "assets_folder" || theType == "playlist_folder" || theType == MEDIA_TYPE_DIRECTORY_CMS ||
		getDefault(engine)->isFolder(theModel)) {
		thumbPath	 = "%APP%/data/images/waffles/icons/4x/Folder_256.png";
		theTypeLabel = "FOLDER";
		showArrow	 = true;
	} else if (theType == "current_playlist") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Playlist_256.png";
		showArrow = true;
	} else if (theType == "ambient" || getDefault(engine)->isAmbientPlaylist(theModel)) {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Ambient_256.png";
	} else if (theType == "presentation_controller") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Presentations_256.png";
	} else if (theType == "close_assets") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Close_256.png";
	} else if (theType == "presentation" || getDefault(engine)->isPresentation(theModel)) {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Presentations_256.png";
		showArrow = false;
	} else if (theType == "pinboard") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
		showArrow = true;
	} else if (theType == "pinboard_event") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
	} else if (theType == "pinboard_mode") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
	} else if (theType == "browser") {
		thumbPath  = "%APP%/data/images/waffles/icons/4x/Browser_256.png";
		showSelect = isSelectable;
	} else if (theType == "streams") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
		showArrow = true;
	} else if (theType == "stream") {
		auto iconPath = item->getContentModel().getPropertyResource("icon").getAbsoluteFilePath();
		if (iconPath.empty()) {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
		} else {
			thumbPath = iconPath;
		}
		theTypeLabel = "STREAM";
		showSelect	 = isSelectable;
	} else if (theType == "search") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Search_256.png";
	} else if (theType == "custom_layout_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Custom Layout_256.png";
	} else if (theType == "particle_text_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Particle_Text_256.png";
	} else if (theType == "message_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Message_256.png";
	} else if (theType == "bubbles_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Bubbles_256.png";
	} else if (theType == "media_gallery") {
		thumbPath = "%APP%/data/images/waffles/ui/Media_Gallery_256.png";
	} else if (theType == "feature_story_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Feature Story_256.png";
	} else if (theType == "cards_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Cards_256.png";
	} else if (theType == "carrousel_cards_template") {
		thumbPath = "%APP%/data/images/waffles/ui/Carrousel Cards_256.png";
	} else if (theType == "asset_mode") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Asset viewing_256.png";
	} else if (theType == "assets") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Assets stack_256.png";
		showArrow = true;
	} else if (theType == "assets_mode") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Asset viewing_256.png";
	} else if (theTypeUid == "ykAdRYGJeuXI") {
		thumbPath = "%APP%/data/images/ui/Media_256.png";
	} else if (theType == "media" || getDefault(engine)->isMedia(theModel)) {
		if (mediaType == ds::Resource::IMAGE_TYPE) {
			theTypeLabel = "IMAGE";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Image_256.png";
			if (useThumbnails) {
				auto preview =
					item->getContentModel().getPropertyResource(mediaPropertyKey + "_preview").getAbsoluteFilePath();
				if (!preview.empty()) {
					thumbPath = preview;
				}
			}
		} else if (mediaType == ds::Resource::PDF_TYPE) {
			theTypeLabel = "PDF";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/PDF_256.png";
			if (useThumbnails) {
				auto preview =
					item->getContentModel().getPropertyResource(mediaPropertyKey + "_preview").getAbsoluteFilePath();
				if (!preview.empty()) {
					thumbPath = preview;
				}
			}
		} else if (mediaType == ds::Resource::VIDEO_TYPE || mediaType == ds::Resource::YOUTUBE_TYPE) {
			theTypeLabel = "VIDEO";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Video_256.png";
			if (useThumbnails) {
				auto preview =
					item->getContentModel().getPropertyResource(mediaPropertyKey + "_preview").getAbsoluteFilePath();
				if (!preview.empty()) {
					thumbPath = preview;
				}
			}
		} else if (mediaType == ds::Resource::WEB_TYPE) {
			if (theType == "miro_link")
				theTypeLabel = "MIRO";
			else if (theType == "google_drive_link")
				theTypeLabel = "DRIVE";
			else
				theTypeLabel = "WEB";
			thumbPath = "%APP%/data/images/waffles/icons/4x/Link_256.png";
			if (useThumbnails) {
				auto preview =
					item->getContentModel().getPropertyResource(mediaPropertyKey + "_preview").getAbsoluteFilePath();
				if (!preview.empty()) {
					thumbPath = preview;
				}
			}
		} else if (theType == "media" && mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
			theTypeLabel = "STREAM";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
		} else if (theType == "stream_source") {
			theTypeLabel  = "STREAM";
			auto layoutId = theModel.getPropertyString("layout_id");
			// Look up this layout id in the platform stream children
			// NOTE!!! This suddenly became quite redhat specific, but we don't have time to rework how the ui helper
			// here works
			thumbPath = item->getContentModel().getPropertyResource("icon").getAbsoluteFilePath();
			if (thumbPath.empty()) thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
		} else if (theType == "layout") {
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Layout4x.png";
			theTypeLabel = "LAYOUT";
		} else if (theType == "miro_meeting_placeholder") {
			theTypeLabel = "MIRO";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Miro Placeholder4x.png";
		} else if (theType == "google_drive_link_placeholder") {
			theTypeLabel = "DRIVE";
			thumbPath	 = "%APP%/data/images/waffles/icons/4x/Drive Placeholder4x.png";
		}
		showSelect = isSelectable;
	} else if (theType == "recent") {
		thumbPath = "%APP%/data/images/waffles/icons/1x/Star_64.png";
	} else if (theType == "images") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Image_256.png";
	} else if (theType == "links") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Link_256.png";
	} else if (theType == "pdfs") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/PDF_256.png";
	} else if (theType == "presentations") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Presentations_256.png";
	} else if (theType == "streams") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
	} else if (theType == "videos") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Video_256.png";
	} else if (theType == "folders") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Folder_256.png";
		/* } else if (theType == "miro_meeting_placeholder") {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Miro Placeholder4x.png";
		} else if (theType == "google_drive_link_placeholder") {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Drive Placeholder4x.png"; */
	} else {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Asset viewing_256.png";
		validy	  = true;
	}


	ds::cfg::Settings settings;
	settings.getSetting("label", 0).mOriginalValue		= item->getContentModel().getPropertyString("record_name");
	settings.getSetting("icon_src", 0).mOriginalValue	= thumbPath;
	settings.getSetting("has_arrow", 0).mOriginalValue	= ds::unparseBoolean(showArrow);
	settings.getSetting("has_select", 0).mOriginalValue = ds::unparseBoolean(showSelect);
	settings.getSetting("type", 0).mOriginalValue		= theTypeLabel;
	item->setLayoutSettings(settings);

	item->initialize();
	if (size.x > 0.0f && size.y > 0.0f) {
		item->setSize(size);
	}
	item->runLayout();
}

bool ContentUtils::handleListItemTap(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item,
									 const std::string& channel, const ci::vec3& pos, const ci::vec3& raw_pos) {

	auto model	 = item->getContentModel();
	auto type	 = model.getPropertyString("type_key");
	auto typeUid = model.getPropertyString("type_uid");
	//  else if (getDefault(engine)->isAmbientPlaylist(model))
	//	type = "ambient";

	auto& notifier = channel.empty() ? engine.getNotifier() : engine.getChannel(channel);

	// Try the customs first on the raw type
	auto customs = ds::model::ContentHelperFactory::getDefault<WafflesHelper>()->getLauncherCustomContent();
	if (customs.find(type) != customs.end()) {
		customs[type](model, raw_pos);
		return true;
	}

	// If not a custom, we can coerce types a bit and try the remaining options
	if (getDefault(engine)->isFolder(model)) {
		type = "folder";
	} else if (getDefault(engine)->isMedia(model)) {
		type = "media";
	} else if (getDefault(engine)->isPresentation(model)) {
		type = "presentation";
	}

	if (type == "ambient") {
		engine.startIdling();
	} else if (type == "media_template" || type == "presentation") {
		if (model.hasChildren()) {
			notifier.notify(RequestEngagePresentation(model.getChild(0)));
		} else {
			DS_LOG_WARNING("tried presentation open for model with no children " << model.getPropertyString("uid"));
		}
	} else if (type == "folder") {
		return false;
	} else if (type == "media") {
		auto media_field = model.getPropertyResource("media");
		if (media_field.empty()) {
			model.setPropertyResource("media", model.getPropertyResource("asset_media"));
		} // TODO: this is a solve for search items, but probably should be fixed initially
		notifier.notify(
			RequestViewerLaunchEvent(
				ViewerCreationArgs(model, VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)
			)
		);
	} else if (type == "browser") {
		auto browserRes	  = ds::Resource("https://google.com");
		auto browserModel = ds::model::ContentModelRef();
		browserModel.setPropertyResource("media", browserRes);
		notifier.notify(
			RequestViewerLaunchEvent(ViewerCreationArgs::detached(browserModel, VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
	} else if (type == "asset_mode") {
		auto model = ds::model::ContentModelRef();
		model.setName("assets");
		notifier.notify(RequestEngagePresentation(model));
		notifier.notify(ChangeTemplateRequest());
	} else if (type == "search") {
		notifier.notify(RequestViewerLaunchEvent(
			ViewerCreationArgs::detached(ds::model::ContentModelRef(), VIEW_TYPE_SEARCH, pos)));
	} else if (type == "stream") {
		auto streamRes = ds::Resource(model.getPropertyString("stream_uri"));
		streamRes.setWidth(1920.f);
		streamRes.setHeight(1080.f);
		streamRes.setType(ds::Resource::VIDEO_STREAM_TYPE);

		auto streamModel = ds::model::ContentModelRef();
		streamModel.setProperty("record_name", model.getPropertyString("record_name"));
		streamModel.setPropertyResource("media", streamRes);
		notifier.notify(
			RequestViewerLaunchEvent(ViewerCreationArgs::detached(streamModel, VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
	} else if (type == "pinboard_event") {
		notifier.notify(RequestEngagePresentation(model));
		notifier.notify(ChangeTemplateRequest(model));
	} else if (type == "presentation_controller") {
		notifier.notify(RequestViewerLaunchEvent(ViewerCreationArgs::detached(
			ds::model::ContentModelRef(), VIEW_TYPE_PRESENTATION_CONTROLLER, pos, ViewerCreationArgs::kViewLayerTop)));
	} else if (type == "close_assets") {
		notifier.notify(RequestCloseAllEvent(pos));
	} else {
		notifier.notify(WafflesFilterEvent(type, true));
	}

	return true;
}

std::string ContentUtils::extractYoutubeId(const std::string& url) {
	if (url.find("youtube.com") == std::string::npos && url.find("youtu.be") == std::string::npos) {
		return "";
	}
	auto splitters = std::vector<std::string> {
		"/v/", "\\v\\", "/watch?v=", "\\watch?v=", "/embed/", "\\embed\\", "youtu.be/", "youtu.be\\"
	};
	for (std::string splitter : splitters) {
		if (url.find(splitter) != std::string::npos) {
			auto parts = ds::split(url, splitter);
			if (parts.size() > 1) {
				std::string id = parts[1];
				for (auto separator : std::vector<std::string>{ "?", "&", "#" }) {
					id = ds::split(id, separator)[0];
				}
				if (id.size() == 11) {
					return id;
				}
			}
		}
	}
	if (url.find("/user/") != std::string::npos) {
		auto parts = ds::split(url, "/");
		if (!parts.empty()) {
			std::string last = parts[parts.size() - 1];
			if (last.size() == 11) {
				return last;
			}
		}
	}
    return "";
}

} // namespace waffles
