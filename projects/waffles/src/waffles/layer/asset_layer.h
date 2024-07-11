#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace ds::ui {
class TouchMenu;
}

namespace waffles {
class ViewerController;
}

namespace waffles {

/**
 * \class waffles::AssetLayer
 *			The Asset layer. Holds Waffles
 */
class AssetLayer : public ds::ui::SmartLayout {
  public:
	AssetLayer(ds::ui::SpriteEngine& eng, bool isReceiver = false);

	virtual void drawClient(const ci::mat4& transform, const ds::DrawParams& dp) override;

	virtual void onSizeChanged() override;

  private:
	bool mAmReceiver = false;
	bool mAmMulti = false;

	waffles::ViewerController* mViewerController = nullptr;
	float mReceiverScale = 1.f;

};

} // namespace waffles
