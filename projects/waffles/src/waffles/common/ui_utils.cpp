#include "stdafx.h"

#include "ui_utils.h"

#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/media/player/pdf_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/media/player/web_player.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"

#include "waffles/waffles_events.h"

namespace waffles {

ContentUtils::ContentUtils(ds::ui::SpriteEngine& g)
  : ds::ui::Sprite(g) {
	// read in acceptable content from waffles settings
	auto foldersCount = mEngine.getWafflesSettings().countSetting("launcher:folder:key");
	for (int i = 0; i < foldersCount; ++i) {
		auto folder = mEngine.getWafflesSettings().getString("launcher:folder:key", i, "");
		mAcceptableFolders.push_back(folder);
	}

	auto mediaCount = mEngine.getWafflesSettings().countSetting("launcher:media:key");
	for (int i = 0; i < mediaCount; ++i) {
		auto media = mEngine.getWafflesSettings().getString("launcher:media:key", i, "");
		mAcceptableMedia.push_back(media);
	}

	auto presentationCount = mEngine.getWafflesSettings().countSetting("launcher:presentation:key");
	for (int i = 0; i < presentationCount; ++i) {
		auto presentation = mEngine.getWafflesSettings().getString("launcher:presentation:key", i, "");
		mAcceptablePresentations.push_back(presentation);
	}

	auto ambientPlaylistCount = mEngine.getWafflesSettings().countSetting("launcher:ambient_playlist:key");
	for (int i = 0; i < ambientPlaylistCount; ++i) {
		auto ambientPlaylist = mEngine.getWafflesSettings().getString("launcher:ambient_playlist:key", i, "");
		mAcceptableAmbientPlaylists.push_back(ambientPlaylist);
	}
}

bool ContentUtils::isFolder(ds::model::ContentModelRef model) {
	auto uid	= model.getPropertyString("type_uid");
	auto type	= model.getPropertyString("type_key");
	bool result = std::find(mAcceptableFolders.begin(), mAcceptableFolders.end(), type) != mAcceptableFolders.end();
	result = result || std::find(mAcceptableFolders.begin(), mAcceptableFolders.end(), uid) != mAcceptableFolders.end();
	return result;
}

bool ContentUtils::isMedia(ds::model::ContentModelRef model) {
	auto uid	= model.getPropertyString("type_uid");
	auto type	= model.getPropertyString("type_key");
	bool result = std::find(mAcceptableMedia.begin(), mAcceptableMedia.end(), type) != mAcceptableMedia.end();
	result		= result || std::find(mAcceptableMedia.begin(), mAcceptableMedia.end(), uid) != mAcceptableMedia.end();
	return result;
}

bool ContentUtils::isPresentation(ds::model::ContentModelRef model) {
	auto uid	= model.getPropertyString("type_uid");
	auto type	= model.getPropertyString("type_key");
	bool result = std::find(mAcceptablePresentations.begin(), mAcceptablePresentations.end(), type) !=
				  mAcceptablePresentations.end();
	result = result || std::find(mAcceptablePresentations.begin(), mAcceptablePresentations.end(), uid) !=
						   mAcceptablePresentations.end();
	return result;
}

bool ContentUtils::isAmbientPlaylist(ds::model::ContentModelRef model) {
	auto uid	= model.getPropertyString("type_uid");
	auto type	= model.getPropertyString("type_key");
	bool result = std::find(mAcceptableAmbientPlaylists.begin(), mAcceptableAmbientPlaylists.end(), type) !=
				  mAcceptableAmbientPlaylists.end();
	result = result || std::find(mAcceptableAmbientPlaylists.begin(), mAcceptableAmbientPlaylists.end(), uid) !=
						   mAcceptableAmbientPlaylists.end();
	return result;
}


ContentUtils* ContentUtils::getDefault(ds::ui::SpriteEngine& g) {
	static ContentUtils* sDefault = nullptr;
	if (!sDefault) {
		sDefault = new ContentUtils(g);
	}
	return sDefault;
}

void ContentUtils::configureListItem(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, ci::vec2 size) {

	std::string thumbPath  = "";
	auto		theModel = item->getContentModel();
	auto		theType	   = item->getContentModel().getPropertyString("type_key");
	auto		theTypeUid = item->getContentModel().getPropertyString("type_uid");
	auto		mediaType  = item->getContentModel().getPropertyResource("media").getType();

	bool showArrow = false;
	bool validy	   = true;

	if (theType == "assets_folder" || theType == "playlist_folder" || theType == waffles::MEDIA_TYPE_DIRECTORY_CMS || getDefault(engine)->isFolder(theModel)) {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Folder_256.png";
		showArrow = true;
	} else if (theType == "current_playlist" ) {
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
		showArrow = true;
	} else if (theType == "pinboard") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
		showArrow = true;
	} else if (theType == "pinboard_event") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
	} else if (theType == "pinboard_mode") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Pin_256.png";
	} else if (theType == "browser") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Browser_256.png";
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
			thumbPath = "%APP%/data/images/waffles/icons/4x/Image_256.png";
		} else if (mediaType == ds::Resource::PDF_TYPE) {
			thumbPath = "%APP%/data/images/waffles/icons/4x/PDF_256.png";
		} else if (mediaType == ds::Resource::VIDEO_TYPE || mediaType == ds::Resource::YOUTUBE_TYPE) {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Video_256.png";
		} else if (mediaType == ds::Resource::WEB_TYPE) {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Link_256.png";
		} else if (theType == "media" && mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
			thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
		}

	} else if (theType == "Recent") {
		thumbPath = "%APP%/data/images/waffles/icons/1x/Star_64.png";
	} else if (theType == "Images") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Image_256.png";
	} else if (theType == "Links") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Link_256.png";
	} else if (theType == "PDFs") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/PDF_256.png";
	} else if (theType == "Presentations") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Presentations_256.png";
	} else if (theType == "Streams") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Stream_256.png";
	} else if (theType == "Videos") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Video_256.png";
	} else if (theType == "Folders") {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Folder_256.png";
	} else {
		thumbPath = "%APP%/data/images/waffles/icons/4x/Asset viewing_256.png";
		validy	  = true;
	}


	ds::cfg::Settings settings;
	settings.getSetting("label", 0).mOriginalValue	   = item->getContentModel().getPropertyString("record_name");
	settings.getSetting("icon_src", 0).mOriginalValue  = thumbPath;
	settings.getSetting("has_arrow", 0).mOriginalValue = ds::unparseBoolean(showArrow);
	item->setLayoutSettings(settings);

	item->initialize();
	if (size.x > 0.0f && size.y > 0.0f) {
		item->setSize(size);
	}
	item->runLayout();
}

