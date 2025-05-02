#pragma once
#ifndef DS_CONTENT_PLATFORM
#define DS_CONTENT_PLATFORM

#include <ds/app/event_client.h>
#include <ds/content/content_model.h>

namespace ds::model {

using PlatformType = std::string;

class Platform {
  public:
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper class or the ContentHelperFactory instead.")]]
	Platform(ui::SpriteEngine& engine, const std::string& platformKey = "");
	virtual ~Platform() = default;

	Platform() = delete;
	Platform(const Platform&)			 = delete;
	Platform& operator=(const Platform&) = delete;
	Platform(Platform&&)				 = delete;
	Platform& operator=(Platform&&)		 = delete;

	// types
	static const PlatformType UNDEFINED;

	// Static methods for retrieving records.
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getRecordByUid() instead.")]]
	static ContentModelRef getRecordByUid(const ContentModelRef& model, const std::string& uid);
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getRecordByUid() instead.")]]
	static ContentModelRef getRecordByUid(const ui::SpriteEngine& engine, const std::string& uid);

	bool					   isInitialized() const { return mInitialized; }
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getPlatformKey() instead.")]]
	virtual const std::string& getPlatformKey() const;
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getPlatformModel() instead.")]]
	virtual ContentModelRef	   getPlatformModel();
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getPlatformType() instead.")]]
	virtual PlatformType	   getPlatformType() const;
	[[deprecated("Use of the Platform class is discouraged. Please use ContentHelper::getCurrentContent() instead.")]]
	virtual ContentModelRef	   getCurrentContent() const;

	virtual void			   refreshContent();
	virtual void			   setupContentListener();


  protected:
	std::string		  mPlatformKey;
	PlatformType	  mPlatformType = UNDEFINED;
	ui::SpriteEngine& mEngine;
	EventClient		  mEventClient;
	ContentModelRef	  mPlatformModel;
	ContentModelRef	  mCurrentContent;
	ContentModelRef	  mEvents;

  private:
	bool mInitialized = false;
};


} // namespace ds::model

#endif