#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/app/engine/engine_events.h>

#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mTheVideo(nullptr)
	, mCurShader(-1)
	, mNumShaders(mEngine.getAppSettings().getInt("video:num_shaders", 0, 3))
{

	// Play a video and render it to a texture so we can apply a custom shader to it
	// You can't just set a shader on the video since it uses it's own shader pipeline to display video
	mTheVideo = new ds::ui::Video(mEngine);
	mTheVideo->setLooping(true);
	mTheVideo->setAutoStart(true);
	mTheVideo->setFinalRenderToTexture(true);
	mTheVideo->setMute(true);
	mTheVideo->parseLaunch(mEngine.getAppSettings().getString("video:pipeline", 0, ""), 1920.0f, 1080.0f, ds::ui::GstVideo::kColorTypeSolid);
	addChildPtr(mTheVideo);


	setUseShaderTexture(true);
	setTransparent(false);
	setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setRotation(180.0f);
	setPosition(mEngine.getWorldWidth(), mEngine.getWorldHeight());

	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
	//	getBaseShader().getShader()->uniform("iTime", ti.mCurrentGlobalPoint.x);
		if(ti.mPhase == ds::ui::TouchInfo::Moved) {
			getBaseShader().getShader()->uniform("touchPos", ci::vec2(ti.mCurrentGlobalPoint));
		}
	});

	setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		incrementShader();
	});

	incrementShader();
}

void StoryView::incrementShader() {
	mCurShader++;
	if(mCurShader >= mNumShaders) {
		mCurShader = 0;
	}

	ds::ui::SpriteShader::clearShaderCache();
	setBaseShader(ds::Environment::expand("%APP%/data/shaders"), "vidjo-" + std::to_string(mCurShader));
	getBaseShader().loadShaders();
	getBaseShader().getShader()->uniform("touchPos", ci::vec2(10.0f, 10.0f));

}

void StoryView::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == ds::app::IdleEndedEvent::WHAT()){
		const ds::app::IdleEndedEvent& e((const ds::app::IdleEndedEvent&)in_e);
		animateOn();
	} else if(in_e.mWhat == ds::app::IdleStartedEvent::WHAT()){
		animateOff();
	}

	// If you have an event that is dispatched when new content is queryied, you could map that here.
	if(in_e.mWhat == StoryDataUpdatedEvent::WHAT()){
		setData();
	}
}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data
	if(!mGlobals.mAllData.mStories.empty()){

		auto storyRef = mGlobals.mAllData.mStories.front();

		if(mTheVideo){
			mTheVideo->setResource(storyRef.getPrimaryResource());
			mTheVideo->play();
		}

	}

	layout();
}

void StoryView::layout(){
	if(mTheVideo) {
		ds::ui::fitInside(mTheVideo, ci::Rectf(0.0f, 0.0f, mEngine.getWorldWidth(), mEngine.getWorldHeight()), false, true);
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

void StoryView::onUpdateServer(const ds::UpdateParams& p){
	// any changes for this frame happen here
	if(getBaseShader().getShader()) {
		getBaseShader().getShader()->uniform("iTime", p.getElapsedTime());
	}
}

void StoryView::drawLocalClient() {
	if(mTheVideo && mTheVideo->getFinalOutTexture()) {
		mTheVideo->getFinalOutTexture()->bind(0);
	}

	if(mRenderBatch) {
		mRenderBatch->draw();
	}

	if(mTheVideo && mTheVideo->getFinalOutTexture()) {
		mTheVideo->getFinalOutTexture()->unbind();
	}
}


} // namespace downstream

