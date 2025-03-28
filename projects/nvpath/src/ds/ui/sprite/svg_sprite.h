#pragma once

#include <ds/ui/button/button.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/util/float_util.h>

#include <nvpath/nv_path_svg.h>

namespace ds::ui {

class SvgSprite : public ds::ui::Sprite {
  public:
	SvgSprite(ds::ui::SpriteEngine& engine);

	void setFile(const std::string& filename);

	float getWidth() const override { return mDoc && approxZero(mWidth) ? mDoc->getWidth() : mWidth; }
	float getHeight() const override { return mDoc && approxZero(mHeight) ? mDoc->getHeight() : mHeight; }

	void drawLocalClient() override;

  private:
	std::string			   mFile;
	nvpath::svg::SvgDocRef mDoc;
	nvpath::svg::Svg	   mSvg;
};

class SvgButton : public ds::ui::Sprite, public ds::ui::IButton {
  public:
	static SvgButton& makeButton(ds::ui::SpriteEngine& engine, const std::string& downSvg, const std::string& upSvg,
								 float touchPad = 0.0f, ds::ui::Sprite* parent = nullptr);

	SvgButton(ds::ui::SpriteEngine& engine, const std::string& downSvg = "", const std::string& upSvg = "",
			  float touchPad = 0.0f);

	float getPad() const override;
	void  setTouchPad(float touchPad) override;

	float getWidth() const override;
	float getHeight() const override;

	void setAnimationDuration(float dur) override;

	void setClickFn(const std::function<void()>&) override;

	void setStateChangeFn(const std::function<void(bool pressed)>&) override;

	ds::ui::Sprite& getNormalSprite() const override { return mUp; }
	ds::ui::Sprite& getHighSprite() const override { return mDown; }

	SvgSprite&	getNormalSvg() const { return mUp; }
	void		setNormalSvg(const std::string& svgFile);
	std::string getNormalSvgPath() { return mNormalFilePath; }

	SvgSprite&	getHighSvg() const { return mDown; }
	void		setHighSvg(const std::string& svgFile);
	std::string getHighSvgPath() { return mHighFilePath; }

	void showDown() const override;
	void showUp() const override;

	ds::ButtonBehaviour::State getButtonState() override { return mButtonBehaviour.getState(); }

  private:
	void handleResize() const;

	void					  onClicked() const;
	std::function<void()>	  mClickFn;
	std::function<void(bool)> mStateChangeFunction;

	/// VIEW
	SvgSprite&	mDown;
	SvgSprite&	mUp;
	std::string mHighFilePath;
	std::string mNormalFilePath;

	/// TOUCH
	ds::ButtonBehaviour mButtonBehaviour;

	/// SETTINGS
	float mPad;
	float mAnimDuration;
};

} // namespace ds::ui