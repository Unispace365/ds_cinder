#include "stdafx.h"

#include "query_handler.h"

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/content/content_events.h>
#include <ds/debug/logger.h>
#include "events/app_events.h"

namespace downstream {

QueryHandler::QueryHandler(ds::ui::SpriteEngine& eng)
	: mEngine(eng)
	, mEventClient(eng)
{

	mComputerName = getenv("COMPUTERNAME");
	DS_LOG_INFO("Computer name: " << mComputerName);


	/* TEMP? adding streams from settings - probably should re-implement
	int i = 0;
	int idy = 10000000;
	while (true) {
	if (!createTempStreamModel("temp:stream_" + std::to_string(i), idy + i, allNodesList)) {
	break;
	}
	i++;
	}
	*/


	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](auto& e) {


		// ----------- PLATFORMS --------------------------------------
		ds::model::ContentModelRef cmsPlatforms = ds::model::ContentModelRef("cms_platforms");
		cmsPlatforms.setProperty("kind", std::string("platform"));
		cmsPlatforms.setProperty("name", std::string("Platforms"));

		// add properties to the platforms
		std::vector<ds::model::ContentModelRef> allPlatformsList = mEngine.mContent.getChildByName("sqlite.waffles_platforms").getChildren();
		for (auto it : allPlatformsList) {
			parseModelProperties(it);
		}

		/// filter out all platforms to this computer name
		/// Note, you could change mComputerName to something from app settings or other value to filter in other ways
		ds::model::ContentModelRef thisPlatform;
		for (auto it : allPlatformsList) {
			if (it.getPropertyString("platform_key") == mComputerName) {
				thisPlatform = it;
			}
		}

		if (thisPlatform.empty()) {
			DS_LOG_WARNING("No specific platform for this app. Check the computer name " << mComputerName << " against the CMS");
		}

		cmsPlatforms.clearChildren();
		cmsPlatforms.addChild(thisPlatform);
		mEngine.mContent.replaceChild(cmsPlatforms);


		// ----------- Events --------------------------------------
		ds::model::ContentModelRef cmsEvents = ds::model::ContentModelRef("cms_events");
		cmsEvents.setProperty("kind", std::string("event"));
		cmsEvents.setProperty("name", std::string("Events"));

		// add properties to the platforms
		std::vector<ds::model::ContentModelRef> allEventsList = mEngine.mContent.getChildByName("sqlite.waffles_events").getChildren();
		for (auto it : allEventsList) {
			parseModelProperties(it);
			cmsEvents.addChild(it);
		}

		//cmsEvents.printTree(true);

		mEngine.mContent.replaceChild(cmsEvents);

		// ----------- NODES --------------------------------------
		// The main waffles nodes, aka the primary content
		ds::model::ContentModelRef cmsRoot = ds::model::ContentModelRef("cms_root");
		cmsRoot.setProperty("kind", std::string("node"));
		cmsRoot.setProperty("name", std::string("Nodes"));

		std::map<int, ds::model::ContentModelRef> allNodes;
		std::vector<ds::model::ContentModelRef> allNodesList = mEngine.mContent.getChildByName("sqlite.waffles_nodes").getChildren();

		for (auto it : allNodesList) {
			parseModelProperties(it);

			if (it.getPropertyString("type") == "media" && it.getPropertyResource("media_res").empty()) {
				DS_LOG_WARNING("No media detected on node " << it.getId() << " " << it.getPropertyString("name"));
				continue;
			}

			allNodes[it.getId()] = it;
		}

		//set up the node tree
		for (auto it : allNodesList) {
			auto parentId = it.getPropertyInt("parent_id");
			if (parentId == 0) {
				cmsRoot.addChild(it);
			} else {
				//if parentId does not exist as a key 
				//(in the instance the parent is disabled and the children arent) 
				//this adds a blank contentModelRef which gets deleted later
				allNodes[parentId].addChild(it);
			}
		}

		//make sure no one can use the old list of nodes
		mEngine.mContent.getChildByName("sqlite.waffles_nodes").clearChildren();

		// add valid nodes as references on the root node for lookup speed
		std::map<int, ds::model::ContentModelRef> allValidNodes;
		addReference(cmsRoot, allValidNodes);
		cmsRoot.setReferences("valid_nodes", allValidNodes);

		//add the new tree of nodes
		mEngine.mContent.replaceChild(cmsRoot);


		//let everyone know we updated the tree of nodes
		mEventClient.notify(CmsDataLoadCompleteEvent());
	});
}

