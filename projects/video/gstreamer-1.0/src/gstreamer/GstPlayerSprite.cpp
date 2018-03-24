#include "stdafx.h"

#include "GstPlayerSprite.h"
#include "GstPlayer.h"

namespace ds {
namespace ui {

GstPlayerSprite::GstPlayerSprite(SpriteEngine& eng)
	: Sprite(eng)
{
	mGstPlayer = new gst::video::GstPlayer();
	mGstPlayer->initialize();

	setTransparent(false);
}

GstPlayerSprite::~GstPlayerSprite() {
	if(mGstPlayer) {
		mGstPlayer->stop();
//		delete mGstPlayer;
//		mGstPlayer = nullptr;
	}
}

ds::ui::GstPlayerSprite& GstPlayerSprite::loadVideo(const std::string &filename) {
	if(!mGstPlayer) {
		return *this;
	}

	mGstPlayer->load(filename);

	setSize(mGstPlayer->width(), mGstPlayer->height());

	return *this;
}

void GstPlayerSprite::play() {
	if(!mGstPlayer) return;
	mGstPlayer->play();
}

void GstPlayerSprite::stop() {
	if(!mGstPlayer) return;
	mGstPlayer->stop();
}


void GstPlayerSprite::drawLocalClient() {
	if(!mGstPlayer) return;
	updateVideoTexture();
	if(mFrameTexture) {
		ci::gl::draw(mFrameTexture);
	}
}

void GstPlayerSprite::updateVideoTexture() {
	mFrameTexture = mGstPlayer->getVideoTexture();
}


}
}