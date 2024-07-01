#pragma once
#ifndef DS_CONTENT_PLATFORM
#define DS_CONTENT_PLATFORM

#include <ds/data/resource.h>
#include <vector>
#include "content_model.h"
#include "ds/ui/sprite/sprite_engine.h"
#include <ds/app/event_client.h>


namespace ds::model {

class Platform {
  public:
	Platform(ds::ui::SpriteEngine& engine, const std::string& platformKey);
	~Platform();
	static ds::model::ContentModelRef getRecordByUid(const ds::model::ContentModelRef& model, const std::string& uid);
	static ds::model::ContentModelRef getRecordByUid(const ds::ui::SpriteEngine& engine, const std::string& uid);
	std::string						  getPlatformKey();
	void							  refreshContent();
	ds::model::ContentModelRef		  getPlatform();
	ds::model::ContentModelRef		  getCurrentPlaylist();
	void 							  setCurrentPlaylist(std::string playlistKey);
	ds::model::ContentModelRef		  getPlatformProperty(const std::string& key);

  private:
	std::string					mPlatformKey;
	ds::ui::SpriteEngine&		mEngine;
	ds::EventClient				mEventClient;
	ds::model::ContentModelRef	mPlatform;
	ds::model::ContentModelRef	mCurrentPlaylist;
};

}

#endif