void QueryHandler::addReference(ds::model::ContentModelRef curParent, std::map<int, ds::model::ContentModelRef>& overallMap) {
	for (auto it : curParent.getChildren()) {
		overallMap[it.getId()] = it;
		addReference(it, overallMap);
	}
}

void QueryHandler::parseModelProperties(ds::model::ContentModelRef& it) {
	//add all fields as properties of the node
	for (auto props : it.getChildren()) {
		auto propName = props.getName();
		if (propName == "waffles_text_fields"
			|| propName == "waffles_checkboxes"
			|| propName == "waffles_color"
			) {
			it.setProperty(props.getPropertyString("app_key"), props.getPropertyString("value"));
		} else if (propName == "waffles_media_fields") {
			it.setProperty(props.getPropertyString("app_key") + "_media_res", props.getPropertyString("media_res"));
			it.setPropertyResource(props.getPropertyString("app_key") + "_media_res", processResource(props.getPropertyResource("media_res")));
			it.setProperty(props.getPropertyString("app_key") + "_thumb_res", props.getPropertyString("media_thumb_res"));
			it.setPropertyResource(props.getPropertyString("app_key") + "_thumb_res", processResource(props.getPropertyResource("media_thumb_res")));
			if (props.getPropertyString("app_key") == "media") {
				it.setPropertyResource("media_res", processResource(props.getPropertyResource("media_res"))); // map a primary "media" to be "media_res"
			}
			if (props.getPropertyBool("floatable")) {
				it.setProperty(props.getPropertyString("app_key") + "_float_fullscreen", props.getPropertyString("float_fullscreen"));
				it.setProperty(props.getPropertyString("app_key") + "_float_autoplay", props.getPropertyString("float_autoplay"));
				it.setProperty(props.getPropertyString("app_key") + "_float_loop", props.getPropertyString("float_loop"));
				it.setProperty(props.getPropertyString("app_key") + "_float_volume", props.getPropertyString("float_volume"));
				it.setProperty(props.getPropertyString("app_key") + "_float_touch", props.getPropertyString("float_touch"));
				it.setProperty(props.getPropertyString("app_key") + "_float_page", props.getPropertyString("float_page"));
				it.setProperty(props.getPropertyString("l,app_key") + "_floatable", props.getPropertyString("floatable"));
				it.setProperty(props.getPropertyString("app_key") + "_hide_thumbnail", props.getPropertyString("hide_thumbnail"));
			}
		} else if (propName == "waffles_dropdown_fields") {
			if (props.getPropertyBool("allow_multiple")) {
				it.addPropertyToList(props.getPropertyString("app_key"), props.getPropertyString("option_save_value"));
			} else {
				it.setProperty(props.getPropertyString("app_key"), props.getPropertyString("option_save_value"));
				it.setProperty(props.getPropertyString("app_key") + "_dropdown_label", props.getPropertyString("dropdown_label"));
			}

		} else if (propName == "waffles_selections") {
			if (props.getPropertyBool("allow_multiple")) {
				it.addPropertyToList(props.getPropertyString("app_key"), props.getPropertyString("node_id"));
			} else {
				it.setProperty(props.getPropertyString("app_key"), props.getPropertyString("node_id"));
			}
		} else if (propName == "waffles_hotspots") {
			it.setProperty(props.getPropertyString("app_key") + "_shape", props.getPropertyString("shape"));
			it.setProperty(props.getPropertyString("app_key") + "_pos_x", props.getPropertyString("pos_x"));
			it.setProperty(props.getPropertyString("app_key") + "_pos_y", props.getPropertyString("pos_y"));
			it.setProperty(props.getPropertyString("app_key") + "_pos_w", props.getPropertyString("pos_w"));
			it.setProperty(props.getPropertyString("app_key") + "_pos_h", props.getPropertyString("pos_h"));
		} else if (propName == "waffles_composites") {
			it.setProperty("composite_pos_x", props.getPropertyString("pos_x")); // todo: handle multiple composites?
			it.setProperty("composite_pos_y", props.getPropertyString("pos_y"));
			it.setProperty("composite_pos_w", props.getPropertyString("pos_w"));
		} else if (propName == "waffles_composite_details") {
			it.setProperty(props.getPropertyString("app_key") + "_preview_res", props.getPropertyString("preview_res"));
			it.setPropertyResource(props.getPropertyString("app_key") + "_preview_res", processResource(props.getPropertyResource("preview_res")));
			it.setProperty(props.getPropertyString("app_key") + "_preview_thumb_res", props.getPropertyString("preview_thumb_res"));
			it.setPropertyResource(props.getPropertyString("app_key") + "_preview_thumb_res", processResource(props.getPropertyResource("preview_thumb_res")));
		} else if (propName == "waffles_tags") {
			it.addPropertyToList(props.getPropertyString("taggable_field"), props.getPropertyString("title"));
		} else if (propName == "waffles_streamconfs") {
			auto theLocation = props.getPropertyString("location");
			if (!theLocation.empty()) {
				auto theResouce = props.getPropertyResource("stream_res");
				theResouce.setFileName(props.getPropertyString("media_title"));
				theResouce.setLocalFilePath(theLocation);
				it.setPropertyResource(props.getPropertyString("resourcesfilename"), theResouce);
			}
		}

	}

	it.clearChildren();
}

