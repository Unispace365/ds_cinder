#ifndef DS_UI_SPRITE_SPRITE_H_
#define DS_UI_SPRITE_SPRITE_H_

// Cinder includes
#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/Tween.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"
// STL includes
#include <list>
#include <exception>
// DS includes
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

namespace ui {
	class SpriteEngine;
	struct DragDestinationInfo;
	struct TapInfo;
	struct TouchInfo;

	// Attribute access
	extern const char     SPRITE_ID_ATTRIBUTE;

	/**
	 \class Sprite
	 \brief The root unit for almost any on-screen element. 
		- UI elements extend from Sprite to participate in the display hierarchy, drawing transformations, built-in tweens, touch picking, etc.
		- Sprite can have children, which will be positioned in local space relative to the parent sprite.
		- Sprite will release all children when removing. Use release() to remove the sprite from it's parent and delete it.
		- The base Sprite class can be set to display a solid rectangle, assuming it has a size, is non-transparent, and has a color.
		- Sprites can be animated using the SpriteAnimatable functions to gracefully change size, position, opacity, scale, color, and rotation.
		- Sprites can clip their children along their bounds using setClipping(true)	 */
	class Sprite : public SpriteAnimatable
	{
	public:

		/** Generic sprite creation function.
			The variadic args will be passed in the same order to your Sprite's constructor.
			\param engine The SpriteEngine for your app.
			\param parent An optional parent for the new sprite to be added to as a child.
			\param args Parameter arguments that are passed to your custom sprite type, in order, on construction		*/
		template <typename T, typename... Args>
		static T&				make(SpriteEngine& engine, Sprite* parent = nullptr, Args... args);

		/** Sprite creation convenience, throw on failure.
			\param allocFn A function to be called to create the sprite. See makeSprite() for an example.
			\param parent An optional parent for the new sprite to be added to as a child.		*/
		template <typename T>
		static T&				makeAlloc(const std::function<T*(void)>& allocFn, Sprite* parent = nullptr);

		/** Sprite creation convenience for this basic sprite type, throws on failure.
			\param engine The SpriteEngine for your app.
			\param parent An optional parent for the new sprite to be added to as a child.		*/
		static Sprite&			makeSprite(SpriteEngine& engine, Sprite* parent = nullptr){ return makeAlloc<Sprite>([&engine]()->Sprite*{return new Sprite(engine); }, parent); }

		/** Constructor for Sprite.
			\param engine The SpriteEngine for your app.
			\param width Initial horizontal size of the sprite.
			\param height Initial vertical size of the sprite.		*/
		Sprite(SpriteEngine& engine, float width = 0.0f, float height = 0.0f);
		virtual ~Sprite();

		/** Update function for when this app is set to be a client.
			Sprite behaviour can vary whether this is running on the server or client, and you can hook into that here.
			\param updateParams UpdateParams containing some conveniences such as delta time.		*/
		virtual void			updateClient(const ds::UpdateParams& updateParams);

		/** Update function for when this app is set to be a server.
			Sprite behaviour can vary whether this is running on the server or client, and you can hook into that here.
			In most cases, you'll override this function for logic on updates, rather than updateClient()
			\param updateParams UpdateParams containing some conveniences such as delta time.		*/
		virtual void			updateServer(const ds::UpdateParams& updateParams);

		/** Draw function for when this app is set to be a client.
			In most cases, you'll want to override drawLocalClient() to do custom drawing, as this function handles drawing for children as well.
			\param transformMatrix The transform matrix of the parent.
			\param drawParams Parameters for drawing, such as the opacity of the parent.		*/
		virtual void			drawClient(const ci::Matrix44f &transformMatrix, const DrawParams &drawParams);

		/** Draw function for when this app is set to be a server.
			In most cases, you'll want to override drawLocalClient() to do custom drawing, as this function handles drawing for children as well.
			\param transformMatrix The transform matrix of the parent.
			\param drawParams Parameters for drawing, such as the opacity of the parent.		*/
		virtual void			drawServer(const ci::Matrix44f &transformMatrix, const DrawParams &drawParams);

