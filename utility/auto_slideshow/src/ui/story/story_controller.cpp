#include "stdafx.h"

#include "story_controller.h"
#include "story_view.h"

#include <poco/File.h>
#include <poco/Path.h>

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>

#include "events/app_events.h"


namespace downstream {

StoryController::StoryController(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_controller.xml")
{

	try {
		std::vector<Poco::File> files;
		auto pathy = ds::Environment::expand("%LOCAL%/photos/");
		Poco::File theDir = Poco::File(pathy);
		theDir.list(files);

		for(auto it : files) {
			if(!it.isFile()) continue;
			auto video = ds::model::ContentModelRef("photo");

			Poco::Path thePath = it.path();
			auto theName = thePath.getFileName();
			theName = theName.substr(0, theName.size() - thePath.getExtension().size() - 1);
			video.setProperty("name", theName);
			video.setProperty("path", thePath.toString());
			video.setProperty("type", std::string("image"));
			mPhotoz.emplace_back(video);
		}
	} catch(std::exception& e) {
		DS_LOG_WARNING("Exception scanning files: " << e.what());
	}

	std::srand(Poco::Timestamp().epochMicroseconds());
	std::random_shuffle(mPhotoz.begin(), mPhotoz.end());

	rotatePhoto();
}

void StoryController::rotatePhoto() {

	mCurrentPhotoIndex++;
	if(mCurrentPhotoIndex > mPhotoz.size() - 1) {
		mCurrentPhotoIndex = 0;
		std::random_shuffle(mPhotoz.begin(), mPhotoz.end());
	}

	mOnA = !mOnA;
	if(mOnA) {
		removePhoto(mStoryB);
		mStoryA = new StoryView(mEngine);
		mStoryA->setContentModel(mPhotoz[mCurrentPhotoIndex]);
		addChildPtr(mStoryA);
		mStoryA->tweenAnimateOn(true, 0.0f, 0.05f);
	} else {
		removePhoto(mStoryA);
		mStoryB = new StoryView(mEngine);
		mStoryB->setContentModel(mPhotoz[mCurrentPhotoIndex]);
		addChildPtr(mStoryB);
		mStoryB->tweenAnimateOn(true, 0.0f, 0.05f);
	}

	callAfterDelay([this] { rotatePhoto(); }, mEngine.getAppSettings().getDouble("photo:duration", 0, 5.0));
}


void StoryController::removePhoto(StoryView* sv) {
	if(sv) {
		/// The animate off script has to finish or this sprite will leak, so be careful not to animate that sprite again
		auto cs = sv;
		cs->tweenAnimateOff(true, 0.0f, 0.02f, [cs] { cs->release(); });
	}
}

} // namespace downstream

