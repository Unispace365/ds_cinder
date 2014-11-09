#ifndef DS_UI_SPRITE_SPRITE_H_
#define DS_UI_SPRITE_SPRITE_H_

// ###Cinder includes
#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/Tween.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"
// ###STL includes
#include <list>
#include <exception>
// ###DS includes
#include "ds/app/app_defs.h"
#include "ds/data/user_data.h"
#include "ds/gl/uniform.h"
#include "ds/util/bit_mask.h"
#include "ds/ui/sprite/dirty_state.h"
#include "ds/ui/touch/touch_process.h"
#include "ds/ui/touch/multi_touch_constraints.h"
#include "ds/ui/tween/sprite_anim.h"
#include "ds/ui/sprite/shader/sprite_shader.h"
#include "ds/ui/sprite/util/blend.h"
#include "ds/util/idle_timer.h"
#include "ds/debug/debug_defines.h"

// `ds::` forward declarations
namespace ds {
class BlobReader;
class BlobRegistry;
class CameraPick;
class DataBuffer;
class DrawParams;
class Engine;
class EngineRoot;
class Event;
class UpdateParams;

// `ds::ui::` forward declarations
namespace ui {
struct DragDestinationInfo;
class SpriteEngine;
struct TapInfo;
struct TouchInfo;

// Attribute access, used for network synchronization
extern const char     SPRITE_ID_ATTRIBUTE;

// #Sprite
// `sprite` is the smallest unit of the scene in DS Cinder.
// `sprite`s were meant to abstract away basic functionality
// of various viewable elements such as __positioning__,
// __rotation__, __color__, and etc.
// Every sprite can have multiple children and each sprite
// upon delete, clears all its children.
class Sprite : public SpriteAnimatable
{
public:
// ##makeSprite <small style="color:blue;">static</small>
// ###A Sprite creation convenience
// throws `std::runtime_error` on failure.
// calls `makeAlloc` internally.
static Sprite&		makeSprite(SpriteEngine& aEngine, Sprite* aParent = nullptr);
	
#if (_MSC_VER >= 1800)
// ##make <small style="color:blue;">static</small>
// ###A Sprite creation convenience
// throws `std::runtime_error` on failure.
// Visual Studio 2013+
// The variadic arguments will be passed to the sprite's
// constructor. Specifically aimed for subclasses of the `sprite`
// To make the subclass, `make` friendly, make sure one of the
// constructor overloads accepts the `ds::SpriteEngine&` as its
// first argument
template <typename T, typename... Args>
static T&			make(SpriteEngine& aEngine, Sprite* aParent = nullptr, Args... aArgs);

#else

// ##make <small style="color:blue;">static</small>
// ###A Sprite creation convenience
// throws `std::runtime_error` on failure.
// Visual Studio 2013-
// Specifically aimed for subclasses of the `sprite`
// To make the subclass, `make` friendly, make sure one of the
// constructor overloads accepts the `ds::SpriteEngine&` as its
// first argument
template <typename T>
static T&			make(SpriteEngine& aEngine, Sprite* aParent = nullptr);
#endif

// ##makeAlloc <small style="color:blue;">static</small>
// ###A Sprite creation convenience
// throws `std::runtime_error` on failure.
// Specifically aimed for subclasses of the `sprite` whose allocation
// needs to be of a custom type. E.g: a sprite subclass that does not take
// `ds::SpriteEngine&` as its first argument.
template <typename T>
static T&			makeAlloc(const std::function<T*(void)>& aAllocFn, Sprite* aParent = nullptr);

// ##removeAndDelete <small style="color:blue;">static</small>
// ###Sprite deletion convenience
// Attempts to remove the sprite via `remove` method.
// sets the pointer to `nullptr` upon success.
template <typename T>
static void			removeAndDelete(T *& aSprite);

	// ##Main constructor
	// Takes care of proper initialization of a sprite and its registration
	// with the`engine`. It seeks a unique Sprite `id` upon construction
	// from the `engine` and also initializes all default values for
	// different attributes of a sprite such as color, position and size.
	Sprite(SpriteEngine& aEngine, float aWidth = 0.0f, float aHeight = 0.0f);
	
	// ##destructor <small style="color:blue;">virtual</small>
	// Properly destructs `this` sprite. A lot's going on in this
	// destructor such as calling running animations to stop,
	// de-registering the sprite from the engine and clearing the
	// children of `this` (if any).
	virtual ~Sprite();

