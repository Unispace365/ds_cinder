#pragma once

#include <memory>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/content/content_helper.h>
namespace waffles {

/**
 * \class waffles::wafflesHelper
 *			The background layer for interactive playlists and templates
 */

class WafflesHelper;

typedef std::shared_ptr<WafflesHelper> WafflesHelperPtr;





class WafflesHelper : public ds::model::ContentHelper {
  public:
	  
	WafflesHelper(ds::ui::SpriteEngine& eng)
	  : ds::model::ContentHelper(eng) {};
	
	virtual bool							getApplyParticles() = 0;
	virtual ds::model::ContentModelRef				getPinboard()= 0;
	virtual ds::model::ContentModelRef				getAnnotationFolder()			   = 0;
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards()=0;
	virtual std::vector<ds::Resource>				findMediaResources()=0;

 
};

} // namespace waffles
