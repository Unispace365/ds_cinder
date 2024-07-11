#pragma once

#include <ds/app/event_client.h>
#include <ds/network/https_client.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

/**
 * \class tcs::PinboardService
 *			 Communicates to the CMS when an item should be added or removed from the pinboard
 */
class PinboardService {
  public:
	PinboardService(ds::ui::SpriteEngine&);

  private:
	void		saveToPinboard(ds::model::ContentModelRef model, const bool isAdd);

	void		addToPinboard(ds::model::ContentModelRef model);
	void		removeFromPinboard(ds::model::ContentModelRef model);

	std::vector<std::string> getHeaders();

	ds::ui::SpriteEngine& mEngine;
	ds::EventClient		  mEventClient;

	ds::net::HttpsRequest mCreateRequest;
	ds::net::HttpsRequest mSetMediaRequest;

	ds::net::HttpsRequest mPurgeRequest;
	// ds::net::HttpsRequest mNoteRequest;
	// ds::net::HttpsRequest mNoteRenameRequest;
};

} // namespace waffles
