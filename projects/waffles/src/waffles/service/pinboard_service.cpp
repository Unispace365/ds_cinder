#include "stdafx.h"

#include "pinboard_service.h"

#include <cinder/Json.h>

#include <Poco/DateTimeFormatter.h>
#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include "waffles/waffles_events.h"


namespace waffles {

PinboardService::PinboardService(ds::ui::SpriteEngine& g)
	: mEngine(g)
	, mEventClient(g)
	, mCreateRequest(g)
	, mSetMediaRequest(g)
	, mPurgeRequest(g) {

	// mNoteRequest.setVerboseOutput(true);

	mEventClient.listenToEvents<RequestPinboardSave>([this](auto& e) { saveToPinboard(e.mContentModel, e.mIsAdd); });

	/* mEventClient.listenToEvents<RequestNoteSave>(
		[this](auto& e) { saveNote(e.mTheNote, e.mAlsoToPinboard, e.mEditingNode); }); */
}

void PinboardService::saveToPinboard(ds::model::ContentModelRef model, const bool isAdd) {
	if (isAdd) {
		addToPinboard(model);
	} else {
		removeFromPinboard(model);
	}
}

void PinboardService::addToPinboard(ds::model::ContentModelRef model) {
	auto helper = WafflesHelperFactory::getDefault();
	auto pinboard	 = helper->getPinboard();
	auto pinboardUid = pinboard.getPropertyString("uid");

	auto typeKey  = model.getPropertyString("type_key");
	auto uid	  = model.getPropertyString("uid");
	auto fieldUid = model.getPropertyString("media_field_uid");

	// NOTE: Hardcoded slot_uids here! Should be replaced with an appsetting or something
	std::string thePayload =
		R"JSON( {"type": "JfDgLbj9vxT8","parent": {"variant": "RECORD","record": "%RECORD%","slot": "R2zDOgfpFydV"},"name": "%NAME%"} )JSON";

	ds::replace(thePayload, "%RECORD%", pinboardUid);
	ds::replace(thePayload, "%NAME%", model.getPropertyString("record_name"));

	mCreateRequest.setReplyFunction([this, uid, fieldUid, model](const bool erroed, const std::string& reply,
																 long httpCode) {
		if (httpCode == 200 && !erroed) {
			std::string toRecordUid;
			auto		replyJson = ci::JsonTree(reply);
			if (replyJson.hasChild("data") && replyJson.getChild("data").hasChild("id")) {
				toRecordUid = replyJson.getChild("data").getChild("id").getValue();
			}
			mSetMediaRequest.setReplyFunction(
				[this, model](const bool erroed, const std::string& reply, long httpCode) {
					DS_LOG_VERBOSE(1, "SetMedia: " << reply);
					if (httpCode != 200 || erroed) {
						DS_LOG_WARNING("Error when trying to setMedia for pinboard child record!" << reply);
						mEngine.getNotifier().notify(PinboardSaveComplete(model, true));
					} else {
						mEngine.getNotifier().notify(PinboardSaveComplete(model, false));
					}
				});

			std::string setMediaPayload =
				R"JSON( {"from":{"field":"%FROMFIELD%","record":"%FROMRECORD%"},"to":{"field":"%TOFIELD%","record":"%TORECORD%"}} )JSON";
			ds::replace(setMediaPayload, "%FROMFIELD%", fieldUid);
			ds::replace(setMediaPayload, "%FROMRECORD%", uid);
			ds::replace(setMediaPayload, "%TOFIELD%", "S2ex3AuxOtS3");
			ds::replace(setMediaPayload, "%TORECORD%", toRecordUid);

			auto endpoint = mEngine.getCmsURL() + "/editing/value/copy";
			mSetMediaRequest.makePostRequest(endpoint, setMediaPayload, false, false, "POST", getHeaders());
		} else {
			DS_LOG_WARNING(reply);
			mEngine.getNotifier().notify(PinboardSaveComplete(model, false));
		}
	});

	// auto endpoint = "https://app.tetrapak.bridge.downstream.com/editing/record/create";
	auto endpoint = mEngine.getCmsURL() + "/editing/record/create";
	mCreateRequest.makePostRequest(endpoint, thePayload, false, false, "POST", getHeaders());
}

void PinboardService::removeFromPinboard(ds::model::ContentModelRef model) {
	auto helper = WafflesHelperFactory::getDefault();
	auto pinboard	 = helper->getPinboard();
	auto pinboardUid = pinboard.getPropertyString("uid");

	for (auto pinboardItem : pinboard.getChildren()) {
		if (pinboardItem.getPropertyResource("media").getAbsoluteFilePath() ==
			model.getPropertyResource("media").getAbsoluteFilePath()) {
			auto		purgeId	   = pinboardItem.getPropertyString("uid");
			std::string thePayload = R"JSON({"record":"%PURGEID%"})JSON";
			ds::replace(thePayload, "%PURGEID%", purgeId);

			mPurgeRequest.setReplyFunction([this, model](const bool erroed, const std::string& reply, long httpCode) {
				DS_LOG_VERBOSE(1, "Purge: " << reply);
				if (httpCode != 200 || erroed) {
					DS_LOG_WARNING("Error when trying to purge pinboard record!" << reply);
					mEngine.getNotifier().notify(PinboardSaveComplete(model, true));
				} else {
					mEngine.getNotifier().notify(PinboardSaveComplete(model, false));
				}
			});

			auto endpoint = mEngine.getCmsURL() + "/editing/record/delete";
			mPurgeRequest.makePostRequest(endpoint, thePayload, false, false, "POST", getHeaders());
			break;
		}
	}
}

std::vector<std::string> PinboardService::getHeaders() {
	std::string authHash = mEngine.mContent.getChildByName("server").getPropertyString("auth");
	if (authHash.empty()) {
		authHash = mEngine.getAppSettings().getString("cms:auth_hash", 0, "");
	}
	std::vector<std::string> headers;
	headers.emplace_back("Authorization: Bearer " + authHash);
	headers.emplace_back("Content-Type: application/json");
	headers.emplace_back("Accept: application/json");
	return headers;
}

/* std::string PinboardService::getTextPayload(const std::string defId, const int nodeId, const std::string& theText) {
	auto theNote = theText;
	// escape backslashes with a double backslash
	ds::replace(theNote, "\\", "\\\\");
	// escape double quotes, replace them with \"
	ds::replace(theNote, "\"", "\\\"");

	std::stringstream endLine;
	endLine << std::endl;
	ds::replace(theNote, endLine.str(), "\\r\\n");

	std::string thePayload =
		"{\"definitionId\": " + defId + ", \"nodeId\": " + std::to_string(nodeId) + ", \"value\":{ ";
	thePayload.append("\"text\":\"" + theNote + "\",");
	thePayload.append("\"internal\":\"" + theNote + "\",");
	thePayload.append("\"plain\":\"" + theNote + "\"");
	thePayload.append("}}");
	return thePayload;
}

void PinboardService::saveNote(const std::string& theText, const bool alsoToPinboard, const int nodeId) {
	// if the node exists already, just update the text up there
	if (nodeId > 0) {

		// save the note text
		auto		defId  = mEngine.mContent.getPropertyString("notetaking_definition_id");
		std::string cmsUrl = mEngine.getCmsURL();
		cmsUrl.append("api/waffles/content/field/" + defId);

		auto thePayload = getTextPayload(defId, nodeId, theText);

		DS_LOG_VERBOSE(1, "Sending note text request to " << cmsUrl << " " << thePayload);

		mNoteRequest.setReplyFunction(
			[this, alsoToPinboard, nodeId, theText](const bool erroed, const std::string& reply, long httpCode) {
				DS_LOG_VERBOSE(1, "Note text response, errored=" << erroed << " http status=" << httpCode
																 << " reply: " << reply);
				if (erroed || httpCode != 200) {
					DS_LOG_WARNING("Note text response error! " << httpCode << " " << reply);
				}
				mEngine.getNotifier().notify(NoteSaveComplete(theText, nodeId));
				if (alsoToPinboard) {
					saveToPinboard(nodeId, true);
				}
			});

		mNoteRequest.makePostRequest(cmsUrl, thePayload, false, false, "PATCH", getHeaders());

	} else {

		auto currentEvents = mEngine.mContent.getChildByName("current_events");
		if (currentEvents.empty() || currentEvents.getChildren().empty()) {
			DS_LOG_WARNING("couldn't save a note to the cms because there's no current event");
			// we send out this event to finish the spinning thing on pinboard buttons
			// normally, the request would hit the CMS and in the reply, it'd send this out, and also after the round
			// trip from dsnode
			mEngine.getNotifier().notify(CmsDataLoadCompleteEvent());
			return;
		}

		ds::model::ContentModelRef mainEvent = currentEvents.getChild(0);

		std::string cmsUrl = mEngine.getCmsURL();
		cmsUrl.append("api/waffles/node/store/basic");

		std::string thePayload = "{\"kindKey\": \"note\", \"parentId\": " + std::to_string(mainEvent.getId()) + "}";

		DS_LOG_VERBOSE(1, "Creating a note on event id " << mainEvent.getId());

		mNoteRequest.setReplyFunction(
			[this, theText, alsoToPinboard](const bool erroed, const std::string& reply, long httpCode) {
				DS_LOG_VERBOSE(1, "Note created response, errored=" << erroed << " http status=" << httpCode
																	<< " reply: " << reply);
				if (erroed) {
					DS_LOG_WARNING("Note response error! " << httpCode << " " << reply);
				}

				auto theJsonResp = ci::JsonTree(reply);
				if (theJsonResp.hasChild("data.node.id")) {
					auto theId = ds::string_to_int(theJsonResp.getChild("data.node.id").getValue());
					if (theId < 1) {
						DS_LOG_WARNING("Invalid node id in note creation request! " << theId);
						return;
					} else {
						saveNote(theText, alsoToPinboard, theId);

						/// rename the note to something more descriptive
						std::string cmsUrl = mEngine.getCmsURL();
						cmsUrl.append("api/waffles/content/rename");
						auto noteName = mEngine.getAppSettings().getString("note:save_name", 0, "Note - ");
						auto dateFormat =
							mEngine.getAppSettings().getString("drawings:date_time_format", 0, "%B %e, %Y %h:%M %a");

						Poco::LocalDateTime ldt;
						noteName.append(Poco::DateTimeFormatter().format(ldt, dateFormat));
						std::string payload = "{\"name\":\"" + noteName + "\", \"nodeId\":" + std::to_string(theId) +
											  ", \"_method\":\"PATCH\"}";
						mNoteRenameRequest.makePostRequest(cmsUrl, payload, false, false, "PATCH", getHeaders());
					}
				}
			});

		mNoteRequest.makePostRequest(cmsUrl, thePayload, false, false, "POST", getHeaders());
	}
} */


} // namespace waffles