bool QueryHandler::createTempStreamModel(const std::string& settingsName, const int fakeId, std::vector<ds::model::ContentModelRef>& streamList) {
	auto theUrl = mEngine.getAppSettings().getString(settingsName, 0, "");
	auto theName = mEngine.getAppSettings().getString(settingsName + "_name", 0, "");
	if (theUrl.empty() || theName.empty()) return false;
	ds::Resource fakeStreamRes = ds::Resource(theUrl);
	ds::model::ContentModelRef fakeStreamOne = ds::model::ContentModelRef("waffles_nodes", fakeId);
	fakeStreamRes.setWidth(1920.0f);
	fakeStreamRes.setHeight(1080.0f);
	fakeStreamOne.setPropertyResource("media_res", fakeStreamRes);
	fakeStreamOne.setProperty("kind", "media");
	fakeStreamOne.setProperty("name", theName);
	streamList.emplace_back(fakeStreamOne);

	return true;
}

ds::Resource QueryHandler::processResource(ds::Resource input) {
	auto output = input;
	if (input.getType() == ds::Resource::WEB_TYPE
		&& input.getFileName().find("/youtube/") != std::string::npos
		) {

		std::string theFilename = input.getFileName();
		auto yt_loc = theFilename.find("/youtube/");
		theFilename = theFilename.substr(yt_loc + 9);
		output.setFileName(theFilename);
		output.setType(ds::Resource::YOUTUBE_TYPE);
	} else if (input.getType() == ds::Resource::VIDEO_STREAM_TYPE) {

		auto allPlatforms = mEngine.mContent.getChildByName("cms_platforms");
		if (allPlatforms.hasChildren()) {
			// the input's filename will be the app key of the platform's stream resource, if it has a stream configuration
			return allPlatforms.getChild(0).getPropertyResource(input.getFileName());
		}

		return ds::Resource();

	}

	return output;
}

} 