		/** Returns the unique id for this Sprite. The SpriteEngine will automatically generate an id for the sprite when constructed.
			We recommend you use references or pointers to keep track of sprites, rather than looking up by Id.
			\return Unique sprite id.		*/
		ds::sprite_id_t			getId() const { return mId; }

		/** Gets the SpriteEngine that was passed to this Sprite upon construction.
			\return The app's SpriteEngine.		*/
		ds::ui::SpriteEngine&	getEngine() { return mEngine; }

		/** Get the width, height, and depth of this Sprite. Convenience for getWidth(), getHeight() and getDepth().
			\return Returns a 3-dimensional vector equivalent to ci::Vec3f(width, height, depth).		*/
		const ci::Vec3f			getSize() const;

		/** Sets the width and height of the Sprite. 
			This does not affect the scale of the Sprite. Many subclasses set the size of the Sprite themselves, such as Image and Text.
			\param size2d The size to set in the form of ci::Vec2f(width, height).		*/
		void					setSize(const ci::Vec2f& size2d);

		/** Sets the width and height of the Sprite.
			This does not affect the scale of the Sprite.
			Many subclasses set the size of the Sprite themselves, such as Image and Text, and in those cases you should not call this function yourself.
			\param width The new width of the Sprite.
			\param height The new height of the Sprite.		*/
		void					setSize(float width, float height);

		/** Sets the width, height, and depth of the Sprite.
			This does not affect the scale of the Sprite.
			Many subclasses set the size of the Sprite themselves, such as Image and Text, and in those cases you should not call this function yourself.
			This is identical to setSize(), except it also sets the depth.
			\param size3d The size to set in the form of ci::Vec2f(width, height, depth).		*/
		virtual void			setSizeAll(const ci::Vec3f& size3d);

		/** Sets the width, height, and depth of the Sprite.
			This does not affect the scale of the Sprite.
			Many subclasses set the size of the Sprite themselves, such as Image and Text, and in those cases you should not call this function yourself.
			This is identical to setSize(), except it also sets the depth.
			\param width The new width of the Sprite.
			\param height The new height of the Sprite.
			\param depth The new depth of the Sprite.		*/
		virtual void			setSizeAll(float width, float height, float depth);

		/** Sets the size based on the boundaries of this Sprite's immediate children, not recursive		*/
		void					sizeToChildBounds();

		/** Answer the preferred size for this object. 
			This is intended to be part of a layout pass, so the default preferred size is 0 not width/height. 
			Subclasses need to override this to be meaningful.
			\return The 3d vector of the size this sprite should be. Override this function and return a meaningful value.		 */
		virtual ci::Vec3f		getPreferredSize() const;

		/** The width of this sprite, not including scale.
			For instance, an Image Sprite will always return the width of the image from this function, even if the Sprite has been scaled.
			\return The width in pixels of this Sprite.		*/
		virtual float			getWidth() const;

		/** The height of this sprite, not including scale.
			For instance, an Image Sprite will always return the height of the image from this function, even if the Sprite has been scaled.
			\return The height in pixels of this Sprite.		*/
		virtual float			getHeight() const;

		/** The depth of this sprite, not including scale.
			For instance, an Image Sprite will always return the height of the image from this function, even if the Sprite has been scaled.
			\return The height in pixels of this Sprite.		*/
		float					getDepth() const;

		/** The local display width of this sprite.
			Assuming all parents are scale=1.0, this will return the number of pixels this Sprite displays.
			\return The display width of this Sprite, width * scale.x		*/
		float					getScaleWidth() const;

		/** The local display height of this sprite.
			Assuming all parents are scale=1.0, this will return the number of pixels this Sprite displays.
			\return The display height of this Sprite, height * scale.y		*/
		float					getScaleHeight() const;

		/** The local display depth of this sprite.
			Assuming all parents are scale=1.0, this will return the number of pixels this Sprite displays.
			\return The display depth of this Sprite, depth * scale.z		*/
		float					getScaleDepth() const;

