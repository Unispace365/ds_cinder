#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/sprite.h>

#include <waffles/util/waffles_helper.h>
namespace ds::model {
class Platform;
}
namespace waffles {
class ViewerCreationArgs;
class BaseElement;
struct RequestViewerLaunchEvent;

/**
 * \class waffles::ViewerController
 *			Manages and mediates all media viewers
 */
class ViewerController : public ds::ui::Sprite {
  public:
	ViewerController(ds::ui::SpriteEngine& g, ci::vec2 size = ci::vec2(-1.f));

	static ViewerController* getInstance();

	virtual void setLayerBounds(int viewLayer, ci::Rectf bounds);

	/// Add a new viewer with the specified type and media
	virtual void addViewer(ViewerCreationArgs& creationArgs, const float delay = 0.0f);

	/// Animates a viewer offscreen and releases it when the animation completes.
	/// Assume the viewer won't exist after calling this
	/// style: 0 == scale / fade to center; 1 == fade only, 2 == fall off the bottom (see app defs)
	virtual void animateViewerOff(BaseElement* viewer, const float delayey, const int style);

	/// Tries to evenly space everything using a bin packing algorithm
	/// ScreenId is the ID of the screen to arrange. ScreenId < 0 will arrange all screens
	virtual void arrangeViewers();

	/// Moves all moveable/layoutable viewers to near this point and makes resizeable viewers smaller
	virtual void gatherViewers(const ci::vec3& location);

	// DO NOT RELEASE, REMOVE, or ADD ANY ELEMENTS FROM/TO THIS VECTOR
	// This is exposed for convenience - for easier querying and interconnection of elements onscreen
	// The viewers in this list are still owned by this class.
	// Use the public methods for adding/removing viewers.
	virtual std::vector<BaseElement*>& getViewers() { return mViewers; }

	virtual std::vector<BaseElement*> getViewersOfType(const std::string& type);

	virtual std::vector<BaseElement*> getViewersWithResourceId(const ds::Resource::Id& resourceId);

	// Makes the supplied viewer fullscreen and adds a black layer behind it
	virtual void fullscreenViewer(BaseElement* viewer, const bool immediate, const bool showController = true);

	// Makes the supplied viewer not fullscreen and removes any associated black layers behind it
	virtual void unfullscreenViewer(BaseElement* viewer, const bool immediate);


	std::map<BaseElement*, ds::ui::Sprite*> getFullscreenDarkeners(){
		return mFullscreenDarkeners;
	}



  protected:
	// immediately releases the viewer with no animation
	virtual void removeViewer(BaseElement* viewer);
	virtual void removeFullscreenDarkener(BaseElement* be);
	virtual void enforceViewerLimits(BaseElement* viewer);
	virtual void gatherAviewer(BaseElement* viewer, const ci::vec3& pos);

	// responds true if there was a pdf, false if this was not handled
	virtual bool advancePDF(const bool forwards);

	virtual void startPresentation(ds::model::ContentModelRef newPresentation, const ci::vec3& startLocation,
						   const bool showController);
	virtual void endPresentation();
	virtual void advancePresentation(const bool forwards);
	virtual void setPresentationSlide(int slideId);

	// Do not call directly, this should only be called from the app event
	virtual void loadPresentationSlide(ds::model::ContentModelRef slideRef);
	virtual void loadSlideBackground(ds::model::ContentModelRef slideRef);
	virtual void loadSlideComposite(ds::model::ContentModelRef slideRef);

	virtual void viewerActivated(BaseElement* be);


	//handlers
	virtual void handleRequestViewerLaunch(const RequestViewerLaunchEvent& e);
	std::function <void(const RequestViewerLaunchEvent&)> mRequestViewerLaunchCallback;
	
	ds::ui::Sprite* mTopLayer		 = nullptr;
	ds::ui::Sprite* mNormalLayer	 = nullptr;
	ds::ui::Sprite* mBackgroundLayer = nullptr;

	ds::EventClient mEventClient;

	std::vector<BaseElement*>				mViewers;
	std::map<BaseElement*, ds::ui::Sprite*> mFullscreenDarkeners;
	ci::vec2 mDisplaySize;
};

} // namespace waffles
