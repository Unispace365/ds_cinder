#pragma once
#include <waffles/util/base_waffles_helper.h>
#include <ds/content/base_content_helper.h>
#include <ds/ui/media/media_interface.h>
namespace waffles {

using namespace ds::model;

class FramedWafflesHelper : public BaseWafflesHelper {
  public:
	FramedWafflesHelper(ds::ui::SpriteEngine& eng);
	~FramedWafflesHelper();
	   
	// Inherited via BaseWafflesHelper
	virtual void									setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) override;
};
}