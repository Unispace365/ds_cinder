#include "viewer_controller.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ui/viewers/titled_media_viewer.h"

namespace mv {

ViewerController::ViewerController(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{
}

void ViewerController::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestMediaOpenEvent::WHAT()){
		const RequestMediaOpenEvent& e((const RequestMediaOpenEvent&)in_e);
		addViewer(e.mMedia, e.mLocation, e.mStartWidth);
	} else if(in_e.mWhat == RequestCloseAllEvent::WHAT()){
		const float deltaAnim = mGlobals.getAnimDur() / (float)mViewers.size();
		float delayey = 0.0f;
		for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
			animateViewerOff((*it), delayey);
			delayey += deltaAnim;
		}
	} else if(in_e.mWhat == RequestLayoutEvent::WHAT()){
		layoutViewers();
	}
}

void ViewerController::addViewer(ds::model::MediaRef newMedia, const ci::vec3 location, const float startWidth) {
	TitledMediaViewer* tmv = new TitledMediaViewer(mGlobals);
	addChildPtr(tmv);
	mViewers.push_back(tmv);
	tmv->setMedia(newMedia);
	tmv->setPosition(location);
	tmv->setViewerWidth(startWidth);
	tmv->animateOn();
}

void ViewerController::animateViewerOff(TitledMediaViewer* viewer, const float delayey) {
	if(!viewer) return;

	viewer->tweenStarted();
	viewer->tweenPosition(ci::vec3(viewer->getPosition().x + viewer->getWidth()/4.0f, viewer->getPosition().y + 100.0f + viewer->getHeight()/4.0f, viewer->getPosition().z), mGlobals.getAnimDur(), delayey, ci::EaseInQuad());
	viewer->tweenOpacity(0.0f, mGlobals.getAnimDur(), delayey, ci::EaseInQuad());
	viewer->tweenScale(viewer->getScale() / 2.0f, mGlobals.getAnimDur(), delayey, ci::EaseInQuad(), [this, viewer](){
		removeViewer(viewer);
	});
}

void ViewerController::removeViewer(TitledMediaViewer* viewer) {
	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		if((*it) == viewer){
			mViewers.erase(it);
			break;
		}
	}

	if(viewer){
		viewer->exit();
		viewer->release();
	}
}

void ViewerController::createGridLayout(const ci::Rectf area, const int numItems, std::vector<ci::vec2>& positions){
	if(numItems < 1) return;

	float xStart = area.getX1();
	float yStart = area.getY1();

	float numContents = (float)(numItems);
	int contentsPerRow = (int)ceilf(sqrt(numContents));
	if(numContents > 5) contentsPerRow += 1; /// make the aspect more wide
	if(numContents > 15) contentsPerRow += 1; /// make the aspect more wide
	if(numContents > 25) contentsPerRow += 1; /// make the aspect more wide
	if(numContents > 35) contentsPerRow += 2; /// make the aspect more more wide
	int numRows = (int)ceilf(numContents / (float)(contentsPerRow));

	float xSpacing = area.getWidth() / (float)(contentsPerRow);
	float ySpacing = area.getHeight() / (float)(numRows);

	int rowCounter = 0;

	xStart += xSpacing / 2.0f;
	float xp = xStart;
	float yp = yStart;

	for(int i = 0; i < numItems; i++){
		ci::vec2 origin = ci::vec2(xp, yp);

		if(contentsPerRow == 1){
			origin.x = area.getX1() + area.getWidth() / 2.0f;
		}

		if(numRows == 1){
			origin.y = yStart + area.getHeight() / 4.0f;
		}

		positions.push_back(origin);

		xp += xSpacing;

		rowCounter++;
		if(rowCounter == contentsPerRow){
			rowCounter = 0;
			xp = xStart;
			if(numRows == 2){
				// 2 rows, the second row was too low
				yp += ySpacing / 2.0f;
			} else {
				yp += ySpacing;
			}
		}

	}
}

void ViewerController::layoutViewers() {

	float xStart = 0.0f;
	float xStop = mEngine.getWorldWidth();

	float yStart = 0.0f;
	float yStop = mEngine.getWorldHeight();


	ci::Rectf area = ci::Rectf(xStart, yStart, xStop, yStop);
	std::vector<ci::vec2> positions;
	int numItems = (int)mViewers.size();
	createGridLayout(area, numItems, positions);

	float delayey = 0.0f;
	const float deltaDelay = mGlobals.getAnimDur() / (float)(numItems);

	for(int i = 0; i < mViewers.size(); ++i){
		mViewers[i]->tweenStarted();
		mViewers[i]->hideTitle();
		mViewers[i]->tweenPosition(ci::vec3(positions[i].x - mViewers[i]->getMinSize().x/2.0f, positions[i].y, 0.0f), mGlobals.getAnimDur(), delayey, ci::EaseInOutQuad());
		mViewers[i]->animateWidthTo(mViewers[i]->getMinSize().x);
		delayey += deltaDelay;
	}
}



} // namespace mv


