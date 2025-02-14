#include "stdafx.h"

#include "svg_sprite.h"

namespace {

// Add the 'svg' sprite type so we can use it in our layout XML.
auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
		// Register our custom sprite(s).
		e.registerSpriteImporter(
			"svg", [](ds::ui::SpriteEngine& engine) -> ds::ui::Sprite* { return new ds::ui::SvgSprite(engine); });
		e.registerSpriteImporter("svg_button", [](ds::ui::SpriteEngine& engine) -> ds::ui::Sprite* {
			return new ds::ui::SvgButton(engine);
		});
		// Register the properties for our custom sprites.
		e.registerSpritePropertySetter<ds::ui::SvgSprite>("file",
														  [](ds::ui::SvgSprite& sprite, const std::string& theValue,
															 const std::string&) { sprite.setFile(theValue); });
		e.registerSpritePropertySetter<ds::ui::SvgButton>("up_svg",
														  [](ds::ui::SvgButton& sprite, const std::string& theValue,
															 const std::string&) { sprite.setNormalSvg(theValue); });
		e.registerSpritePropertySetter<ds::ui::SvgButton>("down_svg",
														  [](ds::ui::SvgButton& sprite, const std::string& theValue,
															 const std::string&) { sprite.setHighSvg(theValue); });
	});
	return true;
}();

} // namespace

namespace ds::ui {

SvgSprite::SvgSprite(ds::ui::SpriteEngine& engine)
	: Sprite(engine) {
}

void SvgSprite::setFile(const std::string& filename) {
	mFile = ds::Environment::expand(filename);
	if (std::filesystem::exists(mFile)) {
		mDoc = nvpath::svg::SvgDoc::create(mFile);
		setSize(mDoc->getWidth(), mDoc->getHeight());
		setTransparent(false);
	} else {
		DS_LOG_WARNING("Unable to find SVG: " << mFile);
		setTransparent(true);
	}
}

void SvgSprite::drawLocalClient() {
	if (mDoc) {
		nvpath::ScopedPathRendering sPath{};
		mSvg.setOpacity(getDrawOpacity());
		mDoc->render(mSvg);
	}
}

SvgButton& SvgButton::makeButton(ds::ui::SpriteEngine& engine, const std::string& downSvg, const std::string& upSvg,
								 float touchPad, Sprite* parent) {
	SvgButton* b = new SvgButton(engine, downSvg, upSvg, touchPad);
	if (!b) {
		DS_LOG_WARNING("Can't create SvgButton");
		return *b;
	}
	if (parent) parent->addChild(*b);
	return *b;
}

SvgButton::SvgButton(ds::ui::SpriteEngine& engine, const std::string& downSvg, const std::string& upSvg, float touchPad)
	: Sprite(engine)
	, mDown(*(new SvgSprite(mEngine)))
	, mUp(*(new SvgSprite(mEngine)))
	, mHighFilePath(downSvg)
	, mNormalFilePath(upSvg)
	, mButtonBehaviour(*this)
	, mPad(touchPad)
	, mAnimDuration(0.1f) {
	mLayoutFixedAspect	 = true;
	mDown.mExportWithXml = false;
	mUp.mExportWithXml	 = false;

	addChild(mDown);
	addChild(mUp);

	mUp.setDimensionsChangedCallback([this](Sprite*) { handleResize(); });
	mDown.setDimensionsChangedCallback([this](Sprite*) { handleResize(); });

	mDown.setOpacity(0.0f);

	if (!mHighFilePath.empty()) mDown.setFile(mHighFilePath);
	if (!mNormalFilePath.empty()) mUp.setFile(mNormalFilePath);

	mButtonBehaviour.setOnClickFn([this]() { onClicked(); });
	// Purely for visual state
	mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&) { showDown(); });
	mButtonBehaviour.setOnEnterFn([this]() { showDown(); });
	mButtonBehaviour.setOnExitFn([this]() { showUp(); });
	mButtonBehaviour.setOnUpFn([this]() { showUp(); });
}

void SvgButton::setTouchPad(float touchPad) {
	mPad = touchPad;
	handleResize();
}

void SvgButton::setAnimationDuration(float dur) {
	mAnimDuration = dur;
}

float SvgButton::getPad() const {
	return mPad;
}

void SvgButton::setClickFn(const std::function<void()>& fn) {
	mClickFn = fn;
}

void SvgButton::showDown() const {
	if (mAnimDuration <= 0.0f) {
		mUp.hide();
		mUp.setOpacity(0.0f);
		mDown.show();
		mDown.setOpacity(1.0f);
	} else {
		mUp.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic(), [this]() { mUp.hide(); });
		mDown.show();
		mDown.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
	}

	if (mStateChangeFunction) {
		mStateChangeFunction(true);
	}
}

void SvgButton::showUp() const {
	if (mAnimDuration <= 0.0f) {
		mUp.show();
		mUp.setOpacity(1.0f);
		mDown.hide();
		mDown.setOpacity(0.0f);
	} else {
		mUp.show();
		mUp.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
		mDown.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic(), [this]() { mDown.hide(); });
	}

	if (mStateChangeFunction) {
		mStateChangeFunction(false);
	}
}

void SvgButton::onClicked() const {
	showUp();
	if (mClickFn) mClickFn();
}

void SvgButton::setHighSvg(const std::string& svgFile) {
	mHighFilePath = svgFile;
	mDown.setFile(svgFile);
}

void SvgButton::setNormalSvg(const std::string& svgFile) {
	if (mNormalFilePath == mHighFilePath) {
		setHighSvg(svgFile);
	}
	mNormalFilePath = svgFile;
	mUp.setFile(svgFile);
}

void SvgButton::setStateChangeFn(const std::function<void(bool pressed)>& func) {
	mStateChangeFunction = func;
}

void SvgButton::handleResize() {
	mDown.setPosition(mPad, mPad);
	mUp.setPosition(mDown.getPosition());
	setSize(mPad + glm::max(mUp.getWidth(), mDown.getWidth()) + mPad,
			mPad + glm::max(mUp.getHeight(), mDown.getHeight()) + mPad);
}

} // namespace ds::ui
