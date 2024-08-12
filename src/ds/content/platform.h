#pragma once
#ifndef DS_CONTENT_PLATFORM
#define DS_CONTENT_PLATFORM

#include <ds/data/resource.h>
#include <vector>
#include "content_model.h"
#include "ds/ui/sprite/sprite_engine.h"
#include <ds/app/event_client.h>


namespace ds::model {

typedef std::string PlatformType;

class Platform {
  public:
	Platform(ds::ui::SpriteEngine& engine, const std::string& platformKey = "");
	~Platform();

	//types
	static const PlatformType UNDEFINED;
	
	
	// Static methods for retrieving records.
	// These should be deprecated. *DO NOT USE*
	static ds::model::ContentModelRef getRecordByUid(const ds::model::ContentModelRef& model, const std::string& uid);
	static ds::model::ContentModelRef getRecordByUid(const ds::ui::SpriteEngine& engine, const std::string& uid);
	/**--**/
	
	virtual void					  refreshContent();
	bool							  isInitialized() { return mInitialized; }
	virtual std::string						  getPlatformKey();
	virtual ds::model::ContentModelRef		  getPlatformModel();
	virtual PlatformType					  getPlatformType();

  protected:
	std::string					mPlatformKey;
	PlatformType				mPlatformType=UNDEFINED;
	ds::ui::SpriteEngine&		mEngine;
	ds::EventClient				mEventClient;
	ds::model::ContentModelRef	mPlatformModel;
	ds::model::ContentModelRef	mEvents;
  private:
	bool mInitialized = false;
};



}

#endif