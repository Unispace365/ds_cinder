#pragma once

#include <ds/data/tuio_object.h>
#include <ds/time/timer.h>
#include <ds/ui/layout/smart_layout.h>

namespace ds::ui {
class TouchMenu;
}

namespace waffles {

class CodiceDial;

/**
 * \class waffles::InterfaceLayer
 *			The interface layer. Manages codice objects and/or the persistant waffles menu
 */
class InterfaceLayer : public ds::ui::SmartLayout {
  public:
	InterfaceLayer(ds::ui::SpriteEngine& eng, bool isReceiver = false, std::string channel_name = "");

  private:
	struct CodiceObject {
		CodiceObject(const ds::TuioObject& obj, CodiceDial* sprite)
			: tuioObj(obj)
			, layout(sprite){};
		ds::TuioObject tuioObj;
		CodiceDial*	   layout = nullptr;
	};

	ds::model::ContentModelRef getRecordForCodice(int id);

	void setupTouchMenu();

	std::vector<int>									mLiveObjects;
	std::unordered_map<int, ds::model::ContentModelRef> mObjectMap;
	std::string											mActivePresUid;
	int													mNextFakeCode	  = -1;
	bool												mAllDialsInactive = false;

	ds::time::Callback mPulseTimer;

	ds::ui::TouchMenu* mTouchMenu = nullptr;
};

} // namespace waffles