	// ##updateClient <small style="color:blue;">virtual</small>
	// Event to be called on every update cycle of the `engine`, running
	// in __Client__ mode for `this` sprite. `override` it to receive
	// its events.
	// > Sprite behavior can vary whether this is running on the server or client mode.
	virtual void		updateClient(const ds::UpdateParams& aParams);

	// ##updateServer <small style="color:blue;">virtual</small>
	// Event to be called on every update cycle of the `engine`, running
	// in __Server__ mode for `this` sprite. `override` it to receive
	// its events.
	// > Sprite behavior can vary whether this is running on the server or client mode.
	virtual void		updateServer(const ds::UpdateParams& aParams);

	// ##drawClient <small style="color:blue;">virtual</small>
	// Draws `this` on screen. Usually `drawClient` is the lowest drawing level of a
	// `sprite`. This method specifically takes care of low-level engine features
	// (e.g: checks if a sprite is `hidden` and returns immediately). Also draws all
	// the children of `this` and calls `drawLocalClient` internally.
	// __This method is guaranteed to be called__ for a sprite. Therefore if you are
	// having issues regarding troubleshooting a sprite not appearing on the screen,
	// overriding this method and setting a breakpoint in it is a good idea.
	virtual void		drawClient(const ci::Matrix44f& aTransMatrix, const DrawParams &aDrawParams);

	// ##drawServer <small style="color:blue;">virtual</small>
	// Same as `drawClient` but for the `Server` mode.
	virtual void		drawServer(const ci::Matrix44f& aTransMatrix, const DrawParams &aDrawParams);

	// ##getId
	// Retrieves the __unique id__ associated to `this` sprite by the `Engine`.
	ds::sprite_id_t		getId() const		{ return mId; }

	// ##getEngine
	// Returns the `ds::ui:SpriteEngine` which is associated with `this` sprite.
	// Basically a reference to the `engine` passed to `this` upon its construction.
	ds::ui::SpriteEngine&
						getEngine();

	// ##setSize
	// Sets the size of this sprite via a supplied `ci::Vec2f`.
	// Internally calls `setSizeAll`.
	void				setSize(const ci::Vec2f& aSize2d);
	
	// ##setSize
	// Sets the size of this sprite via supplied `width` and `height`.
	// Internally calls `setSizeAll`.
	void				setSize(float aWidth, float aHeight);
	
	// ##setSizeAll <small style="color:blue;">virtual</small>
	// subclasses can override this method to control the behavior of sizing.
	// and example of that would be the `text` sprite which subclasses this method
	// to maintain a auto-resize by text feature.
	virtual void		setSizeAll(const ci::Vec3f& aSize3d);

	// ##setSizeAll <small style="color:blue;">virtual</small>
	// subclasses can override this method to control the behavior of sizing.
	// and example of that would be the `text` sprite which subclasses this method
	// to maintain a auto-resize by text feature.
	virtual void		setSizeAll(float aWidth, float aHeight, float aDepth);

	// ##sizeToChildBounds
	// sets the size based on the size of it's immediate children, not recursive
	void				sizeToChildBounds();
	
	// ##getPreferredSize
	// Answer the preferred size for this object. This is intended to be part of a
	// layout pass, so the default preferred size is 0 not width/height. Subclasses
	// need to override this to be meaningful.
	virtual ci::Vec3f	getPreferredSize() const;

	// ##getWidth <small style="color:blue;">virtual</small>
	// returns `width` of `this` sprite. Scale does not accounted for.
	virtual float		getWidth() const;

	// ##getHeight <small style="color:blue;">virtual</small>
	// returns `height` of `this` sprite. Scale does not accounted for.
	virtual float		getHeight() const;

	// ##getDepth
	// returns `depth` of `this` sprite. Scale does not accounted for.
	float				getDepth() const;
	
	// ##getScaleWidth
	// returns `width` of `this` sprite. Scale DOES accounted for.	
	float				getScaleWidth() const;

	// ##getScaleHeight
	// returns `height` of `this` sprite. Scale DOES accounted for.
	float				getScaleHeight() const;
	
	// ##getScaleDepth
	// returns `depth` of `this` sprite. Scale DOES accounted for.	
	float				getScaleDepth() const;

	// ##setPosition
	// sets the position of the sprite __with respect to its parent__ (if any)
	void				setPosition(const ci::Vec3f &aPos);
	
	// ##setPosition
	// sets the position of the sprite __with respect to its parent__ (if any)
	void				setPosition(float x, float y, float z = 0.0f);
	
