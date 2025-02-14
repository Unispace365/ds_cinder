#pragma once

#include <ds/ui/sprite/sprite.h>

#include <nvpath/NvPath.h>

namespace ds::ui {

class PathSprite : public ds::ui::Sprite {
  public:
	PathSprite(ds::ui::SpriteEngine& engine);

	void setPath(const std::string& pathDef);

	void setFill(const std::string& fillColor);
	void setStroke(const std::string& strokeColor);

	void drawLocalClient() override;

  private:
	nvpath::Path mPath;
	ci::ColorA	 mFillColor{1, 1, 1, 1};
	ci::ColorA	 mStrokeColor{0, 0, 0, 0};
	ci::Rectf	 mBounds;
};

} // namespace ds::ui