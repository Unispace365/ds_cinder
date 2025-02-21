#pragma once
#ifndef DS_CONTENT_PLATFORM
#define DS_CONTENT_PLATFORM

#include "content_model.h"

#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds::model {

using PlatformType = std::string;

class Platform {
  public:
	Platform(ui::SpriteEngine& engine, const std::string& platformKey = "");
	virtual ~Platform() = default;

	Platform(const Platform&)			 = delete;
	Platform& operator=(const Platform&) = delete;
	Platform(Platform&&)				 = delete;
	Platform& operator=(Platform&&)		 = delete;

	// types
	static const PlatformType UNDEFINED;


	// Static methods for retrieving records.
	// These should be deprecated. *DO NOT USE*
	[[deprecated]] static ContentModelRef getRecordByUid(const ContentModelRef& model, const std::string& uid);
	[[deprecated]] static ContentModelRef getRecordByUid(const ui::SpriteEngine& engine, const std::string& uid);
	/**--**/

	virtual void			   refreshContent();
	bool					   isInitialized() const { return mInitialized; }
	virtual const std::string& getPlatformKey() const;
	virtual ContentModelRef	   getPlatformModel();
	virtual PlatformType	   getPlatformType() const;
	virtual ContentModelRef	   getCurrentContent() const;
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