bool ContentUtils::handleListItemTap(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, ci::vec3 pos) {
	auto model	 = item->getContentModel();
	auto type	 = model.getPropertyString("type_key");
	auto typeUid = model.getPropertyString("type_uid");

	if (type == "ambient") {
		engine.startIdling();
	} else if (type == "media_template") {
		// Special case for disambiguating media template from media item
		engine.getNotifier().notify(waffles::RequestEngagePresentation(model));
	} else if (type == "media" || getDefault(engine)->isMedia(model)) {
		engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
			waffles::ViewerCreationArgs(model, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
	} else if (type == "browser") {
		auto browserRes	  = ds::Resource("https://google.com");
		auto browserModel = ds::model::ContentModelRef();
		browserModel.setPropertyResource("media", browserRes);
		engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
			waffles::ViewerCreationArgs(browserModel, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
	} else if (type == "asset_mode") {
		engine.getNotifier().notify(waffles::RequestEngagePresentation(ds::model::ContentModelRef("assets")));
		engine.getNotifier().notify(waffles::ChangeTemplateRequest());
	} else if (type == "search") {
		engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
			waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_SEARCH, pos)));
	} else if (type == "stream") {
		auto streamRes = ds::Resource(model.getPropertyString("stream_uri"));
		streamRes.setWidth(1920.f);
		streamRes.setHeight(1080.f);
		streamRes.setType(ds::Resource::VIDEO_STREAM_TYPE);

		auto streamModel = ds::model::ContentModelRef();
		streamModel.setProperty("record_name", model.getPropertyString("record_name"));
		streamModel.setPropertyResource("media", streamRes);
		engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
			waffles::ViewerCreationArgs(streamModel, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
	} else if (type == "pinboard_event") {
		engine.getNotifier().notify(waffles::RequestEngagePresentation(model));
		engine.getNotifier().notify(waffles::ChangeTemplateRequest(model));
	} else if (type == "presentation_controller") {
		engine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
			waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_PRESENTATION_CONTROLLER, pos,
										waffles::ViewerCreationArgs::kViewLayerTop)));
	} else if (type == "close_assets") {
		engine.getNotifier().notify(waffles::RequestCloseAllEvent(pos));
	} else if (type == "Recent" || type == "Images" || type == "Links" || type == "PDFs" || type == "Presentations" ||
			   type == "Streams" || type == "Videos" || type == "Folders") {
		engine.getNotifier().notify(waffles::WafflesFilterEvent(type, true));
	} else {
		// engine.getNotifier().notify(RequestEngagePresentation(model));
		return false;
	}

	return true;
}


