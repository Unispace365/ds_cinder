#pragma once
#ifndef _VIDEO_CONVERTER_APP_UI_CONVERSION_ITEM
#define _VIDEO_CONVERTER_APP_UI_CONVERSION_ITEM

#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/video.h>

#include <gstreamer/video_meta_cache.h>

#include <shellapi.h>

namespace downstream {

/**
* \class downstream::ConversionItem
*			A view that shows what files are being converted
*/
class ConversionItem : public ds::ui::SmartLayout {
public:
	ConversionItem(ds::ui::SpriteEngine& eng, const std::string filePath);

	void				startConversion();
	void				close();

	void				setTransparentType(const bool isTransparent);
	void				setScaleRestriction(const std::string theScale);

	void				setCloseCallback(std::function<void()> func) { mCloseCallback = func; }

	/// not currently working
	void				startConversionGstreamer();

	virtual void					onUpdateServer(const ds::UpdateParams& p) override;
	ds::ui::Video*					mVideoConversion;
	std::string						mFilePath;
	std::string						mOutputPath;
	bool							mCompleted;
	bool							mStarted;
	bool							mValid;

	bool							mIsTransparent;
	std::string						mScaleValue;

	std::function<void()>			mCloseCallback;
	ds::ui::VideoMetaCache::Entry	mMetaEntry;

	SHELLEXECUTEINFOA				ShExecInfo;
};

} // namespace downstream

#endif

