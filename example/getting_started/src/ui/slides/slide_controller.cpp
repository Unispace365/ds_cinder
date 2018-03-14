#include "stdafx.h"

#include "slide_controller.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/storage/directory_watcher.h>

#include <poco/Path.h>

#include "events/app_events.h"


namespace downstream {

SlideController::SlideController(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "slide_controller.xml")
	, mCurrentSlide(nullptr)
{

	listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) { setData(false); });
	listenToEvents<SlideForwardRequest>([this](const SlideForwardRequest& e) { goForward(); });
	listenToEvents<SlideBackRequest>([this](const SlideBackRequest& e) { goBack(); });
	listenToEvents<SlideSetRequest>([this](const SlideSetRequest& e) { setData(true); });

	setSwipeCallback([this](ds::ui::Sprite* bs, const ci::vec3& amount) {
		if(amount.x < 0.0) {
			mEngine.getNotifier().notify(SlideForwardRequest());
		} else {
			mEngine.getNotifier().notify(SlideBackRequest());
		}
	});

}

void SlideController::setData(const bool doAnimation, const bool forwards) {
	if(mCurrentSlide) {
		if(doAnimation) {
			auto cs = mCurrentSlide;
			auto dest = ci::vec3(-mEngine.getWorldWidth(), cs->getPosition().y, 0.0);
			if(!forwards) dest.x = -dest.x;
			cs->tweenPosition(dest, mEngine.getAnimDur(), 0.0f, ci::easeInQuad, [this, cs] { cs->release(); });
		} else {
			mCurrentSlide->release();
			mCurrentSlide = nullptr;
		}
	}
	auto holder = getSprite("slide_holder");
	if(!holder) {
		DS_LOG_WARNING("No slide holder found in SlideController!");
		return;
	}

	auto allSlides = mEngine.mContent.getChildByName("slides");
	int curSlide = mEngine.mContent.getPropertyInt("current_slide");
	auto thisSlide = allSlides.getChildById(curSlide);
	if(thisSlide.empty()) {
		DS_LOG_WARNING("Slide not found!");
		thisSlide = allSlides.getChild(0);
		mEngine.mContent.setProperty("current_slide", thisSlide.getId());
	}

	auto slideLayout = thisSlide.getPropertyString("layout");
	if(slideLayout.empty()) slideLayout = "slide.xml";

	mCurrentSlide = new ds::ui::SmartLayout(mEngine, slideLayout);
	holder->addChildPtr(mCurrentSlide);
	mCurrentSlide->setContentModel(thisSlide);

	if(mCurrentSlide->hasSprite("markdown") && !thisSlide.getPropertyString("markdown_file").empty()) {
		auto markdownFile = Poco::Path::expand(ds::Environment::expand(thisSlide.getPropertyString("markdown_file")));
		std::ifstream t(markdownFile.c_str());
		std::string str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
		ds::ui::XmlImporter::setSpriteProperty(*mCurrentSlide->getSprite("markdown"), "markdown", str);
	}

	setContentModel(thisSlide);

	if(hasSprite("count")) {
		for(int i = 0; i < allSlides.getChildren().size(); i++) {
			if(allSlides.getChildren()[i].getId() == curSlide) {
				setSpriteText("count", std::to_string(i + 1));
				break;
			}
		}
	}

	runLayout();

	if(doAnimation) {
		if(forwards) {
			mCurrentSlide->tweenAnimateOn(true, 0.0f, 0.05f);
		} else {
			auto befor = mCurrentSlide->getPosition();
			mCurrentSlide->setPosition(-mEngine.getWorldWidth(), befor.y);
			mCurrentSlide->tweenPosition(befor, mEngine.getAnimDur(), 0.0f, ci::easeOutQuad);
		}
	}

}


void SlideController::goForward() {
	auto allSlides = mEngine.mContent.getChildByName("slides");

	if(allSlides.getChildren().empty()) return;

	int curSlide = mEngine.mContent.getPropertyInt("current_slide");
	bool found = false;
	for (auto it : allSlides.getChildren()){
		if(found) {
			mEngine.mContent.setProperty("current_slide", it.getId());
			setData(true);
			return;
		}
		if(it.getId() == curSlide) found = true;
	}

	mEngine.mContent.setProperty("current_slide", allSlides.getChild(0).getId());
	setData(true);
	return;

}

void SlideController::goBack() {
	auto allSlides = mEngine.mContent.getChildByName("slides");

	if(allSlides.getChildren().empty()) return;

	int curSlide = mEngine.mContent.getPropertyInt("current_slide");
	int prevId = -1;

	for(auto it : allSlides.getChildren()) {
		if(it.getId() == curSlide && prevId > -1) {
			mEngine.mContent.setProperty("current_slide", prevId);
			setData(true, false);
			return;
		}
		prevId = it.getId();
	}

	mEngine.mContent.setProperty("current_slide", allSlides.getChildren().back().getId());
	setData(true, false);
	return;
}

} // namespace downstream

