#include "five_finger_cluster.h"

#include <ds/debug/logger.h>

namespace ds{
namespace ui{

FiveFingerCluster::FiveFingerCluster(float secondsToTrigger /* = 2.0f */, int minTouches /* = 5 */, float boundingBoxSize /* = 200.0f */)
	: mMinTouches(minTouches)
	, mTriggerTime(secondsToTrigger)
	, mBoundingBoxSize(boundingBoxSize)
	, mMinClusterSeparation(450.0f)
	, mMaxClusterId(0)
{

}

FiveFingerCluster::~FiveFingerCluster(){

}

void FiveFingerCluster::setTriggeredCallback(  const std::function<void (const ci::vec2)> &func )
{
	mTriggeredFunction = func;
}

void FiveFingerCluster::setTriggerableCallback(  const std::function<void (const bool, const ci::vec2)> &func )
{
	mTriggerableFunction = func;
}

void FiveFingerCluster::setClusterUpdateCallback( const std::function<void (const ds::ui::TouchInfo::Phase, const Cluster&)> & func){
	mClusterUpdateFunction = func;
}

int FiveFingerCluster::closeToCluster(ci::vec3 pos){
	float deltaX(0.0f), deltaY(0.0f);
	for (int i = 0; i < mClusters.size(); i++){
		Cluster& c = mClusters[i];
		for (int t = 0; t < c.mTouches.size(); t++){
			ds::ui::TouchInfo& ti = c.mTouches[t];
			deltaX = ti.mCurrentGlobalPoint.x - pos.x;
			deltaY = ti.mCurrentGlobalPoint.y - pos.y;
			float distSquared = deltaX * deltaX + deltaY * deltaY;
			if(distSquared < mMinClusterSeparation * mMinClusterSeparation){
				return i;
			}
		}
	}

	return -1;
}

int FiveFingerCluster::findCluster(int fingerId, ci::vec3 curPos){
	for (int i = 0; i < mClusters.size(); i++){
		Cluster& c = mClusters[i];
		for (auto it = c.mTouches.begin(); it < c.mTouches.end(); ++it){
			if((*it).mFingerId == fingerId){
				(*it).mCurrentGlobalPoint = ci::vec3(curPos.x, curPos.y, 0.0f);
				return i;
			}
		}
	}

	return -1;
}

void FiveFingerCluster::parseTouch(const ds::ui::TouchInfo& ti){

	if(ti.mPhase == ds::ui::TouchInfo::Added){
		TouchInfo nt = ti;
		int relatedCluster = closeToCluster(ti.mCurrentGlobalPoint);
		if(relatedCluster == -1){
			Cluster clust;
			clust.mTouches.push_back(nt);
			clust.mBoundingBox.set(nt.mCurrentGlobalPoint.x, nt.mCurrentGlobalPoint.y, 0, 0);
			clust.mCurrentBoundingBox.set(nt.mCurrentGlobalPoint.x, nt.mCurrentGlobalPoint.y, 0, 0);
			clust.mMaxTouches = 1;
			clust.mClusterId = mMaxClusterId;
			mMaxClusterId++;
			mClusters.push_back(clust);

			if(mClusterUpdateFunction) mClusterUpdateFunction(ds::ui::TouchInfo::Added, mClusters.back());
		} else {
			Cluster& clusty = mClusters[relatedCluster];
			clusty.mTouches.push_back(nt);
			if(clusty.mTouches.size() == 1){
				clusty.mBoundingBox.set(nt.mCurrentGlobalPoint.x, nt.mCurrentGlobalPoint.y, 0,0);
			}

			if(clusty.mTouches.size() < mMinTouches + 1 && !clusty.mTriggerable){
				clusty.mInitialTouchTime = Poco::Timestamp().epochMicroseconds();
			}
			clusty.addToBoundingBox(nt.mCurrentGlobalPoint, clusty.mBoundingBox);
			clusty.mMaxTouches++;
			clusty.configureCurrentBoundingBox();
			if(mClusterUpdateFunction) mClusterUpdateFunction(ds::ui::TouchInfo::Moved, clusty);
		}

	
	} else if (ti.mPhase == ds::ui::TouchInfo::Moved){
		int relatedCluster = findCluster(ti.mFingerId, ti.mCurrentGlobalPoint);
		if(relatedCluster == -1){
			// Disabling warning. This seems to happen only if moved gets called before added,
			// which only happens in case of weird touch errors or from clicks.
			// In both cases, silently throw those touches out, they don't matter.
			//DS_LOG_WARNING("FiveFingerCluster couldn't find a related cluster for finger id:" << ti.fingerID);
			return;
		}

		Cluster& clustyMcClustClust = mClusters[relatedCluster];
		clustyMcClustClust.addToBoundingBox(ti.mCurrentGlobalPoint, clustyMcClustClust.mBoundingBox);
		clustyMcClustClust.configureCurrentBoundingBox();

		if(mClusterUpdateFunction) mClusterUpdateFunction(ds::ui::TouchInfo::Moved, clustyMcClustClust);

		if(clustyMcClustClust.mTriggerable){
			if(clustyMcClustClust.mBoundingBox.getWidth() > mBoundingBoxSize || clustyMcClustClust.mBoundingBox.getHeight() > mBoundingBoxSize){
				clustyMcClustClust.mTriggerable = false;
				if(mTriggerableFunction) mTriggerableFunction(false, ci::vec2(clustyMcClustClust.mBoundingBox.getCenter().x, clustyMcClustClust.mBoundingBox.getCenter().y));
			}
		} else {
			if(clustyMcClustClust.mBoundingBox.getWidth() < mBoundingBoxSize 
				&& clustyMcClustClust.mBoundingBox.getHeight() < mBoundingBoxSize 
				&& clustyMcClustClust.mMaxTouches > mMinTouches - 1 
				&& (Poco::Timestamp().epochMicroseconds() - clustyMcClustClust.mInitialTouchTime) > mTriggerTime * 1000000){
				clustyMcClustClust.mTriggerable = true;
				if(mTriggerableFunction) mTriggerableFunction(true, ci::vec2(clustyMcClustClust.mBoundingBox.getCenter().x, clustyMcClustClust.mBoundingBox.getCenter().y));
			}
		}
	} else if ( ti.mPhase == ds::ui::TouchInfo::Removed ) {
		int relatedCluster = findCluster(ti.mFingerId, ti.mCurrentGlobalPoint);
		if(relatedCluster == -1){
			// Disabling warning. This seems to happen only if removed gets called before added,
			// which only happens in case of weird touch errors or from RE clicks.
			// In both cases, silently throw those touches out, they don't matter.
			//DS_LOG_WARNING("FiveFingerCluster removed: couldn't find a related cluster for finger id:" << ti.fingerID);
			return;
		}

		Cluster& clustyMcClustClust = mClusters[relatedCluster];

		for(int i = 0; i < clustyMcClustClust.mTouches.size(); i++){
			if (clustyMcClustClust.mTouches[i].mFingerId == ti.mFingerId){
				clustyMcClustClust.mTouches.erase(clustyMcClustClust.mTouches.begin() + i);
				if(clustyMcClustClust.mTouches.size() < mMinTouches && !clustyMcClustClust.mTriggerable){
					clustyMcClustClust.mInitialTouchTime = Poco::Timestamp().epochMicroseconds();
					clustyMcClustClust.mMaxTouches --;
				}
				break;
			}
		}
		if (clustyMcClustClust.mTouches.size() == 0){

			if(mClusterUpdateFunction) mClusterUpdateFunction(ds::ui::TouchInfo::Removed, clustyMcClustClust);

			clustyMcClustClust.mTouches.clear();
			if (clustyMcClustClust.mTriggerable){
				if (mTriggeredFunction) mTriggeredFunction(ci::vec2(clustyMcClustClust.mBoundingBox.getCenter().x, clustyMcClustClust.mBoundingBox.getCenter().y));
				if (mTriggerableFunction) mTriggerableFunction(false, ci::vec2(clustyMcClustClust.mBoundingBox.getCenter().x, clustyMcClustClust.mBoundingBox.getCenter().y));
				clustyMcClustClust.mTriggerable = false;
			}
			clustyMcClustClust.mMaxTouches = 0;

			mClusters.erase(mClusters.begin() + relatedCluster);
		}
	}
}

void FiveFingerCluster::Cluster::addToBoundingBox(ci::vec3 p, ci::Rectf& boxToEdit){
	if(!boxToEdit.contains(ci::vec2(p.x, p.y))){
		boxToEdit.include(ci::vec2(p.x, p.y));
	}
}

void FiveFingerCluster::Cluster::configureCurrentBoundingBox(){
	if(mTouches.size() < 1){
		mCurrentBoundingBox.set(0, 0, 0, 0);
		return;
	}

	mCurrentBoundingBox.set(mTouches[0].mCurrentGlobalPoint.x, mTouches[0].mCurrentGlobalPoint.y, 0,0);
	for (int i = 0; i < mTouches.size(); i++){
		addToBoundingBox(mTouches[i].mCurrentGlobalPoint, mCurrentBoundingBox);
	}
}

} // namespace ui
} // namespace ds