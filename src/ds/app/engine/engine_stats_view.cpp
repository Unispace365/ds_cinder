#include "stdafx.h"

#include "engine_stats_view.h"

#include "ds/app/blob_reader.h"
#include "ds/data/data_buffer.h"
#include "engine_data.h"
#include <ds/debug/computer_info.h>

#pragma warning(disable: 4355)

namespace ds {

namespace {
char				BLOB_TYPE			= 0;

std::string			make_line(const std::string &key, const int v) {
	std::stringstream	buf;
	buf << key << ": " << v;
	return buf.str();
}

std::string			make_line(const std::string &key, const std::string &v) {
	return key + ": " + v;
}

std::string			make_line(const std::string &key, const float v) {
	std::stringstream	buf;
	buf << key << ": " << v;
	return buf.str();
}

}

/**
 * \class ds::EngineStatsView
 */
void EngineStatsView::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {ds::ui::Sprite::handleBlobFromClient(r);});
}

void EngineStatsView::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {ds::ui::Sprite::handleBlobFromServer<EngineStatsView>(r);});
}

/**
 * \class ds::EngineStatsView
 */
EngineStatsView::EngineStatsView(ds::ui::SpriteEngine &e)
	: ds::ui::Sprite(e)
	, mEngine((ds::Engine&)e)
	, mEventClient(e.getNotifier(), [this](const ds::Event *e) { if(e) onAppEvent(*e); })
	, mLT(mEngine.getEngineData().mSrcRect.x1, mEngine.getEngineData().mSrcRect.y1)
	, mText(nullptr)

{
	mBlobType = BLOB_TYPE;

	setDrawDebug(true);
	hide();

	mBackground = new ds::ui::Sprite(mEngine, 400.0f, 40.0f);
	mBackground->setTransparent(false);
	mBackground->setColor(0, 0, 0);
	mBackground->setOpacity(0.5f);
	addChildPtr(mBackground);

//	setPosition(mLT);

}

void EngineStatsView::onUpdateServer(const ds::UpdateParams &p) {
	if(visible()) {
		updateStats();
	}
}

void EngineStatsView::onUpdateClient(const ds::UpdateParams& p){
	if(visible()) {
		updateStats();
	}
}

void EngineStatsView::updateStats(){
	if(!mText){
		const float pad = 30.0f;
	
		mText = new ds::ui::Text(mEngine);
		mText->setFont("Arial", 16.0f);
		mText->setPosition(pad, pad);
		mText->setResizeLimit(getWidth() - pad * 2.0f);
		mText->setLeading(1.2f);
		addChildPtr(mText);
	}
	if(mText){
		std::stringstream ss;
		ss << "<span weight='bold'>Sprites:</span> " << mEngine.mSprites.size() << std::endl;
		ss << "<span weight='bold'>Touch mode (t):</span> " << ds::ui::TouchMode::toString(mEngine.mTouchMode) << std::endl;

		ss << "<span weight='bold'>Physical Memory:</span> " << mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess() << std::endl;
		ss << "<span weight='bold'>Virtual Memory:</span> " << mEngine.getComputerInfo().getVirtualMemoryUsedByProcess() << std::endl;
		//ss << "<span weight='bold'>CPU:</span> " << mEngine.getComputerInfo().getPercentUsageCPU() << "%" << std::endl;

		if(mEngine.getMode() != ds::ui::SpriteEngine::STANDALONE_MODE){
			ss << "<span weight='bold'>Bytes Received:</span>\t" << mEngine.getBytesRecieved() << std::endl;
			ss << "<span weight='bold'>Bytes Sent:</span>\t\t" << mEngine.getBytesSent() << std::endl;
		}

		float fpsy = mEngine.getAverageFps();
		if(fpsy < 30.0f){
			ss << "<span weight='bold'>FPS:</span> <span color='red'>" << fpsy << "</span>" << std::endl;
		} else if(fpsy < 59.0f){
			ss << "<span weight='bold'>FPS:</span> <span color='yellow'>" << fpsy << "</span>" << std::endl;
		} else {
			ss << "<span weight='bold'>FPS:</span> " << fpsy << std::endl;
		}

		mText->setText(ss.str());

		if(mBackground->getHeight() < mText->getPosition().y * 2.0f + mText->getHeight()){
			mBackground->setSize(mBackground->getWidth(), mText->getPosition().y * 2.0f + mText->getHeight());
		}
	}
}

void EngineStatsView::onAppEvent(const ds::Event &_e) {
	if (ToggleStatsRequest::WHAT() == _e.mWhat) {
		if(visible()){
			hide();
		} else {
			show();
		}
	}
}

} // namespace ds
