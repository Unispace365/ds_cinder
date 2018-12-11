#include "stdafx.h"

#include "conversion_item.h"

#include <ds/util/file_meta_data.h>

#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>

#include "events/app_events.h"

namespace downstream {


ConversionItem::ConversionItem(ds::ui::SpriteEngine& eng, const std::string filePath)
	: ds::ui::SmartLayout(eng, "conversion_item.xml")
	, mFilePath(filePath)
	, mVideoConversion(nullptr)
	, mStarted(false)
	, mValid(false)
	, mIsTransparent(false)
	, mScaleValue("iw:ih")
	, mCompleted(false)
{
	ShExecInfo.hProcess = NULL;

	Poco::Path thePath = Poco::Path(mFilePath);
	setSpriteText("file", thePath.getFileName());
	setSpriteText("path", mFilePath);

	auto imgBtn = getSprite<ds::ui::ImageButton>("image_button");
	if(imgBtn) {
		imgBtn->setClickFn([this] {
			close();
		});
	}

	if(ds::safeFileExistsCheck(mFilePath)) {

		mMetaEntry.mPath = mFilePath;

		if(ds::ui::VideoMetaCache::getVideoInfo(mMetaEntry)) {
			if(mMetaEntry.mType == ds::ui::VideoMetaCache::AUDIO_ONLY_TYPE) {
				setSpriteText("status", "<span color='red'>No video streams found, and no need to convert audio files.</span>");
			} else if(mMetaEntry.mType == ds::ui::VideoMetaCache::ERROR_TYPE) {
				setSpriteText("status", "<span color='red'>Video or audio info not detected! Perhaps it is the wrong type of file</span>");
			} else {
				mValid = true;

				std::stringstream ss;
				ss << "Size:<b>" << mMetaEntry.mWidth << "</b>x<b>" << mMetaEntry.mHeight << "</b>  codec:<b>" << mMetaEntry.mVideoCodec << "</b>  color space:<b>" << mMetaEntry.mColorSpace << "</b>  duration:<b>" << mMetaEntry.mDuration << "</b>";
				setSpriteText("metadata", ss.str());
				setSpriteText("status", "Ready to convert");
			}
		} else {
			setSpriteText("status", "<span color='red'>Video or audio info not detected!</span>");
		}

	} else {
		setSpriteText("status", "<span color='red'>File not found!</span>");
	}

	auto showOutputButton = getSprite<ds::ui::SpriteButton>("play_output.the_button");
	if(showOutputButton) {
		showOutputButton->enable(false);
		showOutputButton->setClickFn([this] {
			mEngine.getNotifier().notify(PlayVideoRequest(mOutputPath));
			
		});
	}

	auto showInFolder = getSprite<ds::ui::SpriteButton>("show_in_folder.the_button");
	if(showInFolder) {
		showInFolder->enable(false);
		showInFolder->setClickFn([this] {
			if(mOutputPath.empty() || !ds::safeFileExistsCheck(mOutputPath)) return;

			std::string thePath = mOutputPath;
			ds::replace(thePath, "/", "\\");
			std::string theCommand = "explorer /select,\"" + thePath + "\"";
			system(theCommand.c_str());
			//ShellExecuteA(NULL, "open", thePath.toString().c_str(), NULL, NULL, SW_SHOWDEFAULT);
		});
	}
}

void ConversionItem::startConversion() {
	if(!mValid) return;
	if(mStarted) return;

	std::string absPath = ds::Environment::expand(mFilePath);
	ds::replace(absPath, "\\", "/");
	std::string outPath = absPath;
	std::stringstream commandLine;
	// ffmpeg -i {input_file} -codec copy -c:v libx264 -profile:v high -level 4.2 -pix_fmt yuv420p {output_file}
	commandLine << "-i \"" << absPath << "\" ";

	if(mScaleValue != "iw:ih") {
		std::string pathScale = mScaleValue;
		ds::replace(pathScale, "/", "_");
		ds::replace(pathScale, "?", "_");
		ds::replace(pathScale, ":", "_");
		outPath.append("_s_" + pathScale);

		// -1 preserves the aspect ratio
		commandLine << "-vf scale=" << mScaleValue << " ";
	}

	if(mIsTransparent) {
		outPath.append("_pr4444.mov");
		commandLine << "-c:v prores_ks -profile:v 4 -pix_fmt yuva444p10le ";
	} else {
		outPath.append("_h264.mp4");
		commandLine << "-c:v libx264 -profile:v high -level 4.2 -pix_fmt yuv420p ";
	}
		
	commandLine << "\"" << outPath << "\"";

	mOutputPath = outPath;

	std::string theCommand = commandLine.str();
	DS_LOG_INFO("Converting with command line string: " << theCommand);
	//system(theCommand.c_str());

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	//ShExecInfo.fMask = NULL;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = "ffmpeg.exe";
	ShExecInfo.lpParameters = theCommand.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_NORMAL;
	ShExecInfo.hInstApp = NULL;

	ShellExecuteExA(&ShExecInfo);

	mStarted = true;
}

void ConversionItem::close() {
	if(ShExecInfo.hProcess && mStarted) {
		mStarted = false;
		CloseHandle(ShExecInfo.hProcess);
	}
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [this] {
		if(mCloseCallback) {
			mCloseCallback();
		}
		release();
	});
}