	// ##getPosition
	// gets the position of the sprite __with respect to its parent__ (if any)
	const ci::Vec3f&	getPosition() const;

	// ##getCenterPosition
	// returns the center position of the sprite __with respect to `this`' current position__
	ci::Vec3f			getCenterPosition() const;

	// ##getLocalCenterPosition
	// returns the center position of the sprite __with respect to itself (top left corner of the sprite)
	// does NOT account for 3d sprites (always returns z as the third component)
	ci::Vec3f			getLocalCenterPosition() const;

	// ##move
	// Moves a sprite by a `delta` vector __based on its current postion__
	void				move(const ci::Vec3f& aDelta);

	// ##move
	// Moves a sprite by some `delta` values __based on its current postion__
	void				move(float aDeltaX, float aDeltaY, float aDeltaZ = 0.0f);

	// ##setScale
	// Sets scale of a sprite via a `ci::Vec3f`
	// scale also applies to the children of `this`
	void				setScale(const ci::Vec3f &aScale);

	// ##setScale
	// Sets scale of a sprite via a `x`, `y`, and `z` components.
	// scale also applies to the children of `this`
	void				setScale(float x, float y, float z = 1.0f);

	// ##getScale
	// gets the current scale being applied to `this` sprite and its children
	const ci::Vec3f&	getScale() const;

	// ##setCenter <small style="color:blue;">virtual</small>
	// center of the Sprite. Where its positioned at and rotated at.
	// A value in the range of [0, 1] for both axis
	virtual void		setCenter(const ci::Vec3f &aCenter);

	// ##setCenter <small style="color:blue;">virtual</small>
	// center of the Sprite. Where its positioned at and rotated at.
	// A value in the range of [0, 1] for both axis
	virtual void		setCenter(float x, float y, float z = 0.0f);

	// ##getCenter <small style="color:blue;">virtual</small>
	// gets the current center of `this`. Where its positioned at and rotated at.
	// A value in the range of [0, 1] for both axis
	const ci::Vec3f&	getCenter() const;

	// ##setRotation
	// Rotates the sprite around the Z axis. Basically this is what
	// you are looking for if you want to rotate a sprite in a 2D screen.
	void				setRotation(float rotZ);

	// ##setRotation
	// Rotates the sprite in 3D space.
	void				setRotation(const ci::Vec3f &aRotation);

	// ##getRotation
	// Gets the current rotation of `this` sprite
	ci::Vec3f			getRotation() const;

	// ##setZLevel
	// Sets the z-index of a sprite
	void				setZLevel(float zlevel);

	// ##getZLevel
	// Gets the z-index of a sprite
	float				getZLevel() const;

	// ##getBoundingBox
	// Returns the area bounded by `this` sprite
	ci::Rectf			getBoundingBox() const;

	// ##getBoundingBox
	// Returns the area bounded by children of `this` sprite
	ci::Rectf			getChildBoundingBox() const;

	// ##setDrawSorted
	// whether to draw be by Sprite order or z level.
	// Only works on a per Sprite base.
	void				setDrawSorted( bool drawSorted );

	// ##getDrawSorted
	// return true if `setDrawSorted` was set to `true`
	bool				getDrawSorted() const;

	// ##getTransform
	// gets transform matrix of `this`
	const ci::Matrix44f&
						getTransform() const;

	// ##getInverseTransform
	// gets inverse transform matrix of `this`
	const ci::Matrix44f&
						getInverseTransform() const;

	// ##getGlobalTransform
	// gets global transform matrix of `this`
	const ci::Matrix44f&
						getGlobalTransform() const;

	// ##getInverseGlobalTransform
	// gets inverse global transform matrix of `this`
	const ci::Matrix44f&
						getInverseGlobalTransform() const;

	// ##getUserData
	ds::UserData&		getUserData();

	// ##getUserData
	const ds::UserData&	getUserData() const;

	// ##addChild
	// Adds another `sprite` as a child of `this` by reference
	void				addChild(Sprite&);

	// ##addChild
	// Adds another `sprite` as a child of `this` by pointer
	// Convenient to use like: `this->addChildPtr(new myOtherSprite(mEngine))`
	template <typename T>
	T*					addChildPtr(T* aSprite);

	// Hack! Hack! Hack to fix crash in AT&T Tech Wall! DO NOT USE THIS FOR ANY OTHER REASON!
	// Jeremy
	void				addChildHack( Sprite &child );