		/** Set the position of the Sprite in local space (the parent's relative co-ordinates).
			For perspective Sprites, the y position is inverted, so greater y values will move upwards.
			For ortho Sprites, positive y position is downwards.
			Z position forwards or backwards depends on your camera setup.
			\param pos The 3d vector of the new position, in pixels. */
		void					setPosition(const ci::Vec3f &pos);

		/** Set the position of the Sprite in local space (the parent's relative co-ordinates).
			For perspective Sprites, the y position is inverted, so greater y values will move upwards.
			For ortho Sprites, positive y position is downwards.
			Z position forwards or backwards depends on your camera setup.
			\param x The x (horizontal) position, in pixels.
			\param y The y (vertical) position, in pixels.
			\param z The z (depth) position, in pixels. */
		void					setPosition(float x, float y, float z = 0.0f);

		/** Get the position of the Sprite in local space (the parent's relative co-ordinates).
			For perspective Sprites, the y position is inverted, so greater y values will move upwards.
			For ortho Sprites, positive y position is downwards.
			Z position forwards or backwards depends on your camera setup.
			\return The 3d vector of the new position, in pixels. */
		const ci::Vec3f&		getPosition() const;

		/** The center of the Sprite, in the parent's co-ordinate space. See getCenter() and setCenter().
			Effectively mPosition + getLocalCenterPosition();
			\return 3d Vector of the pixel position of the center of this Sprite. */
		ci::Vec3f				getCenterPosition() const;

		/** The center of the Sprite, in this Sprite's co-ordinate space. See getCenter() and setCenter().
			Effectively ci::Vec3f(floorf(mWidth/2.0f), floorf(mHeight/2.0f), mPosition.z);
			\return 3d Vector of the pixel position of the center of this Sprite, in local space. */
		ci::Vec3f				getLocalCenterPosition() const;

		void					move(const ci::Vec3f &delta);
		void					move(float deltaX, float deltaY, float deltaZ = 0.0f);

		void					setScale(const ci::Vec3f &scale);
		void					setScale(float x, float y, float z = 1.0f);
		void					setScale(float scale);
		const ci::Vec3f&		getScale() const;

		// center of the Sprite. Where its positioned at and rotated at.
		virtual void			setCenter(const ci::Vec3f &center);
		virtual void			setCenter(float x, float y, float z = 0.0f);
		const ci::Vec3f&		getCenter() const;

		void					setRotation(float rotZ);
		void					setRotation(const float xRot, const float yRot, const float zRot);
		void					setRotation(const ci::Vec3f &rot);
		ci::Vec3f				getRotation() const;

		// \note This ZLevel is not analogous to 3D position z value at all.
		// This value affects drawing order of sprites. The higher this value is,
		// the later this sprite gets drawn.
		// \see Sprite::drawServer
		void					setZLevel(float zlevel);
		float					getZLevel() const;

		ci::Rectf				getBoundingBox() const;
		ci::Rectf				getChildBoundingBox() const;

		// whether to draw be by Sprite order or z level.
		// Only works on a per Sprite base.
		void					setDrawSorted(bool drawSorted);
		bool					getDrawSorted() const;

		const ci::Matrix44f&	getTransform() const;
		const ci::Matrix44f&	getInverseTransform() const;
		const ci::Matrix44f&	getGlobalTransform() const;
		const ci::Matrix44f&	getInverseGlobalTransform() const;

		ds::UserData&			getUserData();
		const ds::UserData&		getUserData() const;

		void					addChild(Sprite&);

		template<class T>
		T*						addChildPtr(T* e){
			if(!e) return nullptr;
			addChild(*e);
			return e;
		}

		// removes child from Sprite, but does not delete it.
		void					removeChild(Sprite &child);
		// remove child from parent, does not delete.
		void					removeParent();
		// removes Sprite from parent and deletes all children. Does not delete Sprite.
		void					remove();
		// OK, sprite/child management has become REALLY messy. And the painful thing
		// is none of the existing functions manage memory -- you still need to delete,
		// and I don't know that anyone does. So this function is intended to remove
		// myself from my parent, delete all children, and delete myself.
		void					release();

