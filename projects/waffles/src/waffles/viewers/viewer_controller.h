#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/panel/base_panel.h>
#include <ds/ui/sprite/sprite.h>
#include <waffles/viewers/titled_media_viewer.h>
#include <app/waffles_app_defs.h>
#include <waffles/util/waffles_helper.h>

namespace ds::model {
class Platform;
}
namespace waffles {
class ViewerCreationArgs;
class BaseElement;
struct RequestViewerLaunchEvent;


class ViewerController;
class ViewerControllerFactory;


typedef std::shared_ptr<ViewerController> ViewerControllerPtr;


enum class CreationError { OK = 0, INVALID_TYPE, INVALID_MEDIA };

typedef std::function<std::tuple<BaseElement*, CreationError>(const ViewerCreationArgs)> CreatorFunc;


/**
 * \class waffles::ViewerController
 *			Manages and mediates all media viewers
 */
class ViewerController : public ds::ui::Sprite {
  public:
	ViewerController(ds::ui::SpriteEngine& g, ci::vec2 size = ci::vec2(-1.f),std::string channel="");
	~ViewerController() {
		if (mDeletingCallback) mDeletingCallback(); 
	}
	
	virtual void initCreators();

	void					 setChannel(const std::string& channel);
	static ViewerController* getInstance();

	virtual void setLayerBounds(int viewLayer, ci::Rectf bounds);

	/// Add a new viewer with the specified type and media. Returns the new viewer.
	virtual BaseElement* addViewer(ViewerCreationArgs& creationArgs, const float delay = 0.0f);

	/// Animates a viewer offscreen and releases it when the animation completes.
	/// Assume the viewer won't exist after calling this.
	/// style: 0 == scale / fade to center; 1 == fade only, 2 == fall off the bottom (see app defs)
	virtual void animateViewerOff(BaseElement* viewer, const float delayey, const int style);

	/// Removes all viewers from the screen.
	/// Assume no viewers exist after calling this.
	/// style: 0 == scale / fade to center; 1 == fade only, 2 == fall off the bottom (see app defs)
	virtual void animateAllViewersOff(const float delayey, const int style);

	/// Tries to evenly space everything using a bin packing algorithm
	/// ScreenId is the ID of the screen to arrange. ScreenId < 0 will arrange all screens
	virtual void arrangeViewers();

	/// Moves all moveable/layoutable viewers to near this point and makes resizeable viewers smaller
	virtual void gatherViewers(const ci::vec3& location);

	// DO NOT RELEASE, REMOVE, or ADD ANY ELEMENTS FROM/TO THIS VECTOR
	// This is exposed for convenience - for easier querying and interconnection of elements onscreen
	// The viewers in this list are still owned by this class.
	// Use the public methods for adding/removing viewers.
	virtual const std::vector<BaseElement*>& getViewers() const { return mViewers; }

	virtual std::vector<BaseElement*> getViewersOfType(const std::string& type) const;

	virtual std::vector<BaseElement*> getViewersWithResourceId(const ds::Resource::Id& resourceId) const;

	// Makes the supplied viewer fullscreen and adds a black layer behind it
	virtual void fullscreenViewer(BaseElement* viewer, const bool immediate, const bool showController = true);

	virtual void setupFullscreenDarkener(waffles::BaseElement*& viewer, bool& retFlag);

	// Makes the supplied viewer not fullscreen and removes any associated black layers behind it
	virtual void unfullscreenViewer(BaseElement* viewer, const bool immediate);

	virtual void detachViewer(BaseElement* viewer);

	virtual void attachViewer(BaseElement* viewer);

	std::map<BaseElement*, ds::ui::Sprite*> getFullscreenDarkeners(){
		return mFullscreenDarkeners;
	}
	virtual ds::ui::Sprite*					getTopLayer() { return mTopLayer; }
	virtual ds::ui::Sprite*                 getNormalLayer() { return mNormalLayer; }
	virtual ds::ui::Sprite*					getBackgroundLayer() { return mBackgroundLayer; }
	friend ViewerControllerFactory;


  protected:
	// create viewers
	virtual std::tuple < BaseElement*, CreationError> createViewer(const ViewerCreationArgs args);
	virtual void		 setCreator(const std::string viewType, CreatorFunc creator);

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
	ds::EventClient mChannelClient;

	std::vector<BaseElement*>				mViewers;
	std::map<BaseElement*, ds::ui::Sprite*> mFullscreenDarkeners;
	ci::vec2								mDisplaySize;
	std::unordered_map<std::string, CreatorFunc> mCreatorFunctions;

private:
	std::function<void()>					mDeletingCallback;
	void setDeletingCallback(std::function<void()> callback) {
		mDeletingCallback = callback;
	};
	//std::string								mMediaPropertyKey = "media";
};

class ControllerType
{
public:
	virtual ~ControllerType() {}
	virtual ViewerController* allocate(ds::ui::SpriteEngine& g, ci::vec2 size = ci::vec2(-1.f), std::string channel = "")const = 0;
	virtual ViewerController* cast(ViewerController* obj)const = 0;
};

template<typename T> class ControllerTypeImpl : public ControllerType
{
public:
	virtual ViewerController* allocate(ds::ui::SpriteEngine& g, ci::vec2 size = ci::vec2(-1.f), std::string channel = "")const { return new T(g, size, channel); }
	virtual ViewerController* cast(ViewerController* obj)const { return static_cast<T*>(obj); }
};

class ViewerControllerFactory {
public:

	template<class T = ViewerController>
	static void InitViewerController(ds::ui::SpriteEngine& eng) {
		mEngine = &eng;

		mType = new ControllerTypeImpl<T>();
	}

	template <class Tx = ViewerController>
	static Tx* getInstanceOf(ci::vec2 size, std::string channel = "") {
		if (!mEngine) return nullptr;
		if (mType == nullptr) return nullptr;
		auto channelName = channel;
		if (channel.empty()) {
			channelName = "_default_";
		}
		if (mViewerControllers.find(channelName) == mViewerControllers.end()) {
			auto instance = mType->allocate(*mEngine, size, channel);
			instance->initCreators();
			mViewerControllers[channelName] = instance;
			instance->setDeletingCallback([channelName]() { 
				mViewerControllers.erase(channelName); 
			});
			return dynamic_cast<Tx*>(instance);
		}
		else {
			return dynamic_cast<Tx*>(mViewerControllers[channelName]);
		}
	}

	
	


private:
	ViewerControllerFactory() {};
	static ControllerType* mType;

	static ds::ui::SpriteEngine* mEngine;
	static std::unordered_map<std::string, ViewerController*> mViewerControllers;

};

} // namespace waffles