	// ##removeChild
	// removes child from Sprite, but does not delete it.
	void				removeChild( Sprite &child );
	
	// ##removeParent
	// remove child from parent, does not delete.
	void				removeParent();
	
	// ##remove
	// removes Sprite from parent and deletes all children. Does not delete Sprite.
	void				remove();
	
	// ##release
	// OK, sprite/child management has become REALLY messy. And the painful thing
	// is none of the existing functions manage memory -- you still need to delete,
	// and I don't know that anyone does. So this function is intended to remove
	// myself from my parent, delete all children, and delete myself.
	void				release();

	// ##containsChild
	// check to see if Sprite contains child
	bool				containsChild( Sprite *child ) const;
	
	// ##clearChildren
	// removes and deletes all children
	void				clearChildren();
	
	// ##forEachChild
	// iterate through children of `this` sprite and applies a user supplied lambda to each of them
	void				forEachChild(const std::function<void(Sprite&)>&, const bool recurse = false);

	// ##sendToFront
	// sends sprite to front of parents child list.
	void				sendToFront();

	// ##sendToBack
	// sends sprite to back of parents child list.
	void				sendToBack();

	// ##setColor <small style="color:blue;">virtual</small>
	// sets color of `this` sprite, if `setTransparent` is set to false
	// this will be the background paint
	virtual void		setColor(const ci::Color&);

	// ##setColor <small style="color:blue;">virtual</small>
	// sets color of `this` sprite, if `setTransparent` is set to false
	// this will be the background paint
	virtual void		setColor(float r, float g, float b);
	
	// ##setColorA
	// A convenience to set the color and the opacity together
	void				setColorA(const ci::ColorA&);

	// ##getColor
	// returns the current color of `this`, set via `setColor`
	ci::Color			getColor() const;
	
	// ##getColorA
	// returns the current color and opacity of `this`
	ci::ColorA			getColorA() const;

	// ##setOpacity
	// sets the current opacity of `this`
	void				setOpacity( float opacity );

	// ##getOpacity
	// returns the current opacity of `this`
	float				getOpacity() const;

	// ##setTransparent
	// Whether or not to show the entity; does not hide children.
	void				setTransparent(bool transparent);

	// ##getTransparent
	// Whether or not the entity is being shown; does not include children
	bool				getTransparent() const;

	// ##show <small style="color:blue;">virtual</small>
	// will show all children that are visible
	virtual void		show();
	
	// ##hide <small style="color:blue;">virtual</small>
	// will hide all children as well.
	virtual void		hide();

	// ##visible
	// returns the status of `show` or `hide` calls on `this` sprite.
	bool				visible() const;

	// ##eventRecieved <small style="color:blue;">virtual</small>
	// Subclasses can handle the event
	virtual void		eventReceived(const ds::Event&);
	
	// ##parentEventReceived
	// Convenience to pass an event up through my parents
	void				parentEventReceived(const ds::Event&);

	// ##enable
	// check to see if Sprite can be touched
	// use in parallel with `enableMultiTouch` to handle touch events on a sprite
	void				enable(bool flag);

	// ##isEnabled
	// returns `true` if `this` is touchable
	bool				isEnabled() const;

	// ##getParent
	// returns pointer to parent, if any
	Sprite*				getParent() const;

	// ##globalToLocal
	// translates a global point (e.g: outside of `this` and relative to the screen) to the current sprite.
	ci::Vec3f			globalToLocal( const ci::Vec3f &globalPoint );
	
	// ##localToGlobal
	// translates a local point to the global coordinates (e.g: outside of `this` and relative to the screen).
	ci::Vec3f			localToGlobal( const ci::Vec3f &localPoint );

	// ##contains <small style="color:blue;">virtual</small>
	// check if a point is inside the Sprite's bounds.
	virtual bool		contains(const ci::Vec3f& point, const float pad = 0.0f) const;

	// ##getHit
	// finds Sprite at position
	Sprite*				getHit( const ci::Vec3f &point );
	
	// ##getPerspectiveHit
	// finds Sprite at perspective
	Sprite*				getPerspectiveHit(CameraPick&);

	// ##setProcessTouchCallback
	// passes touch info events to a user supplied lambda, use for complex touch events, for
	// simple taps, double taps, and etc. use `set_XX_InfoCallback` methods.
	void				setProcessTouchCallback( const std::function<void (Sprite *, const TouchInfo &)> &func );