		// check to see if Sprite contains child
		bool					containsChild(Sprite *child) const;
		// removes and deletes all children
		void					clearChildren();
		void					forEachChild(const std::function<void(Sprite&)>&, const bool recurse = false);

		// sends sprite to front of parents child list.
		void					sendToFront();
		// sends sprite to back of parents child list.
		void					sendToBack();

		virtual void			setColor(const ci::Color&);
		virtual void			setColor(float r, float g, float b);
		// A convenience to set the color and the opacity
		void					setColorA(const ci::ColorA&);
		ci::Color				getColor() const;
		ci::ColorA				getColorA() const;

		void					setOpacity(float opacity);
		float					getOpacity() const;

		// Whether or not to show the entity; does not hide children.
		void					setTransparent(bool transparent);
		bool					getTransparent() const;

		// will show all children that are visible
		virtual void			show();
		// will hide all children as well.
		virtual void			hide();

		bool					visible() const;

		// Subclasses can handle the event
		virtual void			eventReceived(const ds::Event&){};
		// Convenience to pass an event up through my parents
		void					parentEventReceived(const ds::Event&);

		// check to see if Sprite can be touched
		void					enable(bool flag);
		bool					isEnabled() const;

		Sprite*					getParent() const;

		ci::Vec3f				globalToLocal(const ci::Vec3f &globalPoint);
		ci::Vec3f				localToGlobal(const ci::Vec3f &localPoint);

		// check if a point is inside the Sprite's bounds.
		virtual bool			contains(const ci::Vec3f& point, const float pad = 0.0f) const;

		// finds Sprite at position
		Sprite*					getHit(const ci::Vec3f &point);
		Sprite*					getPerspectiveHit(CameraPick&);

		void					setProcessTouchCallback(const std::function<void(Sprite *, const TouchInfo &)> &func);
		// Stateful tap is the new-style and should be preferred over others.  It lets you
		// know the state of the tap in addition to the count.  It also lets clients cancel
		// a tap if, for example, they only want to handle single taps.  Answer true as
		// long as you want to keep checking for a tap, false otherwise.
		void					setTapInfoCallback(const std::function<bool(Sprite *, const TapInfo &)> &func);
		void					setSwipeCallback(const std::function<void(Sprite *, const ci::Vec3f &)> &func);
		void					setTapCallback(const std::function<void(Sprite *, const ci::Vec3f &)> &func);
		void					setDoubleTapCallback(const std::function<void(Sprite *, const ci::Vec3f &)> &func);
		void					setDragDestinationCallback(const std::function<void(Sprite *, const DragDestinationInfo &)> &func);

		// true will size the sprite using setSize() on a touch scale gesture
		// false (the default) will scale the sprite using setScale9) on a touch scale gesture.
		// MULTITOUCH_CAN_SCALE has to be a touch flag as well as the sprite being enabled to take effect
		void					setTouchScaleMode(bool doSizeScale);;

		// Constraints defined in multi_touch_constraints.h
		// ds::ui::MULTITOUCH_XX
		void					enableMultiTouch(const BitMask &);
		void					disableMultiTouch();
		bool					multiTouchEnabled() const;
		bool					hasMultiTouchConstraint(const BitMask &constraint = MULTITOUCH_NO_CONSTRAINTS) const;

		void					callAfterDelay(const std::function<void(void)>&, const float delay_in_seconds);
		void					cancelDelayedCall();

		bool					inBounds() const;
		void					setCheckBounds(bool checkBounds);
		bool					getCheckBounds() const;
		virtual bool			isLoaded() const;
		void					setDragDestination(Sprite *dragDestination);
		void					setDragDestiantion(Sprite *dragDestination);
		Sprite*					getDragDestination() const;

		bool					isDirty() const;
		void					writeTo(ds::DataBuffer&);
		void					readFrom(ds::BlobReader&);
		// Only used when running in client mode
		void					writeClientTo(ds::DataBuffer&) const;
		// IMPORTANT: readClientFrom must not be virtual. If any of the clients try to communicate
		// back with server, the communication must happen through "readClientAttributeFrom".
		void					readClientFrom(ds::DataBuffer&);