void ConversionItem::setTransparentType(const bool isTransparent) {
	mIsTransparent = isTransparent;
}

void ConversionItem::setScaleRestriction(const std::string theScale) {
	mScaleValue = theScale;
}

void ConversionItem::startConversionGstreamer() {
	/*
	/// Note: this "converts" the file, but doesn't finialize it correctly
	std::stringstream thePipeline;
	std::string absPath = ds::Environment::expand(mFilePath);
	ds::replace(absPath, "\\", "/");
	std::string outPath = "\"" + absPath + "_h264.mp4\"";

	/// Sample pipeline with video and audio
	/// gst-launch-1.0 filesrc location="D:/content/sample_videos_2/001.mov" ! decodebin name=demux demux. ! audioresample ! audioconvert ! avenc_aac ! mux. qtmux name=mux ! filesink location=D:/content/sample_videos_2/001_output.mp4 demux. ! videoconvert ! videoscale ! x264enc ! video/x-h264,format=(string)I420 ! h264parse ! mux.

	/// Sample pipeline with only video
	/// gst-launch-1.0 filesrc location="D:/content/sample_videos_2/000.mov" ! decodebin ! videoconvert ! videoscale ! x264enc ! video/x-h264,format=(string)I420 ! h264parse  ! qtmux ! filesink location=D:/content/sample_videos_2/000_output.mp4

	thePipeline << "filesrc location=\"" << absPath << "\" ! decodebin ";
	if(mMetaEntry.mType == ds::ui::VideoMetaCache::VIDEO_AND_AUDIO_TYPE) {
		thePipeline << " name=demux demux. ! audioresample ! audioconvert ! avenc_aac ! mux. mp4mux name=mux ! filesink location=" << outPath << " demux. ! videoconvert ! videoscale ! x264enc ! video/x-h264,format=(string)I420 ! h264parse ! mux. ";
	} else {
		thePipeline << "! videoconvert ! videoscale ! x264enc ! video/x-h264,format=(string)I420 ! h264parse  ! mp4mux ! filesink location=" << outPath;
	}

	mVideoConversion = new ds::ui::Video(mEngine);
	addChildPtr(mVideoConversion);

	mVideoConversion->setErrorCallback([this](const std::string& error) {
		setSpriteText("status", "<span color='red'>Error!</span> something went wrong converting the video: " + error);
	});

	mVideoConversion->setVideoCompleteCallback([this] {
		setSpriteText("status", "<span color='green'>Conversion complete</span>");
		mVideoConversion->stop();
		auto mvc = mVideoConversion;
		mvc->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [mvc] { mvc->release(); });
		mVideoConversion = nullptr;
	});

	mVideoConversion->parseLaunch(thePipeline.str(), 100, 100, ds::ui::GstVideo::ColorType::kColorTypeSolid);
	setSpriteText("status", "Conversion starting...");
	*/
}

void ConversionItem::onUpdateServer(const ds::UpdateParams& p) {
	//ds::ui::SmartLayout::onUpdateServer(p);
	if(mVideoConversion && mVideoConversion->getIsPlaying()) {
		double percenty = 100.0 * mVideoConversion->getCurrentPosition();
		setSpriteText("status", "Converting: " + std::to_string(percenty) + "%");
	}


	if(ShExecInfo.hProcess && mStarted) {
		//WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

		DWORD  outCode;
		GetExitCodeProcess(ShExecInfo.hProcess, &outCode);

		if(outCode == 0) {
			setSpriteText("status", "Conversion complete");
			CloseHandle(ShExecInfo.hProcess);
			mStarted = false;
			mCompleted = true;

			auto showOutputButton = getSprite<ds::ui::SpriteButton>("play_output.the_button");
			if(showOutputButton) {
				showOutputButton->enable(true);
				showOutputButton->setOpacity(1.0f);
			}

			auto showInFolder = getSprite<ds::ui::SpriteButton>("show_in_folder.the_button");
			if(showInFolder) {
				showInFolder->enable(true);
				showInFolder->setOpacity(1.0f);
			}

		} else if(outCode == STILL_ACTIVE) {
			setSpriteText("status", "Converting, please wait");
		} else if(outCode == 3435973836){
			setSpriteText("status", "Starting...");
		} else {
			std::cout << "error: " << outCode << std::endl;
			mStarted = false;
			setSpriteText("status", "<span color='red'>Error!</span> code=" + std::to_string(outCode));
		}
	}
}

} // namespace downstream