	// ##setTapInfoCallback
	// passes a single tap INFO to the user supplied lambda
	void				setTapInfoCallback( const std::function<bool (Sprite *, const TapInfo &)> &func );

	// ##setSwipeCallback
	// passes a single swipe to the user supplied lambda (swipe threshold is configurable in the config files)
	void				setSwipeCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func );

	// ##setTapCallback
	// passes a single tap position to the user supplied lambda
	void				setTapCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func );

	// ##setDoubleTapCallback
	// passes a double tap position to the user supplied lambda
	void				setDoubleTapCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func );
	
	// ##setDragDestinationCallback
	// passes a drag info struct to the user supplied lambda
	void				setDragDestinationCallback( const std::function<void (Sprite *, const DragDestinationInfo &)> &func );

	// ##setTouchScaleMode
	// true will size the sprite using `setSize` on a touch scale gesture
	// false (the default) will scale the sprite using `setScale` on a touch scale gesture.
	// `MULTITOUCH_CAN_SCALE` has to be a touch flag as well as the sprite being enabled to take effect
	void					setTouchScaleMode(bool doSizeScale){ mTouchScaleSizeMode = doSizeScale; };

	// ##enableMultiTouch
	// enables different multi touch flags for `this` sprite such as `ds::ui::MULTITOUCH_CAN_SCALE`, etc.
	// Constraints defined in multi_touch_constraints.h
	// `ds::ui::MULTITOUCH_XX` style
	void				enableMultiTouch(const BitMask &);
	
	// ##disableMultiTouch
	// disables all multi touch flags
	void				disableMultiTouch();

	// ##multiTouchEnabled
	// whether if a multi touch flag has already been set
	bool				multiTouchEnabled() const;

	// ##hasMultiTouchConstraint
	// checks to see a certain multi touch flag is set or not
	bool				hasMultiTouchConstraint( const BitMask &constraint = MULTITOUCH_NO_CONSTRAINTS ) const;

	// ##callAfterDelay
	void				callAfterDelay(const std::function<void(void)>&, const float delay_in_seconds);

	// ##cancelDelayedCall
	void				cancelDelayedCall();

	// ##inBounds
	// check if sprite is in bounds of the viewable area
	bool				inBounds() const;

	// ##setCheckBounds
	void				setCheckBounds(bool checkBounds);
	
	// ##getCheckBounds
	// returns `true` if it needs to be checked in for bounds
	bool				getCheckBounds() const;

	// ##isLoaded <small style="color:blue;">virtual</small>
	// intended for subclasses to `override` this to perform
	// custom loading mechanism
	virtual bool		isLoaded() const;

	// ##setDragDestination
	void				setDragDestination(Sprite *dragDestination);
	
	// ##setDragDestination
	void				setDragDestiantion(Sprite *dragDestination);
	
	// ##getDragDestination
	Sprite*				getDragDestination() const;

	// ##isDirty
	// Whether if the sync status is dirty (needs to be synced over network)
	bool				isDirty() const;

	// ##writeTo
	void				writeTo(ds::DataBuffer&);
	
	// ##readFrom
	void				readFrom(ds::BlobReader&);
	
	// ##writeClientTo
	// Only used when running in client mode
	void				writeClientTo(ds::DataBuffer&) const;
	
	// ##readClientFrom <small style="color:blue;">virtual</small>
	virtual void		readClientFrom(ds::DataBuffer&) { }

	// ##setBlendMode
	void				setBlendMode(const BlendMode &blendMode);
	
	// ##getBlendMode
	BlendMode			getBlendMode() const;

	// ##setBaseShader
	void				setBaseShader(const std::string &location, const std::string &shadername, bool applyToChildren = false);
	
	// ##getBaseShader
	SpriteShader&		getBaseShader();
	
	// ##getBaseShaderName
	std::string			getBaseShaderName() const;
	
	// ##getUniform
	// get uniform property of a shader set to this sprite
	ds::gl::Uniform&	getUniform();

	// ##setClipping
	// whether if children should be shown outside of the bounds of `this` or not
	void				setClipping(bool flag);

	// ##getClipping
	// whether if children is being shown outside of the bounds of `this` or not
	bool				getClipping() const;

	// ##userInputReceived <small style="color:blue;">virtual</small>
	virtual void		userInputReceived();

	// ##setSecondBeforeIdle
	void				setSecondBeforeIdle( const double );
	
	// ##secondsToIdle
	double				secondsToIdle() const;
	
	// ##isIdling
	bool				isIdling() const;
	
	// ##startIdling
	void				startIdling();
	
	// ##resetIdleTimer
	void				resetIdleTimer();
	
	// ##clear
	// clears the idle timer, not the whole sprite or anything
	void				clear();

	// ##setNoReplicationOptimization
	// Prevent this sprite (and all children) from replicating. NOTE: Should
	// only be done once on construction, if you change it, weird things could happen.
	void				setNoReplicationOptimization(const bool = false);
	
	// ##markTreeAsDirty
	// Special function to mark every sprite from me down as dirty.
	void				markTreeAsDirty();

	// ##setRotateTouches
	// When true, the touch input is automatically rotated to account for my rotation.
	void				setRotateTouches(const bool = false);
	
	// ##isRotateTouches
	bool				isRotateTouches() const;

	// ##getPerspective
	bool				getPerspective() const;
	
	// ##setIsInScreenCoordsHack
	// Total hack resulting from my unfamiliarity with 3D systems. This can sometimes be necessary for
	// views that are inside of perspective cameras, but are expressed in screen coordinates.
	void				setIsInScreenCoordsHack(const bool);

	// ##setUseDepthBuffer
	void				setUseDepthBuffer(bool useDepth);
	
	// ##getUseDepthBuffer
	bool				getUseDepthBuffer() const;

	// ##setCornerRadius
	// If this sprite render locally and the radius is > 0, will draw a rounded rect
	void				setCornerRadius(const float newRadius);
	
	// ##getCornerRadius
	float				getCornerRadius() const;

	// ##hasTouches
	// Answer true if this sprite currently has any touches.
	bool				hasTouches() const;
	
	// ##passTouchToSprite
	// must be passed inside handle touch Moved or else will result in an infinite loop.
	void				passTouchToSprite(Sprite *destinationSprite, const TouchInfo &touchInfo);

	// ##postAppSetup
	// A hack needed by the engine, which constructs root types
	// before the blobs are assigned
	void				postAppSetup();

