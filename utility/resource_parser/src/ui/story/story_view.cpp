#include "stdafx.h"

#include "story_view.h"

#include <Poco/Path.h>
#include <Poco/File.h>

#include <ds/query/query_client.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/image_meta_data.h>
#include "events/app_events.h"


namespace downstream {

StoryView::StoryView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_view.xml")
	, mResourcesId(1)
{

	hide();
	setOpacity(0.0f);
		
	listenToEvents<ds::app::IdleStartedEvent>([this](const ds::app::IdleStartedEvent& e) { animateOff(); });
	listenToEvents<ds::app::IdleEndedEvent>([this](const ds::app::IdleEndedEvent& e) { animateOn(); });
	listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) { setData(); });


	auto deleteResources = getSprite<ds::ui::SpriteButton>("delete_buttton.the_button");
	if(deleteResources) {
		deleteResources->setClickFn([this] {
			if(!mDbUrl.empty()) {
				DS_LOG_INFO("Deleting all resources");
				ds::query::Result qr;
				ds::query::Client::queryWrite(mDbUrl, "DELETE FROM resources", qr);
			}
		});
	}

	auto openButton = getSprite<ds::ui::SpriteButton>("open_button.the_button");
	if(openButton) {
		openButton->setClickFn([this] {
			ci::fs::path path = ci::app::getOpenFilePath(ds::Environment::expand("%LOCAL%"));
			mDbUrl = path.string();


			Poco::Path thePath = Poco::Path(path.string());
			thePath.makeParent();
			thePath.makeParent();
			mResourcesRoot = ds::Environment::expand(thePath.toString());

			setSpriteText("scan_dir", "Current resources directory: " + mResourcesRoot);
		});
	}

	auto runButton = getSprite<ds::ui::SpriteButton>("run_button.the_button");
	if(runButton) {
		runButton->setClickFn([this] {
			if(!mResourcesRoot.empty()) {

				ds::query::Result qr;
				//ds::query::Client::queryWrite(mDbUrl, "DELETE FROM resources", qr);
				mResourcesId = 1;

				if(ds::query::Client::query(mDbUrl, "select resourcesid FROM resources ORDER BY resourcesid DESC LIMIT 1", qr) && !qr.rowsAreEmpty()) {
					ds::query::Result::RowIterator it(qr);
					while(it.hasValue()) {
						mResourcesId = it.getInt(0) + 1;
						++it;
					}
				}

				mCurrentResources.clear();

				if(ds::query::Client::query(mDbUrl, "select resourcespath, resourcesfilename FROM resources", qr) && !qr.rowsAreEmpty()) {
					ds::query::Result::RowIterator it(qr);
					while(it.hasValue()) {
						Poco::Path thePath = Poco::Path(mResourcesRoot);
						thePath.append(it.getString(0));
						thePath.append(it.getString(1));
						mCurrentResources.emplace_back(ds::Environment::expand(thePath.toString()));
						++it;
					}
				}


				mNumError = 0;
				mNumInvalid = 0;
				mNumSkipped = 0;
				mNumUpdated = 0;

				setSpriteText("output", "Scanning...");
				callAfterDelay([this] {
					parseDirectoryRecursive(Poco::File(mResourcesRoot));

					std::stringstream ss;
					ss << "Finished. " << std::endl;
					ss << "Updated: " << mNumUpdated << std::endl;
					ss << "Skipped / existing: " << mNumSkipped << std::endl;
					ss << "Invalid file types: " << mNumInvalid << std::endl;
					ss << "Error writing sql: " << mNumError;
					setSpriteText("output", ss.str());
				}, 0.2f);
			}
		});
	}
}


void StoryView::setData() {

	completeAllTweens(false, true);
	clearAnimateOnTargets(true);
	runLayout();
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::parseDirectoryRecursive(Poco::File thisFile) {
	std::vector<Poco::File> files;
	DS_LOG_INFO("Parsing directory: " << thisFile.path());
	ds::query::Result qr;
	thisFile.list(files);
	for(auto it : files) {

		try {
			if(it.isHidden()) {
				DS_LOG_INFO("Skipped file because it's hidden " << it.path());
				mNumInvalid++;
				continue;
			}
			if(it.isDirectory()) {
				parseDirectoryRecursive(it);
			}
			if(it.isFile()) {
				Poco::Path pathy = it.path();
				std::string pathString = pathy.toString();
				std::string normalPath = ds::Environment::expand(pathString);
				DS_LOG_VERBOSE(1, "Parsing file: " << pathString);
				std::string fileName = pathy.getFileName();
				std::string theExtension = pathy.getExtension();
				ds::Resource theResource = ds::Resource(pathString);

				auto findy = std::find(mCurrentResources.begin(), mCurrentResources.end(), normalPath);
				if(findy != mCurrentResources.end()) {
					DS_LOG_VERBOSE(1, "Current resource already exists in the thing, skipping! " << normalPath);
					mNumSkipped++;
					continue;
				}
				
				// todo: pdf size and page number
				// todo: video size and duration
				// todo: delete from list if the file doesn't exist anymore
				if(theResource.getType() == ds::Resource::IMAGE_TYPE) {
					ds::ImageMetaData d(pathString);
					if(!d.empty()) {
						theResource.setWidth(d.mSize.x);
						theResource.setHeight(d.mSize.y);
					}

				}

				if(theResource.getType() != ds::Resource::ERROR_TYPE) {

					auto now = Poco::Timestamp().epochMicroseconds();

					pathString = ds::Environment::expand(pathString);
					pathString = pathString.substr(mResourcesRoot.size());
					pathString = pathString.substr(0, pathString.size() - fileName.size());
					std::string theInsert = "INSERT INTO Resources (resourcesid, resourcestype, resourcesfilename, resourcespath, resourceswidth, resourcesheight, resourceshash, media_id, file_id) VALUES ("
						+ std::to_string(mResourcesId)
						+ ", '" + theResource.getTypeChar()
						+ "', '" + fileName
						+ "', '" + pathString
						+ "', " + std::to_string(theResource.getWidth())
						+ ", " + std::to_string(theResource.getHeight())
						+ ", 'local'"
						+ ", " + std::to_string(mResourcesId)
						+ ", " + std::to_string(mResourcesId)
						+ ")";
					mResourcesId++;
					if(ds::query::Client::queryWrite(mDbUrl, theInsert, qr)) {
						std::cout << theInsert << std::endl;
						mNumUpdated++;
						DS_LOG_INFO( "Wrote a resource! " << theResource.getAbsoluteFilePath());
					} else {
						mNumError++;
						DS_LOG_WARNING("Didn't write a resource! " << theResource.getAbsoluteFilePath());
					}
				} else {
					DS_LOG_INFO("Skipped file because it's not a supported type " << pathString);
					mNumInvalid++;
				}
			}

		} catch(std::exception& e) {
			std::cout << "oops " << e.what() << std::endl;
		}
	}
}

void StoryView::animateOn(){
	show();
	tweenOpacity(1.0f, mEngine.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace downstream

