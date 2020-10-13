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
	mVisibleFrame = nullptr;
	int i = 0;
	for (auto it = imageFiles.begin(); it < imageFiles.end(); ++it) {
		bool created_new_frames = false;
		while (mFrames.size() < i + 1) {
			ds::ui::Image* img = new ds::ui::Image(mEngine, (*it), ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
			addChild(*img);
			img->hide();
			mFrames.push_back(img);
			created_new_frames = true;
		}

		if (!created_new_frames)
				mFrames[i]->setImageFile(*it, ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);

		i++;
	}

	mNumFrames = static_cast<int>(imageFiles.size());

	// Hide all the old frames if there were previously more frames than there are now
		for (i = mNumFrames; i < mFrames.size(); i++)
		mFrames[i]->hide();

		if (mNumFrames == 0) {
		DS_LOG_WARNING("Png Sequence didn't load any frames. Whoops.");
		mPlaying = false;
		return;
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

void PngSequenceSprite::onUpdateServer(const ds::UpdateParams& p) {
	// Remove old (unused) Image sprites from the back of the frames if the count has changed...
	while (mFrames.size() > mNumFrames) {
		auto last_frame = mFrames.back();
		last_frame->release();
		mFrames.pop_back();
	}

	if (!isLoaded()) {
		bool allFramesLoaded = true;
		for (auto frame : mFrames) {
			if (!frame->isLoaded()) {
				allFramesLoaded = false;
				break;
			}
		}

		if (mIsLoaded != allFramesLoaded) {
			mIsLoaded = allFramesLoaded;
			if (mLoadedCallback && mIsLoaded) {
				mLoadedCallback();
			}
		}
	}

	if (mPlaying
	   && mNumFrames > 0
	   && !mFrames.empty() 
	   && mCurrentFrameIndex < mFrames.size()) {

		bool advanceFrame(true);

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

		bool doAdvance = advanceFrame && !mFrames.empty() && mCurrentFrameIndex > -1 && mCurrentFrameIndex < mFrames.size();
	//	std::cout << doAdvance << " " << advanceFrame<< " "  << !mFrames.empty() << " " << mCurrentFrameIndex << " " << mFrames.size() << std::endl;
		if(advanceFrame 
		   && !mFrames.empty() 
		   && mCurrentFrameIndex > -1 
		   && mCurrentFrameIndex < mFrames.size()){
			// hide the old frame
			mFrames[mCurrentFrameIndex]->hide();

			// advance the frame
			mCurrentFrameIndex++;
			if(mCurrentFrameIndex > mNumFrames - 1){
				if(mLoopStyle == Loop){
					mCurrentFrameIndex = 0;
				} else if(mLoopStyle == Once){
					mCurrentFrameIndex = mNumFrames - 1;
					pause();
				} else {
					mCurrentFrameIndex = mNumFrames - 1;
					pause();
				}
			}

			// show the new frame

			if (mVisibleFrame) {
				mVisibleFrame->hide();
			}

			mVisibleFrame = mFrames[mCurrentFrameIndex];
			mVisibleFrame->show();
		}

		if (mPlaying
			&& mNumFrames > 0
			&& !mFrames.empty()
			&& mAnimationEndedCallback
			&& mCurrentFrameIndex == mFrames.size() - 1)
		{
			mAnimationEndedCallback();
		}
	}
}

} // namespace ui
} // namespace ds
