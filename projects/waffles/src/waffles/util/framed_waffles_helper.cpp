#include "stdafx.h"

#include "framed_waffles_helper.h"

#include <ds/ui/button/layout_button.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/button/toggle_container.h>
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/media/media_interface.h>

#include <glm/gtx/matrix_decompose.hpp>


namespace waffles {

FramedWafflesHelper::FramedWafflesHelper(ds::ui::SpriteEngine& eng)
  : BaseWafflesHelper(eng) {}


FramedWafflesHelper::~FramedWafflesHelper() = default;


void FramedWafflesHelper::setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) {
	if (!interfacey) return;

	auto& engine			  = interfacey->getEngine();
	auto  playHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:play:size", 0, 0.0f);
	auto  pauseHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:pause:size", 0, 0.0f);
	auto  keyboardHeight	  = engine.getWafflesSettings().getFloat("ui:media_button:keyboard:size", 0, 0.0f);
	auto  backHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:back:size", 0, 0.0f);
	auto  forwardHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:forward:size", 0, 0.0f);
	auto  refreshHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:refresh:size", 0, 0.0f);
	auto  lockHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:lock:size", 0, 0.0f);
	auto  loopHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:loop:size", 0, 0.0f);
	auto  volumeHeight		  = engine.getWafflesSettings().getFloat("ui:media_button:volume:size", 0, 0.0f);
	auto  pageIndicatorHeight = engine.getWafflesSettings().getFloat("ui:media_button:page_indicator:size", 0, 0.0f);
	auto  scrubBarHeight	  = engine.getWafflesSettings().getFloat("ui:media_button:scrub_bar:size", 0, 0.0f);

	auto cornerRad		= engine.getWafflesSettings().getFloat("ui:corner_radius", 0, 0.0f);
	auto interfaceScale = engine.getWafflesSettings().getFloat("ui:interface_scale", 0, 1.0f);

	auto viewerBackground = engine.getColors().getColorFromName("transparent_white");
	auto backgroundColor  = engine.getColors().getColorFromName("viewer_background");
	auto normalColor	  = engine.getColors().getColorFromName("ui_normal");
	auto highColor		  = engine.getColors().getColorFromName("ui_selected");

	constexpr auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

	if (auto vidInterface = dynamic_cast<ds::ui::VideoInterface*>(interfacey)) {
		auto interfaceHeight = vidInterface->getHeight();
		if (auto play = vidInterface->getPlayButton()) {
			vidInterface->setButtonColor(play, normalColor, highColor);
		}

		if (auto pause = vidInterface->getPauseButton()) {
			vidInterface->setButtonColor(pause, normalColor, highColor);
		}

		if (auto loopy = vidInterface->getLoopButton()) {
			vidInterface->setButtonColor(loopy, normalColor, highColor);
		}

		if (auto unloopy = vidInterface->getUnLoopButton()) {
			vidInterface->setButtonColor(unloopy, normalColor, highColor);
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
			// volumeControl->setMuteImage("%APP%/data/images/waffles/icons2/2x/mute.png");
			// volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons2/2x/volume_down.png");
			// volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons2/2x/volume_up.png");
			// volumeControl->setSliderHeight(8.f);
			// volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			
			vidInterface->setButtonColor(sliderSprites.mMuteButton, normalColor, highColor);
			vidInterface->setButtonColor(sliderSprites.mVolHighButton, normalColor, highColor);
			vidInterface->setButtonColor(sliderSprites.mVolLowButton, normalColor, highColor);

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
			ytInterface->setButtonColor(play, normalColor, highColor); // play button
		}

		if (auto pause = ytInterface->getPauseButton()) {
			ytInterface->setButtonColor(pause, normalColor, highColor); // pause button
		}

		if (ytInterface->getScrubBarBackground() && ytInterface->getScrubBarProgress()) {
			ytInterface->getScrubBarBackground()->setColor(normalColor);
			ytInterface->getScrubBarBackground()->setOpacity(0.25);
			ytInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			ytInterface->getScrubBarProgress()->setColor(normalColor);
			ytInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (ytInterface->getVolumeControl()) {
			auto volumeControl = ytInterface->getVolumeControl();
			// volumeControl->setMuteImage("%APP%/data/images/waffles/icons2/2x/mute.png");
			// volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons2/2x/volume_down.png");
			// volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons2/2x/volume_up.png");
			// volumeControl->setSliderHeight(8.f);
			// volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			ytInterface->setButtonColor(sliderSprites.mMuteButton, normalColor, highColor);
			ytInterface->setButtonColor(sliderSprites.mVolHighButton, normalColor, highColor);
			ytInterface->setButtonColor(sliderSprites.mVolLowButton, normalColor, highColor);
			

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.25);
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
		auto interfaceHeight = webInterface->getHeight();
		webInterface->setKeyboardDisablesTimeout(false);

		if (auto keebArea = webInterface->getKeyboardArea()) {
			// keebArea->setCornerRadius(0.f);
			keebArea->setColor(backgroundColor);
		}

		if (auto keyboard = webInterface->getKeyboardButton()) {

			webInterface->setButtonColor(keyboard->getCheckedButton(), highColor,
										 normalColor);
			webInterface->setButtonColor(keyboard->getUncheckedButton(), normalColor,highColor);
		}
		if (auto backy = webInterface->getBackButton()) {
			webInterface->setButtonColor(backy, normalColor, highColor);
		}
		if (auto forward = webInterface->getForwardButton()) {
			webInterface->setButtonColor(forward, normalColor, highColor);
		}
		if (auto reload = webInterface->getRefreshButton()) {
			webInterface->setButtonColor(reload, normalColor, highColor);
		}
		if (auto toggy = webInterface->getTouchToggleButton()) {
			webInterface->setButtonColor(toggy->getUncheckedButton(), normalColor, highColor);
			webInterface->setButtonColor(toggy->getCheckedButton(), highColor, normalColor);
		}
	}

	auto pdfInterface = dynamic_cast<ds::ui::PDFInterface*>(interfacey);
	if (pdfInterface) {
		auto interfaceHeight = pdfInterface->getHeight();
		if (auto uppy = pdfInterface->getUpButton()) {
			// uppy->setNormalImage("%APP%/data/images/waffles/icons2/2x/prev.png", imageFlags);
			// uppy->setHighImage("%APP%/data/images/waffles/icons2/2x/prev.png", imageFlags);
			// uppy->setScale(interfaceHeight / uppy->getHeight());
			pdfInterface->setButtonColor(uppy, normalColor, highColor);
		}
		if (auto downy = pdfInterface->getDownButton()) {
			// downy->setNormalImage("%APP%/data/images/waffles/icons2/2x/arrow_next.png", imageFlags);
			// downy->setHighImage("%APP%/data/images/waffles/icons2/2x/arrow_next.png", imageFlags);
			// downy->setScale(interfaceHeight / downy->getHeight());
			pdfInterface->setButtonColor(downy, normalColor, highColor);
		}
		if (auto toggy = pdfInterface->getTouchToggle()) {
			// pdfInterface->setToggleLockedImage("%APP%/data/images/waffles/icons2/2x/locked.png");
			// pdfInterface->setToggleUnlockedImage("%APP%/data/images/waffles/icons2/2x/unlocked.png");
			pdfInterface->setToggleUnlockedColor(normalColor);
			pdfInterface->setToggleLockedColor(highColor);
		}
		if (auto thumbs = pdfInterface->getThumbsButton()) {
			pdfInterface->setButtonColor(thumbs, normalColor, highColor);
		}
		if (auto count = pdfInterface->getPageCounter()) {
			count->setColor(normalColor);
		}
		if (pdfInterface->getScrubBarBackground() && pdfInterface->getScrubBarProgress()) {
			pdfInterface->getScrubBarBackground()->setColor(normalColor);
			pdfInterface->getScrubBarBackground()->setOpacity(0.25);
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
	if (engine.getAppSettings().getString("app:mode", 0, "single") == "multi") {
		interfacey->move(engine.getWafflesSettings().getFloat("media_viewer:multi_offset", 0, 0.f), 0.f);
	}
}

} // namespace waffles
