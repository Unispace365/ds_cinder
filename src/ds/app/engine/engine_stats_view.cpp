#include "engine_stats_view.h"

#include "ds/app/blob_reader.h"
#include "ds/data/data_buffer.h"
#include "engine_data.h"

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
		: inherited(e)
		, mEngine((ds::Engine&)e)
		, mEventClient(e.getNotifier(), [this](const ds::Event *e) { if (e) onAppEvent(*e); })
		, mFontSize(30.0f)
		, mLT(mEngine.getEngineData().mSrcRect.x1, mEngine.getEngineData().mSrcRect.y1)
		, mBorder(20.0f, 20.0f) {
	mBlobType = BLOB_TYPE;

	setDrawDebug(true);
	hide();
	setTransparent(false);
	setColor(0, 0, 0);
	setOpacity(0.5f);

	setSize(400.0f, 400.0f);
}

void EngineStatsView::updateServer(const ds::UpdateParams &p) {
	inherited::updateServer(p);

	if (!visible()) {
		mTextureFont = ci::gl::TextureFontRef();
	}
}

void EngineStatsView::updateClient(const ds::UpdateParams& p){
	inherited::updateClient(p);

	if(!visible()) {
		mTextureFont = ci::gl::TextureFontRef();
	}
}

void EngineStatsView::drawLocalClient() {
	inherited::drawLocalClient();
	drawStats();
}

void EngineStatsView::drawLocalServer(){
	inherited::drawLocalServer();
	drawStats();
}

void EngineStatsView::drawStats(){
	if(!mTextureFont) {
		makeTextureFont();
		if(!mTextureFont) return;
	}

	if(!mFont) return;

	// TODO
	/*
	ci::gl::GlslProg&	shader = mSpriteShader.getShader();
	if(shader) shader.unbind();

	ci::gl::color(1, 1, 1, 1);

	float				y = mLT.y + mBorder.y;
	const float			gap = 5.0f;
	y = drawLine(make_line("Sprites", (int)mEngine.mSprites.size()), y) + gap;
	y = drawLine(make_line("Touch mode (t)", ds::ui::TouchMode::toString(mEngine.mTouchMode)), y) + gap;
	y = drawLine(make_line("FPS", mEngine.getAverageFps()), y) + gap;
	*/
}

float EngineStatsView::drawLine(const std::string &v, const float y) {
	const float			ascent = mFont.getAscent();
	mTextureFont->drawString(v, ci::vec2(mBorder.x, y + ascent));
	return y + ascent + mFont.getDescent();
}

void EngineStatsView::onAppEvent(const ds::Event &_e) {
	if (Toggle::WHAT() == _e.mWhat) {
		if(visible()){
			hide();
		} else {
			show();
		}
	}
}

void EngineStatsView::makeTextureFont() {
	try {
		// Fonts I'm looking for, in order of precendence
		std::vector<std::string>	cmp;
		cmp.push_back("Helvetica");
		cmp.push_back("Arial");
		cmp.push_back("Segoe UI");
		cmp.push_back("Times New Roman");

		// Find the best match in the list
		std::vector<std::string>	n = ci::Font::getNames();
		std::string					name;
		size_t						dist = 100000;
		for (auto it=n.begin(), end=n.end(); it!=end; ++it) {
//			std::cout << "n=" << *it << std::endl;
			if (name.empty()) {
				name = *it;
			} else {
				auto				found = std::find(cmp.begin(), cmp.end(), *it);
				if (found != cmp.end()) {
					const size_t	d = std::distance(cmp.begin(), found);
					if (d < dist) {
						name = *it;
						dist = d;
					}
				}
			}
		}
		if (name.empty()) return;

		mFont = ci::Font(name, mFontSize);
		mTextureFont = ci::gl::TextureFont::create(mFont);
	} catch (std::exception const&) {
	}
}

/**
 * \class ds::EngineStatsView::Changed
 */
static ds::EventRegistry    TOGGLE_EVENT("EngineStatsView::Toggle");

int EngineStatsView::Toggle::WHAT() {
	return TOGGLE_EVENT.mWhat;
}

EngineStatsView::Toggle::Toggle()
		: Event(TOGGLE_EVENT.mWhat) {
}

} // namespace ds