protected:
	//###friends
	friend class        TouchManager;
	friend class        TouchProcess;

	//###concrete methods
	void				swipe(const ci::Vec3f &swipeVector);
	bool				tapInfo(const TapInfo&);
	void				tap(const ci::Vec3f &tapPos);
	void				doubleTap(const ci::Vec3f &tapPos);
	void				dragDestination(Sprite *sprite, const DragDestinationInfo &dragInfo);
	void				processTouchInfo( const TouchInfo &touchInfo );
	void				processTouchInfoCallback( const TouchInfo &touchInfo );
	void				buildTransform() const;
	void				buildGlobalTransform() const;
	bool				hasDoubleTap() const;
	bool				hasTap() const;
	bool				hasTapInfo() const;
	void				updateCheckBounds() const;
	bool				checkBounds() const;

	// ##getInnerHit <small style="color:blue;">virtual</small>
	// Once the sprite has passed the getHit() sprite bounds, this is a second
	// stage that allows the sprite itself to determine if the point is interior,
	// in the case that the sprite has transparency or other special rules.
	virtual bool		getInnerHit(const ci::Vec3f&) const;

	// ## doSetPosition <small style="color:blue;">virtual</small>
	virtual void		doSetPosition(const ci::Vec3f&);
	
	// ## doSetScale <small style="color:blue;">virtual</small>
	virtual void		doSetScale(const ci::Vec3f&);
	
	// ## doSetRotation <small style="color:blue;">virtual</small>
	virtual void		doSetRotation(const ci::Vec3f&);

	// ## onCenterChanged <small style="color:blue;">virtual</small>
	virtual void		onCenterChanged();
	
	// ## onPositionChanged <small style="color:blue;">virtual</small>
	virtual void		onPositionChanged();
	
	// ## onScaleChanged <small style="color:blue;">virtual</small>
	virtual void		onScaleChanged();
	
	// ## onSizeChanged <small style="color:blue;">virtual</small>
	virtual void		onSizeChanged();

	// ## drawLocalClient <small style="color:blue;">virtual</small>
	virtual void		drawLocalClient();
	
	// ## drawLocalServer <small style="color:blue;">virtual</small>
	virtual void		drawLocalServer();

	// Always access the bounds via this, which will build them if necessary
	const ci::Rectf&	getClippingBounds();
	void				computeClippingBounds();

	void				setSpriteId(const ds::sprite_id_t&);
	
	// ##setFlag
	// Helper utility to set a flag
	void				setFlag(const int newBit, const bool on, const DirtyState&, int& oldFlags);
	bool				getFlag(const int bit, const int flags) const;

	// ## markAsDirty <small style="color:blue;">virtual</small>
	virtual void		markAsDirty(const DirtyState&);
	
	// ## markChildrenAsDirty <small style="color:blue;">virtual</small>
	// Special function that marks all children as dirty, without sending anything up the hierarchy.
	virtual void		markChildrenAsDirty(const DirtyState&);
	
	// ## writeAttributesTo <small style="color:blue;">virtual</small>
	virtual void		writeAttributesTo(ds::DataBuffer&);
	
	// ## writeClientAttributesTo <small style="color:blue;">virtual</small>
	// Used during client mode, to let clients get info back to the server. Use the
	// engine_io.defs::ScopedClientAtts at the top of the function to do all the boilerplate.
	virtual void		writeClientAttributesTo(ds::DataBuffer&) const { }
	
	// ## readAttributeFrom <small style="color:blue;">virtual</small>
	// Read a single attribute
	virtual void		readAttributeFrom(const char attributeId, ds::DataBuffer&);

	void				setUseShaderTextuer(bool flag);
	bool				getUseShaderTextuer() const;

	void				sendSpriteToFront(Sprite &sprite);
	void				sendSpriteToBack(Sprite &sprite);

	void				setPerspective(const bool);

	mutable bool		mBoundsNeedChecking;
	mutable bool		mInBounds;


	SpriteEngine&		mEngine;
	// The ID must always be assigned through setSpriteId(), which has some
	// behaviour associated with the ID changing.
	ds::sprite_id_t		mId;
	ci::Color8u			mUniqueColor;

	float				mWidth,
						mHeight,
						mDepth;

	mutable ci::Matrix44f
						mTransformation;
	mutable ci::Matrix44f
						mInverseTransform;
	mutable bool		mUpdateTransform;

	int                 mSpriteFlags;
	ci::Vec3f           mPosition,
						mCenter,
						mScale,
						mRotation;
	float				mZLevel;
	float				mOpacity;
	ci::Color			mColor;
	ci::Rectf			mClippingBounds;
	bool				mClippingBoundsDirty;
	SpriteShader		mSpriteShader;

	mutable ci::Matrix44f
						mGlobalTransform;
	mutable ci::Matrix44f
						mInverseGlobalTransform;

	ds::UserData		mUserData;

	Sprite*				mParent;
	std::vector<Sprite *>
						mChildren; 
	// A cache for when I need to sort my children. This could be
	// a lot more efficient, only running the sort when Z changes.
	std::vector<Sprite*>
						mSortedTmp;

	// Class-unique key for this type.  Subclasses can replace.
	char				mBlobType;
	DirtyState			mDirty;

	std::function<void (Sprite *, const TouchInfo &)> mProcessTouchInfoCallback;
	std::function<void (Sprite *, const ci::Vec3f &)> mSwipeCallback;
	std::function<bool (Sprite *, const TapInfo &)> mTapInfoCallback;
	std::function<void (Sprite *, const ci::Vec3f &)> mTapCallback;
	std::function<void (Sprite *, const ci::Vec3f &)> mDoubleTapCallback;
	std::function<void (Sprite *, const DragDestinationInfo &)> mDragDestinationCallback;

	bool				mMultiTouchEnabled;
	BitMask				mMultiTouchConstraints;
	bool				mTouchScaleSizeMode;

	// All touch processing happens in the process touch class
	TouchProcess		mTouchProcess;

	bool				mCheckBounds;
	Sprite*				mDragDestination;
	IdleTimer			mIdleTimer;
	bool				mUseDepthBuffer;
	float				mCornerRadius;
	// Hack for clients that do their own drawing -- this is the current parent * me opacity.
	// Essentially anyone who sets alpha in drawLocalClient should probably use this value.
	// This is a hack because this value should really be supplied as an argument to drawLocalClient().
	float				mDrawOpacityHack;
	
	// Transport uniform data to the shader
	ds::gl::Uniform		mUniform;

