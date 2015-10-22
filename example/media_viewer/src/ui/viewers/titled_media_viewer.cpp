#include "titled_media_viewer.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace mv {

TitledMediaViewer::TitledMediaViewer(Globals& g)
	: ds::ui::MediaViewer(g.mEngine, true)
	, mGlobals(g)

	, mTrayHolder(nullptr)
	, mBackground(nullptr)
	, mTitle(nullptr)
	, mBody(nullptr)
	, mShowingTitle(false)
{
	mTrayHolder = new ds::ui::Sprite(mEngine);
	mTrayHolder->hide();
	mTrayHolder->setOpacity(0.0f);
	addChildPtr(mTrayHolder);

	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->setColor(ci::Color::black());
	mBackground->setOpacity(0.98f);
	mBackground->setBlendMode(ds::ui::BlendMode::MULTIPLY);
	mBackground->setTransparent(false);
	mTrayHolder->addChildPtr(mBackground);

	mTitle = mGlobals.getText("viewer:title").createMultiline(mEngine, mTrayHolder);
	mBody = mGlobals.getText("viewer:body").createMultiline(mEngine, mTrayHolder);

	setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
		toggleTitle();
	});
}

void TitledMediaViewer::setMedia(ds::model::MediaRef media) {
	loadMedia(media.getPrimaryResource());

	if(mTitle){
		mTitle->setText(media.getTitle());
	}

	if(mBody){
		mBody->setText(media.getBody());
	}

	onLayout();
	enter();
}

void TitledMediaViewer::onLayout() {
	MediaViewer::onLayout();
	
	const float w = getWidth();
	const float h = getHeight();

	const float padding = mGlobals.getSettingsLayout().getFloat("titled_viewer:padding", 0, 20.0f);
	if(mBody && mBackground && mTitle){
		// do the stuff

		float yp = padding + h;
		mTitle->setPosition(padding, yp);
		mTitle->setResizeLimit(w - padding * 2.0f);

		yp += mTitle->getHeight() + padding;

		mBody->setPosition(padding, yp);
		mBody->setResizeLimit(w - padding * 2.0f);

		yp += mBody->getHeight() + padding;

		mBackground->setSize(w, yp);

	}
}


void TitledMediaViewer::animateOn(){
	tweenStarted();
	show();
	tweenOpacity(1.0f, mGlobals.getAnimDur(), 0.0f, ci::easeNone, [this]{tweenEnded(); });
}

void TitledMediaViewer::animateOff(){
	tweenStarted();
	tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); tweenEnded(); });
}

void TitledMediaViewer::showTitle() {
	if(mShowingTitle) return;
	mShowingTitle = true;

	if(mTrayHolder){
		mTrayHolder->show();
		mTrayHolder->tweenOpacity(1.0f, mGlobals.getAnimDur());
	}
}

void TitledMediaViewer::hideTitle() {
	if(!mShowingTitle) return;
	mShowingTitle = false;

	if(mTrayHolder){
		mTrayHolder->tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{mTrayHolder->hide(); });
	}
}

void TitledMediaViewer::toggleTitle() {
	if(mShowingTitle){
		hideTitle();
	} else {
		showTitle();
	}
}


} // namespace mv
