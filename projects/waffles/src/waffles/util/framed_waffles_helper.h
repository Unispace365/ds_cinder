#pragma once
#include <ds/content/base_content_helper.h>
#include <ds/ui/media/media_interface.h>
#include <waffles/util/base_waffles_helper.h>
namespace waffles {

using namespace ds::model;

class FramedWafflesHelper final : public BaseWafflesHelper {
  public:
	FramedWafflesHelper(ds::ui::SpriteEngine& eng);
	~FramedWafflesHelper() override;

	// Inherited via BaseWafflesHelper
	void setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) override;
};
} // namespace waffles