private:
	// Utility to reorder the sprites
	void				setSpriteOrder(const std::vector<sprite_id_t>&);

	friend class Engine;
	friend class EngineRoot;
	// Disable copy constructor; sprites are managed by their parent and
	// must be allocated
	Sprite(const Sprite&);
	// Internal constructor just for the Engine, used to create the root sprite,
	// which always exists and is identical across all architectures.
	Sprite(SpriteEngine&, const ds::sprite_id_t id, const bool perspective = false);

	void				init(const ds::sprite_id_t);
	void				readAttributesFrom(ds::DataBuffer&);

	void				dimensionalStateChanged();
	// Applies to all children, too.
	void				markClippingDirty();
	// Store all children in mSortedTmp by z order.
	// XXX Need to optimize this so only built when needed.
	void				makeSortedChildren();
	// calls removeParent then addChild to parent.
	// setParent was previously public, but calling it by itself can cause an infinite loop
	// Use addChild() from outside sprite.cpp
	void				setParent(Sprite *parent);

	ci::gl::Texture		mRenderTarget;
	BlendMode			mBlendMode;

	//set flag for determining whether to use orthoganol or perspective.
	// this flag is only set on the root perspective sprite.
	bool				mPerspective;

	//set by sprite constructors. doesn't need to be passed through.
	bool				mUseShaderTexture; 

	ci::ColorA			mServerColor;
	// This to make onSizeChanged() more efficient -- it can get
	// triggered as a result of position changes, which shouldn't affect it.
	float				mLastWidth, mLastHeight;

	// Total hack needed in certain cases where you're using a perspective camera.
	// This is used by the picking to let the touch system know that the sprite
	// (position/dimensions) are in the screen coordinate space.
	bool				mIsInScreenCoordsHack;

	// Store a CueRef from the cinder timeline to clear the callAfterDelay() function
	// Cleared automatically on destruction
	ci::CueRef			mDelayedCallCueRef;

