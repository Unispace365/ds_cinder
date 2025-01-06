#include "stdafx.h"
#include "framed_waffles_helper.h"
#include "ds/content/platform.h"
#include "ds/ui/media/media_interface.h"
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/button/image_button.h>


namespace waffles {
FramedWafflesHelper::FramedWafflesHelper(ds::ui::SpriteEngine& eng) :BaseWafflesHelper(eng) { }
FramedWafflesHelper::~FramedWafflesHelper() {}







void FramedWafflesHelper::setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) {
	if (!interfacey) return;
	auto& mEngine = interfacey->getEngine();

	auto cornerRad		= mEngine.getWafflesSettings().getFloat("ui:corner_radius", 0, 0.0f);
	auto interfaceScale = mEngine.getWafflesSettings().getFloat("ui:interface_scale", 0, 1.0f);

	auto viewerBackground = mEngine.getColors().getColorFromName("transparent_white");
	auto backgroundColor  = mEngine.getColors().getColorFromName("ui_background");
	auto normalColor	  = mEngine.getColors().getColorFromName("ui_normal");
	auto highColor		  = mEngine.getColors().getColorFromName("ui_selected");

	const auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

	if (auto vidInterface = dynamic_cast<ds::ui::VideoInterface*>(interfacey)) {
		auto interfaceHeight = vidInterface->getHeight();
		if (auto play = vidInterface->getPlayButton()) {
			play->setNormalImage("%APP%/data/images/waffles/icons2/4x/play.png", imageFlags);
			play->setHighImage("%APP%/data/images/waffles/icons2/4x/pause.png", imageFlags);
			play->setScale(interfaceHeight / play->getHeight());
			play->setNormalImageColor(normalColor);
			play->setHighImageColor(highColor);
		}

		if (auto pause = vidInterface->getPauseButton()) {
			pause->setNormalImage("%APP%/data/images/waffles/icons2/4x/pause.png", imageFlags);
			pause->setHighImage("%APP%/data/images/waffles/icons2/4x/play.png", imageFlags);
			pause->setScale(interfaceHeight / pause->getHeight());
			pause->setNormalImageColor(normalColor);
			pause->setHighImageColor(highColor);
		}

		if (auto loopy = vidInterface->getLoopButton()) {
			loopy->setNormalImage("%APP%/data/images/waffles/icons2/4x/loop.png", imageFlags);
			loopy->setHighImage("%APP%/data/images/waffles/icons2/4x/loop.png", imageFlags);
			loopy->setScale(interfaceHeight / loopy->getHeight());
			loopy->setNormalImageColor(normalColor);
			loopy->setHighImageColor(highColor);
		}

		if (auto unloopy = vidInterface->getUnLoopButton()) {
			unloopy->setNormalImage("%APP%/data/images/waffles/icons2/4x/loop.png", imageFlags);
			unloopy->setHighImage("%APP%/data/images/waffles/icons2/4x/loop.png", imageFlags);
			unloopy->setScale(interfaceHeight / unloopy->getHeight());
			unloopy->setNormalImageColor(highColor);
			unloopy->setHighImageColor(normalColor);
		}

		if (vidInterface->getScrubBarBackground() && vidInterface->getScrubBarProgress()) {
			vidInterface->getScrubBarBackground()->setColor(normalColor);
			vidInterface->getScrubBarBackground()->setOpacity(0.25);
			vidInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			vidInterface->getScrubBarProgress()->setColor(normalColor);
			vidInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (vidInterface->getVolumeControl()) {
			auto volumeControl = vidInterface->getVolumeControl();
			volumeControl->setMuteImage("%APP%/data/images/waffles/icons2/4x/mute.png");
			volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons2/4x/volume_down.png");
			volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons2/4x/volume_up.png");
			volumeControl->setSliderHeight(8.f);
			volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			sliderSprites.mMuteButton->setNormalImageColor(normalColor);
			sliderSprites.mMuteButton->setHighImageColor(highColor);
			sliderSprites.mMuteButton->setScale(sliderSprites.mMuteButton->getScale() * 1.25f);

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.25);
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
	interfacey->setCenter(0.0, 0.5);
	auto h = interfacey->getHeight();
	if (mEngine.getAppSettings().getString("app:mode", 0, "single") == "multi") {
		interfacey->move(mEngine.getWafflesSettings().getFloat("media_viewer:multi_offset", 0, 0.f), 0.f);
	}
}


} // namespace waffles