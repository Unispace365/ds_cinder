#include "stdafx.h"

#include "png_sequence_sprite.h"

#include <cinder/app/App.h>

#include <ds/debug/logger.h>

namespace ds{
namespace ui{

PngSequenceSprite::PngSequenceSprite(SpriteEngine& engine, const std::vector<std::string>& imageFiles)
	: ds::ui::Sprite(engine)
	, mLoopStyle(Loop)
	, mCurrentFrameIndex(0)
	, mPlaying(true)
	, mIsLoaded(false)
	, mFrameTime(0.0f)
	, mNumFrames(0)
	, mAnimationEndedCallback(nullptr)
	, mLoadedCallback(nullptr)
{
	mLayoutFixedAspect = true;
	setImages(imageFiles);

	if (!mFrames.empty()) {
		mVisibleFrame = mFrames.front();
		mVisibleFrame->show();
	}
	mLastFrameTime = ci::app::getElapsedSeconds();
}

PngSequenceSprite::PngSequenceSprite(SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mLoopStyle(Loop)
	, mCurrentFrameIndex(0)
	, mPlaying(true)
	, mIsLoaded(false)
	, mFrameTime(0.0f)
	, mNumFrames(0)
{
	mLayoutFixedAspect = true;
	mLastFrameTime = ci::app::getElapsedSeconds();
}

void PngSequenceSprite::setImages(const std::vector<std::string>& imageFiles){
	mNumFrames = static_cast<int>(imageFiles.size());

	if (mNumFrames == 0) {
		DS_LOG_WARNING("Png Sequence didn't load any frames. Whoops.");
		mPlaying = false;
	}

	// Remove all the old frames that are no longer needed
	while (mFrames.size() > mNumFrames) {
		auto last_frame = mFrames.back();
		last_frame->release();
		mFrames.pop_back();
	}
	
	// Create new empty frame sprites as needed
	while(mFrames.size() <= mNumFrames){
		ds::ui::Image* img = new ds::ui::Image(mEngine);
		addChildPtr(img); // More idiomatic
		img->hide();
		mFrames.push_back( img );
	}

	mVisibleFrame = nullptr;
	int i = 0;
	for (auto filename : imageFiles) {
		mFrames[i]->setImageFile(filename, mImageFlags);
		i++;
	}
}

void PngSequenceSprite::setFrameTime(const float time){
	if(time < 0.0f) return;
	mFrameTime = time;
}

void PngSequenceSprite::setLoopStyle(LoopStyle style){
	mLoopStyle = style;
}

const PngSequenceSprite::LoopStyle PngSequenceSprite::getLoopStyle()const{
	return mLoopStyle;
}

void PngSequenceSprite::play(){
	mLastFrameTime = ci::app::getElapsedSeconds();
	mPlaying = true;
}

void PngSequenceSprite::pause(){
	mPlaying = false;
}

const bool PngSequenceSprite::isPlaying()const{
	return mPlaying;
}

bool PngSequenceSprite::isLoaded() const {
	return mIsLoaded;
}

void PngSequenceSprite::setCurrentFrameIndex(const int frameIndex) {
	if (frameIndex < 0 || frameIndex > mNumFrames - 1) return;
	mCurrentFrameIndex = frameIndex;

	if (mVisibleFrame) {
		mVisibleFrame->hide();
	}

	mVisibleFrame = mFrames[mCurrentFrameIndex];
	mVisibleFrame->show();
}

const int PngSequenceSprite::getCurrentFrameIndex()const{
	return mCurrentFrameIndex;
}

const int PngSequenceSprite::getNumberOfFrames(){
	return mNumFrames;
}

ds::ui::Image* PngSequenceSprite::getFrameAtIndex(const int frameIndex){
	if(frameIndex < 0 || frameIndex > mNumFrames - 1) return nullptr;
	return mFrames[frameIndex];
}

void PngSequenceSprite::sizeToFirstImage(){
	if(mFrames.empty()){
		setSize(0.0f, 0.0f);
	} else {
		setSize(mFrames[0]->getScaleWidth(), mFrames[0]->getScaleHeight());
	}
}

void PngSequenceSprite::setAnimationEndedCallback(const std::function<void()>& func){
	mAnimationEndedCallback = func;
}

void PngSequenceSprite::setLoadedCallback(const std::function<void()>& func){
	mLoadedCallback = func;
}

void PngSequenceSprite::checkLoaded(){
	if(!mIsLoaded){
		for (auto frame : mFrames) {
			if (!frame->isLoaded()) {
				break;
			}
		}

		mIsLoaded = true;
		if (mLoadedCallback && mIsLoaded) {
			mLoadedCallback();
		}
	}
}

void PngSequenceSprite::onUpdateServer(const ds::UpdateParams& p) {
	checkLoaded();

	if (mPlaying
	   && mNumFrames > 0
	   && !mFrames.empty()
	   && mCurrentFrameIndex > -1 
	   && mCurrentFrameIndex < mFrames.size()) {

		bool advanceFrame = true;

		if (mFrameTime > 0.0f) {
			const double thisTime = ci::app::getElapsedSeconds();
			const double deltaTime = thisTime - mLastFrameTime;
			if (deltaTime > (double)mFrameTime) {
				advanceFrame = true;
				mLastFrameTime = thisTime;
			}
			else {
				advanceFrame = false;
			}
		}

		if(advanceFrame){
			// hide the old frame
			mFrames[mCurrentFrameIndex]->hide();

			// advance the frame
			mCurrentFrameIndex++;

			// Check if we're at the end of a loop/once and act accordingly
			if(mCurrentFrameIndex > mNumFrames - 1){
				if(mLoopStyle == Loop){
					mCurrentFrameIndex = 0;
				} else { // LoopStyle::Once is the default/fallback
					mCurrentFrameIndex = mNumFrames - 1;
					pause();
				}

				if(mAnimationEndedCallback){
					mAnimationEndedCallback();// Call the callback if appropriate
				}
			}

			// Hide the previous frame
			if (mVisibleFrame) {
				mVisibleFrame->hide();
			}

			// Show the current frame
			mVisibleFrame = mFrames[mCurrentFrameIndex];
			mVisibleFrame->show();
		}
	}
}

} // namespace ui
} // namespace ds