public:
	// This is a bit of a hack so I can temporarily set a scale value
	// without causing the whole editing mechanism to kick in.
	// Upon destruction, this class restores the scale.
	class LockScale {
	public:
		LockScale(Sprite&, const ci::Vec3f& temporaryScale);
		~LockScale();

	private:
		Sprite&			mSprite;
		const ci::Vec3f	mScale;
	};

#ifdef _DEBUG
	// Debugging aids to write out my state. write() calls writeState
	// on me and all my children.
	void				write(std::ostream&, const size_t tab) const;
	virtual void		writeState(std::ostream&, const size_t tab) const;
#endif

public:
	static void			installAsServer(ds::BlobRegistry&);
	static void			installAsClient(ds::BlobRegistry&);

	template <typename T>
	static void			handleBlobFromServer(ds::BlobReader&);
	static void			handleBlobFromClient(ds::BlobReader&);
};

#if (_MSC_VER >= 1800)

template <typename T, typename... Args>
static T& Sprite:: make(SpriteEngine& e, Sprite* parent, Args... args)
{
  T*                    s = new T(e, args...);
  if (!s) throw std::runtime_error("Can't create sprite");
  if (parent) parent->addChild(*s);
  return *s;
}

#else

template <typename T>
static T& Sprite::make(SpriteEngine& e, Sprite* parent)
{
	T*                    s = new T(e);
	if (!s) throw std::runtime_error("Can't create sprite");
	if (parent) parent->addChild(*s);
	return *s;
}

#endif

template <typename T>
T* Sprite::addChildPtr(T* aSprite) {
	if (!aSprite) return nullptr;
	addChild(*aSprite);
	return aSprite;
}

template <typename T>
static T& Sprite:: makeAlloc(const std::function<T*(void)>& allocFn, Sprite* parent)
{
  T*                    s = allocFn();
  if (!s) throw std::runtime_error("Can't create sprite");
  if (parent) parent->addChild(*s);
  return *s;
}

template <typename T>
void Sprite::removeAndDelete( T *&sprite )
{
  if (!sprite)
	return;

  sprite->remove();
  delete sprite;
  sprite = nullptr;
}

template <typename T>
static void Sprite::handleBlobFromServer(ds::BlobReader& r)
{
  ds::DataBuffer&       buf(r.mDataBuffer);
  if (buf.read<char>() != SPRITE_ID_ATTRIBUTE) return;
  ds::sprite_id_t       id = buf.read<ds::sprite_id_t>();
  Sprite*               s = r.mSpriteEngine.findSprite(id);
  if (s) {
	s->readFrom(r);
  } else if ((s = new T(r.mSpriteEngine)) != nullptr) {
	s->setSpriteId(id);
	s->readFrom(r);
	// If it didn't get assigned to a parent, something is wrong,
	// and it would disappear forever from memory management if I didn't
	// clean up here.
	if (!s->mParent) {
	  assert(false);
	  delete s;
	}
  }
}

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_SPRITE_H_
