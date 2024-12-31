#pragma once
#include "waffles/viewers/viewer_controller.h"

namespace waffles {
class FramedViewerController : public ViewerController {

public:
	FramedViewerController(ds::ui::SpriteEngine& g, ci::vec2 size = ci::vec2(-1.f), std::string channel = "");
	virtual void initCreators() override;


};
} // namespace waffles