		void					setBlendMode(const BlendMode &blendMode);
		BlendMode				getBlendMode() const;

		void					setBaseShader(const std::string &location, const std::string &shadername, bool applyToChildren = false);
		SpriteShader&			getBaseShader();
		std::string				getBaseShaderName() const;
		ds::gl::Uniform&		getUniform();

		void					setClipping(bool flag);
		bool					getClipping() const;

		virtual void			userInputReceived();
		void					setSecondBeforeIdle(const double);
		double					secondsToIdle() const;
		bool					isIdling() const;
		void					startIdling();
		void					resetIdleTimer();
		// clears the idle timer, not the whole sprite or anything
		void					clear();

		// Prevent this sprite (and all children) from replicating. NOTE: Should
		// only be done once on construction, if you change it, weird things could happen.
		void					setNoReplicationOptimization(const bool = false);
		// Special function to mark every sprite from me down as dirty.
		void					markTreeAsDirty();

		// When true, the touch input is automatically rotated to account for my rotation.
		void					setRotateTouches(const bool = false);
		bool					isRotateTouches() const;

		bool					getPerspective() const;
		// Total hack resulting from my unfamiliarity with 3D systems. This can sometimes be necessary for
		// views that are inside of perspective cameras, but are expressed in screen coordinates.
		void					setIsInScreenCoordsHack(const bool);

		void					setUseDepthBuffer(bool useDepth);
		bool					getUseDepthBuffer() const;

		// If this sprite renders locally and the radius is > 0, will draw a rounded rect
		void					setCornerRadius(const float newRadius);
		float					getCornerRadius() const;

		// Answer true if this sprite currently has any touches.
		bool					hasTouches() const;
		/*
		 * \brief must be passed inside handle touch Moved or else will result in an infinite loop.
		 */
		void					passTouchToSprite(Sprite *destinationSprite, const TouchInfo &touchInfo);

		// A hack needed by the engine, which constructs root types
		// before the blobs are assigned
		void					postAppSetup();

	protected:
		friend class        TouchManager;
		friend class        TouchProcess;

		void				swipe(const ci::Vec3f &swipeVector);
		bool				tapInfo(const TapInfo&);
		void				tap(const ci::Vec3f &tapPos);
		void				doubleTap(const ci::Vec3f &tapPos);
		void				dragDestination(Sprite *sprite, const DragDestinationInfo &dragInfo);
		void				processTouchInfo(const TouchInfo &touchInfo);
		void				processTouchInfoCallback(const TouchInfo &touchInfo);

		void				buildTransform() const;
		void				buildGlobalTransform() const;
		virtual void		drawLocalClient();
		virtual void		drawLocalServer();
		bool				hasDoubleTap() const;
		bool				hasTap() const;
		bool				hasTapInfo() const;
		void				updateCheckBounds() const;
		bool				checkBounds() const;

		// Once the sprite has passed the getHit() sprite bounds, this is a second
		// stage that allows the sprite itself to determine if the point is interior,
		// in the case that the sprite has transparency or other special rules.
		virtual bool		getInnerHit(const ci::Vec3f&) const;

		virtual void		doSetPosition(const ci::Vec3f&);
		virtual void		doSetScale(const ci::Vec3f&);
		virtual void		doSetRotation(const ci::Vec3f&);
		void				doPropagateVisibilityChange(bool before, bool after);

		virtual void		onCenterChanged(){}
		virtual void		onPositionChanged(){}
		virtual void		onScaleChanged(){}
		virtual void		onSizeChanged(){}
		virtual void		onChildAdded(Sprite& child){}
		virtual void		onChildRemoved(Sprite& child){}
		// Note: there's a reason this is not called onVisibilityChanged().
		// TLDR;the visible flag arg here is NOT equal to Sprite::visible()
		// The reason is that,  the final visibility of a sprite is decided
		// by the visibility of itself and its parents.  the "visible" flag
		// here is the result of the calculation based on calls to self and
		// parents' hide() / show() methods. The "visible" flag here is NOT
		// the same as Sprite::visible()! Sprite::visible() is only limited
		// to the sprite itself while visible flag here is described above.
		virtual void		onAppearanceChanged(bool visible){}

