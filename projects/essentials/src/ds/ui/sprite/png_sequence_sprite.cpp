#include "png_sequence_sprite.h"

#include <cinder/app/App.h>

#include <ds/debug/logger.h>

namespace ds{
namespace ui{

PngSequenceSprite::PngSequenceSprite(SpriteEngine& engine, const std::vector<std::string>& imageFiles)
	: inherited(engine)
	, mLoopStyle(Loop)
	, mCurrentFrameIndex(0)
	, mPlaying(true)
	, mFrameTime(0.0f)
{

	setImages(imageFiles);

	mFrames[0]->show();
	mLastFrameTime = ci::app::getElapsedSeconds();
}

PngSequenceSprite::PngSequenceSprite(SpriteEngine& engine)
	: inherited(engine)
	, mLoopStyle(Loop)
	, mCurrentFrameIndex(0)
	, mPlaying(true)
	, mFrameTime(0.0f)
{
	mLastFrameTime = ci::app::getElapsedSeconds();
}

void PngSequenceSprite::setImages(const std::vector<std::string>& imageFiles){
	for(auto it = imageFiles.begin(); it < imageFiles.end(); ++it){
		ds::ui::Image* img = new ds::ui::Image(mEngine, (*it), ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
		addChild(*img);
		mFrames.push_back(img);
		img->hide();
	}

	if(mFrames.empty()){
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

void PngSequenceSprite::setCurrentFrameIndex(const int frameIndex){
	if(frameIndex < 0 || frameIndex > mFrames.size() - 1) return;
	mCurrentFrameIndex = frameIndex;

	for(int i = 0; i < mFrames.size(); i++){
		if(i == mCurrentFrameIndex){
			mFrames[i]->show();
		} else {
			mFrames[i]->hide();
		}
	}
}

const int PngSequenceSprite::getCurrentFrameIndex()const{
	return mCurrentFrameIndex;
}

const int PngSequenceSprite::getNumberOfFrames(){
	return mFrames.size();
}

ds::ui::Image* PngSequenceSprite::getFrameAtIndex(const int frameIndex){
	if(frameIndex < 0 || frameIndex > mFrames.size() - 1) return nullptr;
	return mFrames[frameIndex];
}

void PngSequenceSprite::sizeToFirstImage(){
	if(mFrames.empty()){
		setSize(0.0f, 0.0f);
	} else {
		setSize(mFrames[0]->getScaleWidth(), mFrames[0]->getScaleHeight());
	}
}

void PngSequenceSprite::updateServer(const ds::UpdateParams& p){
	inherited::updateServer(p);

	if(mPlaying && !mFrames.empty()){

		bool advanceFrame(true);

		if(mFrameTime > 0.0f){
			const double thisTime = ci::app::getElapsedSeconds();
			const double deltaTime = thisTime - mLastFrameTime;
			if(deltaTime > (double)mFrameTime){
				advanceFrame = true;
				mLastFrameTime = thisTime;
			} else {
				advanceFrame = false;
			}
		}

		if(advanceFrame){
			// hide the old frame
			mFrames[mCurrentFrameIndex]->hide();

			// advance the frame
			mCurrentFrameIndex++;
			if(mCurrentFrameIndex > mFrames.size() - 1){
				if(mLoopStyle == Loop){
					mCurrentFrameIndex = 0;
				} else if(mLoopStyle == Once){
					mCurrentFrameIndex = mFrames.size() - 1;
					pause();
				} else {
					mCurrentFrameIndex = mFrames.size() - 1;
					pause();
				}
			}

			// show the new frame
			mFrames[mCurrentFrameIndex]->show();
		}

	}
}
} // namespace ui
} // namespace ds