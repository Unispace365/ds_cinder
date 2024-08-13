#include "stdafx.h"

#include "recent_files_service.h"

#include <Poco/DateTimeFormatter.h>
#include <fstream>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/file_meta_data.h>

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {

RecentFilesService::RecentFilesService(ds::ui::SpriteEngine& eng)
	: mEngine(eng)
	, mEventClient(eng)
	, mSaveService(eng, []() { return new FileListSaveService(); }) {

	 mEventClient.listenToEvents<PresentationStatusUpdatedEvent>([this](auto& e) {
			 auto presId = mEngine.mContent.getChildByName("current_presentation").getPropertyInt("presentation_id");
			 auto presModel = mEngine.mContent.getChildByName("cms_root").getReference("valid_nodes", presId);
			 DS_LOG_INFO("Presentation " << presId << " updated")
			 //presModel.printTree(true);
			 addToRecentlyOpened(presModel);
			});

	mEventClient.listenToEvents<ViewerAddedEvent>([this](auto& e) {
		// auto model = e.mModel.duplicate();
		// model.printTree(true);
		addToRecentlyOpened(e.mModel);

		// write the thing

		auto recents = mEngine.mContent.getChildByName("recent_files");
		if (recents.getChildren().empty()) return;

		ci::XmlTree theDoc = ci::XmlTree::createDoc();
		ci::XmlTree rootThing;
		rootThing.setTag("recent_files");

		for (auto it : recents.getChildren()) {
			ci::XmlTree chillin = ci::XmlTree();
			chillin.setTag("media_res");

			chillin.setAttribute("type", it.getPropertyString("type"));
			chillin.setAttribute("last_opened", it.getPropertyString("last_opened"));
			chillin.setAttribute("name", it.getPropertyString("name"));
			chillin.setAttribute("body", it.getPropertyString("body"));

			if (it.getPropertyString("type") == MEDIA_TYPE_FILE_LOCAL) {
				chillin.setValue(it.getPropertyResource("media_res").getAbsoluteFilePath());
			} else {
				chillin.setAttribute("id", it.getId());
			}

			rootThing.push_back(chillin);
		}

		mSaveService.start([this, rootThing](FileListSaveService& q) { q.setFileList(rootThing); });
	});
}

void RecentFilesService::initialize() {

	auto recentFiles = ds::model::ContentModelRef("recent_files");

	std::string fileName = ds::Environment::expand("%LOCAL%/waffles-neu/recent_files.xml");
	if (!ds::safeFileExistsCheck(fileName)) return;

	cinder::XmlTree xml(cinder::loadFile(fileName));
	try {
		if (xml.hasChild("recent_files")) {
			ci::XmlTree recents = xml.getChild("recent_files");

			for (ci::XmlTree::Iter it = recents.begin(); it != recents.end(); ++it) {
				std::string type = it->getAttribute("type");
				if (type.empty()) continue;
				ds::model::ContentModelRef recentMed;
				recentMed.setProperty("name", it->getAttribute("name"));
				recentMed.setProperty("body", it->getAttribute("body"));
				recentMed.setProperty("last_opened", it->getAttribute("last_opened"));
				recentMed.setProperty("type", it->getAttribute("type"));
				if (recentMed.getPropertyString("type") == MEDIA_TYPE_FILE_LOCAL) {
					ds::Resource primaryMed = ds::Resource(it->getValue());
					recentMed.setPropertyResource("media_res", primaryMed);
				}else /*if (recentMed.getPropertyString("type") == MEDIA_TYPE_FILE_CMS)*/ {
					auto id = ds::string_to_int(it->getAttribute("id"));
					recentMed.setId(id);
				}
				recentFiles.addChild(recentMed);
			}
		}

		mEngine.mContent.replaceChild(recentFiles);

	} catch (std::exception& e) {
		DS_LOG_WARNING("Couldn't read the recent files! " << e.what());
	}
}

void RecentFilesService::addToRecentlyOpened(ds::model::ContentModelRef newMedia) {
	newMedia.setProperty("last_opened", Poco::DateTimeFormatter::format(Poco::LocalDateTime(), "%Y-%n-%d %H:%M:%S:%i"));
	bool found	 = false;
	auto recents = mEngine.mContent.getChildByName("recent_files");


	if (newMedia.getPropertyString("type") == MEDIA_TYPE_FILE_LOCAL) {

		std::string thisPath = newMedia.getPropertyResource("media_res").getAbsoluteFilePath();
		for (auto it : recents.getChildren()) {
			if (it.getPropertyResource("media_res").getAbsoluteFilePath() == thisPath) {
				found = true;
				it.setProperty("last_opened", newMedia.getPropertyString("last_opened"));
				break;
			}
		}
	} else { // if(newMedia.getPropertyString("type") == MEDIA_TYPE_FILE_CMS) {
		int thisId = newMedia.getId();
		for (auto it : recents.getChildren()) {
			if (it.getId() == thisId) {
				found = true;
				it.setProperty("last_opened", newMedia.getPropertyString("last_opened"));
				break;
			}
		}
	}

	if (!found) {
		if (!newMedia.getPropertyString("type")
				 .empty()) { // if(newMedia.getPropertyString("type") == MEDIA_TYPE_FILE_LOCAL ||
							 // newMedia.getPropertyString("type") == MEDIA_TYPE_FILE_CMS){
			recents.addChild(newMedia);
		}
	}

	std::vector<ds::model::ContentModelRef> recentsChildren = recents.getChildren();
	std::sort(recentsChildren.begin(), recentsChildren.end(), [this](auto& a, auto& b) {
		if (a.getPropertyString("last_opened") < b.getPropertyString("last_opened")) return true;
		return false;
	});

	while (recentsChildren.size() > 200) {
		recentsChildren.erase(recentsChildren.begin());
	}
	recents.setChildren(recentsChildren);

	mEngine.mContent.replaceChild(recents);
}

} // namespace waffles