void ContentUtils::setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) {
	if (!interfacey) return;
	auto& mEngine = interfacey->getEngine();

	auto cornerRad		= mEngine.getWafflesSettings().getFloat("ui:corner_radius", 0, 0.0f);
	auto interfaceScale = mEngine.getWafflesSettings().getFloat("ui:interface_scale", 0, 1.0f);

	auto viewerBackground = mEngine.getColors().getColorFromName("viewer_background");
	auto backgroundColor  = mEngine.getColors().getColorFromName("ui_background");
	auto normalColor	  = mEngine.getColors().getColorFromName("ui_normal");
	auto highColor		  = mEngine.getColors().getColorFromName("waffles_bloom");

	const auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

	if (auto vidInterface = dynamic_cast<ds::ui::VideoInterface*>(interfacey)) {
		auto interfaceHeight = vidInterface->getHeight();
		if (auto play = vidInterface->getPlayButton()) {
			play->setNormalImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			play->setHighImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			play->setScale(interfaceHeight / play->getHeight());
			play->setNormalImageColor(normalColor);
			play->setHighImageColor(highColor);
		}

		if (auto pause = vidInterface->getPauseButton()) {
			pause->setNormalImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			pause->setHighImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			pause->setScale(interfaceHeight / pause->getHeight());
			pause->setNormalImageColor(normalColor);
			pause->setHighImageColor(highColor);
		}

		if (auto loopy = vidInterface->getLoopButton()) {
			loopy->setNormalImage("%APP%/data/images/waffles/icons/4x/Loop_256.png", imageFlags);
			loopy->setHighImage("%APP%/data/images/waffles/icons/4x/No loop_256.png", imageFlags);
			loopy->setScale(interfaceHeight / loopy->getHeight());
			loopy->setNormalImageColor(normalColor);
			loopy->setHighImageColor(highColor);
		}

		if (auto unloopy = vidInterface->getUnLoopButton()) {
			unloopy->setNormalImage("%APP%/data/images/waffles/icons/4x/No loop_256.png", imageFlags);
			unloopy->setHighImage("%APP%/data/images/waffles/icons/4x/Loop_256.png", imageFlags);
			unloopy->setScale(interfaceHeight / unloopy->getHeight());
			unloopy->setNormalImageColor(normalColor);
			unloopy->setHighImageColor(highColor);
		}

		if (vidInterface->getScrubBarBackground() && vidInterface->getScrubBarProgress()) {
			vidInterface->getScrubBarBackground()->setColor(normalColor);
			vidInterface->getScrubBarBackground()->setOpacity(0.2);
			vidInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			vidInterface->getScrubBarProgress()->setColor(normalColor);
			vidInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (vidInterface->getVolumeControl()) {
			auto volumeControl = vidInterface->getVolumeControl();
			volumeControl->setMuteImage("%APP%/data/images/waffles/icons/4x/Mute_256.png");
			volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons/4x/Volume low_256.png");
			volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons/4x/Volume high_256.png");
			volumeControl->setSliderHeight(8.f);
			volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			sliderSprites.mMuteButton->setNormalImageColor(normalColor);
			sliderSprites.mMuteButton->setHighImageColor(highColor);
			sliderSprites.mMuteButton->setScale(sliderSprites.mMuteButton->getScale() * 1.25f);

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.2);
			sliderSprites.mSliderTrack->setCornerRadius(cornerRad);

			sliderSprites.mSliderFill->setColor(normalColor);
			sliderSprites.mSliderFill->setCornerRadius(cornerRad);

			sliderSprites.mSliderNub->setColor(normalColor);
			sliderSprites.mSliderNub->setScale(sliderSprites.mSliderNub->getScale() * 1.25f);
			sliderSprites.mSliderNub->setCornerRadius(100.f);

			/* volumeControl->setStyle(ds::ui::VideoVolumeStyle::CLASSIC);
			auto volumeBars = vidInterface->getVolumeControl()->getBars();
			for (auto eachBar : volumeBars) {
				eachBar->setColor(normalColor);
				eachBar->setCornerRadius(cornerRad);
			}
			*/
		}
	}

	if (auto ytInterface = dynamic_cast<ds::ui::YoutubeInterface*>(interfacey)) {
		auto interfaceHeight = ytInterface->getHeight();
		if (auto play = ytInterface->getPlayButton()) {
			play->setNormalImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			play->setHighImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			play->setScale(interfaceHeight / play->getHeight());
			play->setNormalImageColor(normalColor);
			play->setHighImageColor(highColor);
		}

		if (auto pause = ytInterface->getPauseButton()) {
			pause->setNormalImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			pause->setHighImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			pause->setScale(interfaceHeight / pause->getHeight());
			pause->setNormalImageColor(normalColor);
			pause->setHighImageColor(highColor);
		}

		if (ytInterface->getScrubBarBackground() && ytInterface->getScrubBarProgress()) {
			ytInterface->getScrubBarBackground()->setColor(normalColor);
			ytInterface->getScrubBarBackground()->setOpacity(0.2);
			ytInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			ytInterface->getScrubBarProgress()->setColor(normalColor);
			ytInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (ytInterface->getVolumeControl()) {
			auto volumeControl = ytInterface->getVolumeControl();
			volumeControl->setMuteImage("%APP%/data/images/waffles/icons/4x/Mute_256.png");
			volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons/4x/Volume low_256.png");
			volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons/4x/Volume high_256.png");
			volumeControl->setSliderHeight(8.f);
			volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			sliderSprites.mMuteButton->setNormalImageColor(normalColor);
			sliderSprites.mMuteButton->setHighImageColor(highColor);
			sliderSprites.mMuteButton->setScale(sliderSprites.mMuteButton->getScale() * 1.25f);

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.2);
			sliderSprites.mSliderTrack->setCornerRadius(cornerRad);

			sliderSprites.mSliderFill->setColor(normalColor);
			sliderSprites.mSliderFill->setCornerRadius(cornerRad);

			sliderSprites.mSliderNub->setColor(normalColor);
			sliderSprites.mSliderNub->setScale(sliderSprites.mSliderNub->getScale() * 1.25f);
			sliderSprites.mSliderNub->setCornerRadius(100.f);

			/* volumeControl->setStyle(ds::ui::VideoVolumeStyle::CLASSIC);
			auto volumeBars = ytInterface->getVolumeControl()->getBars();
			for (auto eachBar : volumeBars) {
				eachBar->setColor(normalColor);
				eachBar->setCornerRadius(cornerRad);
			}
			*/
		}
	}

	auto webInterface = dynamic_cast<ds::ui::WebInterface*>(interfacey);
	if (webInterface) {
		webInterface->setKeyboardDisablesTimeout(false);
		/* TODO: getKeyboardArea() doesn't exist
		if (auto keebArea = webInterface->getKeyboardArea()) {
			keebArea->setCornerRadius(0.f);
		}
		*/
		if (auto uppy = webInterface->getKeyboardButton()) {
			uppy->setNormalImageColor(normalColor);
			uppy->setHighImageColor(highColor);
			uppy->setCornerRadius(0.f);
		}
		if (auto downy = webInterface->getBackButton()) {
			downy->setNormalImageColor(normalColor);
			downy->setHighImageColor(highColor);
			downy->setCornerRadius(0.f);
		}
		if (auto toggy = webInterface->getForwardButton()) {
			toggy->setNormalImageColor(normalColor);
			toggy->setHighImageColor(highColor);
			toggy->setCornerRadius(0.f);
		}
		if (auto thumbs = webInterface->getRefreshButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
			thumbs->setCornerRadius(0.f);
		}
		if (auto thumbs = webInterface->getTouchToggleButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
			thumbs->setCornerRadius(0.f);
		}
	}

	auto pdfInterface = dynamic_cast<ds::ui::PDFInterface*>(interfacey);
	if (pdfInterface) {
		if (auto uppy = pdfInterface->getUpButton()) {
			uppy->setNormalImageColor(normalColor);
			uppy->setHighImageColor(highColor);
		}
		if (auto downy = pdfInterface->getDownButton()) {
			downy->setNormalImageColor(normalColor);
			downy->setHighImageColor(highColor);
		}
		if (auto toggy = pdfInterface->getTouchToggle()) {
			toggy->setNormalImageColor(normalColor);
			toggy->setHighImageColor(highColor);
		}
		if (auto thumbs = pdfInterface->getThumbsButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
		}
		if (auto count = pdfInterface->getPageCounter()) {
			count->setColor(normalColor);
		}
		if (pdfInterface->getScrubBarBackground() && pdfInterface->getScrubBarProgress()) {
			pdfInterface->getScrubBarBackground()->setColor(highColor);
			pdfInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			pdfInterface->getScrubBarProgress()->setColor(normalColor);
			pdfInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}
	}

	interfacey->setBackgroundColor(viewerBackground);
	interfacey->getBackground()->setCornerRadius(cornerRad);
	interfacey->setScale(interfaceScale, interfaceScale);
	if (mEngine.getAppSettings().getString("app:mode", 0, "single") == "multi") {
		interfacey->move(mEngine.getWafflesSettings().getFloat("media_viewer:multi_offset", 0, 0.f), 0.f);
	}
}

} // namespace waffles