		// Always access the bounds via this, which will build them if necessary
		const ci::Rectf&	getClippingBounds();
		void				computeClippingBounds();

		void				setSpriteId(const ds::sprite_id_t&);
		// Helper utility to set a flag
		void				setFlag(const int newBit, const bool on, const DirtyState&, int& oldFlags);
		bool				getFlag(const int bit, const int flags) const;

		virtual void		markAsDirty(const DirtyState&);
		// Special function that marks all children as dirty, without sending anything up the hierarchy.
		virtual void		markChildrenAsDirty(const DirtyState&);
		virtual void		writeAttributesTo(ds::DataBuffer&);
		// Used during client mode, to let clients get info back to the server. Use the
		// engine_io.defs::ScopedClientAtts at the top of the function to do all the boilerplate.
		virtual void		writeClientAttributesTo(ds::DataBuffer&) const {};
		virtual void		readClientAttributeFrom(const char attributeId, ds::DataBuffer&){}
		// Read a single attribute
		virtual void		readAttributeFrom(const char attributeId, ds::DataBuffer&){}

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

		mutable ci::Matrix44f	mTransformation;
		mutable ci::Matrix44f	mInverseTransform;
		mutable bool			mUpdateTransform;

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

		mutable ci::Matrix44f	mGlobalTransform;
		mutable ci::Matrix44f	mInverseGlobalTransform;

		ds::UserData			mUserData;

		Sprite*					mParent;
		std::vector<Sprite *>	mChildren;
		// A cache for when I need to sort my children. This could be
		// a lot more efficient, only running the sort when Z changes.
		std::vector<Sprite*>	mSortedTmp;

		// Class-unique key for this type.  Subclasses can replace.
		char				mBlobType;
		DirtyState			mDirty;

		std::function<void(Sprite *, const TouchInfo &)> mProcessTouchInfoCallback;
		std::function<void(Sprite *, const ci::Vec3f &)> mSwipeCallback;
		std::function<bool(Sprite *, const TapInfo &)> mTapInfoCallback;
		std::function<void(Sprite *, const ci::Vec3f &)> mTapCallback;
		std::function<void(Sprite *, const ci::Vec3f &)> mDoubleTapCallback;
		std::function<void(Sprite *, const DragDestinationInfo &)> mDragDestinationCallback;

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

	template <typename T, typename... Args>
	static T& Sprite::make(SpriteEngine& e, Sprite* parent, Args... args)
	{
		T*                    s = new T(e, args...);
		if(!s) throw std::runtime_error("Can't create sprite");
		if(parent) parent->addChild(*s);
		return *s;
	}

	template <typename T>
	static T& Sprite::makeAlloc(const std::function<T*(void)>& allocFn, Sprite* parent)
	{
		T*                    s = allocFn();
		if(!s) throw std::runtime_error("Can't create sprite");
		if(parent) parent->addChild(*s);
		return *s;
	}

	template <typename T>
	static void Sprite::handleBlobFromServer(ds::BlobReader& r)
	{
		ds::DataBuffer&       buf(r.mDataBuffer);
		if(buf.read<char>() != SPRITE_ID_ATTRIBUTE) return;
		ds::sprite_id_t       id = buf.read<ds::sprite_id_t>();
		Sprite*               s = r.mSpriteEngine.findSprite(id);
		if(s) {
			s->readFrom(r);
		} else if((s = new T(r.mSpriteEngine)) != nullptr) {
			s->setSpriteId(id);
			s->readFrom(r);
			// If it didn't get assigned to a parent, something is wrong,
			// and it would disappear forever from memory management if I didn't
			// clean up here.
			if(!s->mParent) {
				assert(false);
				delete s;
			}
		}
	}

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_SPRITE_H_
