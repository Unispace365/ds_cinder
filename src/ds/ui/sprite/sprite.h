#pragma once
#ifndef DS_OBJECT_INTERFACE_H
#define DS_OBJECT_INTERFACE_H
#include <exception>
#include "cinder/Cinder.h"
#include <list>
#include "cinder/Color.h"
#include "cinder/Matrix22.h"
#include "cinder/Matrix33.h"
#include "cinder/MatrixAffine2.h"
#include "cinder/Matrix44.h"
#include "cinder/Tween.h"
#include "cinder/Vector.h"
#include "ds/app/app_defs.h"
#include "ds/util/bit_mask.h"
#include "ds/ui/sprite/dirty_state.h"
#include "ds/ui/touch/touch_process.h"
#include "ds/ui/touch/multi_touch_constraints.h"
#include "ds/ui/tween/sprite_anim.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "shader/sprite_shader.h"
#include "util/blend.h"
#include "ds/util/idle_timer.h"

namespace ds {

class BlobReader;
class BlobRegistry;
class DataBuffer;
class DrawParams;
class Engine;
class UpdateParams;

namespace ui {

struct DragDestinationInfo;
class SpriteEngine;
struct TapInfo;
struct TouchInfo;

// Attribute access
extern const char     SPRITE_ID_ATTRIBUTE;

/*!
 * brief Base Class for App Entities
 *
 * basic scene container for app. objects implement a few functions to abstract functionality.
 * Sprite will delete children when clearing.
 */
class Sprite : public SpriteAnimatable
{
    public:
        // Generic sprite creation function
        template <typename T>
        static T&           make(SpriteEngine&, Sprite* parent = nullptr);

        Sprite(SpriteEngine&, float width = 0.0f, float height = 0.0f);
        virtual ~Sprite();

        // Sprite behaviour can vary whether this is running on the server
        // or client.
        virtual void        updateClient(const UpdateParams &updateParams);
        virtual void        updateServer(const UpdateParams &updateParams);

        virtual void        drawClient( const ci::Matrix44f &trans, const DrawParams &drawParams );
        virtual void        drawServer( const ci::Matrix44f &trans, const DrawParams &drawParams );

        ds::sprite_id_t     getId() const       { return mId; }
        ds::ui::SpriteEngine& getEngine()       { return mEngine; }

        void                setSize(float width, float height);
        virtual void        setSizeAll(float width, float height, float depth);
        float               getWidth() const;
        float               getHeight() const;
        float               getDepth() const;
        float               getScaleWidth() const;
        float               getScaleHeight() const;
        float               getScaleDepth() const;

        void                setPosition(const ci::Vec3f &pos);
        void                setPosition(float x, float y, float z = 0.0f);
        const ci::Vec3f    &getPosition() const;

        void                move(const ci::Vec3f &delta);
        void                move(float deltaX, float deltaY, float deltaZ = 0.0f);

        void                setScale(const ci::Vec3f &scale);
        void                setScale(float x, float y, float z = 1.0f);
        const ci::Vec3f    &getScale() const;

        // center of the Sprite. Where its positioned at and rotated at.
        void                setCenter(const ci::Vec3f &center);
        void                setCenter(float x, float y, float z = 0.0f);
        const ci::Vec3f    &getCenter() const;

        void                setRotation(float rotZ);
        void                setRotation(const ci::Vec3f &rot);
        ci::Vec3f           getRotation() const;

        void                setZLevel( float zlevel );
        float               getZLevel() const;

        // whether to draw be by Sprite order or z level.
        // Only works on a per Sprite base.
        void                setDrawSorted( bool drawSorted );
        bool                getDrawSorted() const;

        const ci::Matrix44f &getTransform() const;
        const ci::Matrix44f &getInverseTransform() const;
        const ci::Matrix44f &getGlobalTransform() const;
        const ci::Matrix44f &getInverseGlobalTransform() const;

        void                addChild( Sprite &child );

        // removes child from Sprite, but does not delete it.
        void                removeChild( Sprite &child );
        // calls removeParent then addChild to parent.
        void                setParent( Sprite *parent );
        // remove child from parent, does not delete.
        void                removeParent();

        // check to see if Sprite contains child
        bool                containsChild( Sprite *child ) const;
        // removes and deletes all children
        void                clearChildren();

        // sends sprite to front of parents child list.
        void                sendToFront();
        // sends sprite to back of parents child list.
        void                sendToBack();

        virtual void        setColor( const ci::Color &color );
        virtual void        setColor( float r, float g, float b );
        ci::Color           getColor() const;

        void                setOpacity( float opacity );
        float               getOpacity() const;

        //whether or not to show the entity; does not hide children.
        void                setTransparent(bool transparent);
        bool                getTransparent() const;

        // will show all children that are visible
        void                show();
        // will hide all children as well.
        void                hide();

        bool                visible() const;

        int                 getType() const;

        // removes Sprite from parent and deletes all children. Does not delete Sprite.
        void                remove();

        // check to see if Sprite can be touched
        void                enable(bool flag);
        bool                isEnabled() const;

        Sprite             *getParent() const;

        ci::Vec3f           globalToLocal( const ci::Vec3f &globalPoint );
        ci::Vec3f           localToGlobal( const ci::Vec3f &localPoint );

        // check if a point is inside the Sprite's bounds.
        bool                contains( const ci::Vec3f &point ) const;

        // finds Sprite at position
        Sprite             *getHit( const ci::Vec3f &point );

        void                setProcessTouchCallback( const std::function<void (Sprite *, const TouchInfo &)> &func );
        // Stateful tap is the new-style and should be preferred over others.  It lets you
        // know the state of the tap in addition to the count.  It also lets clients cancel
        // a tap if, for example, they only want to handle single taps.  Answer true as
        // long as you want to keep checking for a tap, false otherwise.
        void                setTapInfoCallback( const std::function<bool (Sprite *, const TapInfo &)> &func );
        void                setTapCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func );
        void                setDoubleTapCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func );
        void                setDragDestinationCallback( const std::function<void (Sprite *, const DragDestinationInfo &)> &func );

        // Constraints defined in multi_touch_constraints.h
        void                enableMultiTouch(const BitMask &);
        void                disableMultiTouch();
        bool                multiTouchEnabled() const;
        bool                hasMultiTouchConstraint( const BitMask &constraint = MULTITOUCH_NO_CONSTRAINTS ) const;
        bool                multiTouchConstraintNotZero() const;

        bool                inBounds() const;
        void                setCheckBounds(bool checkBounds);
        bool                getCheckBounds() const;
        virtual bool        isLoaded() const;
        void                setDragDestination(Sprite *dragDestination);
        void                setDragDestiantion(Sprite *dragDestination);
        Sprite             *getDragDestination() const;

        bool                isDirty() const;
        void                writeTo(ds::DataBuffer&);
        void                readFrom(ds::BlobReader&);

        void                setBlendMode(const BlendMode &blendMode);
        BlendMode           getBlendMode() const;

        void                setBaseShader(const std::string &location, const std::string &shadername, bool applyToChildren = false);
        SpriteShader       &getBaseShader();
        std::string         getBaseShaderName() const;

        void                setClipping(bool flag);
        bool                getClipping() const;


        virtual             void userInputReceived();
        void                setSecondBeforeIdle( const double );
        double              secondsToIdle() const;
        bool                isIdling() const;
        void                startIdling();
        void                resetIdleTimer();
        void                clear();
    protected:
        friend class        TouchManager;
        friend class        TouchProcess;
        void                swipe(const ci::Vec3f &swipeVector);
        bool                tapInfo(const TapInfo&);
        void                tap(const ci::Vec3f &tapPos);
        void                doubleTap(const ci::Vec3f &tapPos);
        void                dragDestination(Sprite *sprite, const DragDestinationInfo &dragInfo);
        void                processTouchInfo( const TouchInfo &touchInfo );
        void                processTouchInfoCallback( const TouchInfo &touchInfo );
        void                buildTransform() const;
        void                buildGlobalTransform() const;
        virtual void        drawLocalClient();
        virtual void        drawLocalServer();
        bool                hasDoubleTap() const;
        bool                hasTap() const;
        bool                hasTapInfo() const;
        void                setType(int type);
        void                updateCheckBounds() const;
        bool                checkBounds() const;

        virtual void        onSizeChanged();

        // Always access the bounds via this, which will build them if necessary
        const ci::Rectf&    getClippingBounds();
        void                computeClippingBounds();

        void                setSpriteId(const ds::sprite_id_t&);
        // Helper utility to set a flag
        void                setFlag(const int newBit, const bool on, const DirtyState&, int& oldFlags);
        bool                getFlag(const int bit, const int flags) const;

        virtual void		    markAsDirty(const DirtyState&);
		    // Special function that marks all children as dirty, without sending anything up the hierarchy.
    		virtual void		    markChildrenAsDirty(const DirtyState&);
        virtual void        writeAttributesTo(ds::DataBuffer&);
        // Read a single attribute
        virtual void        readAttributeFrom(const char attributeId, ds::DataBuffer&);

        void                setUseShaderTextuer(bool flag);
        bool                getUseShaderTextuer() const;

        void                sendSpriteToFront(Sprite &sprite);
        void                sendSpriteToBack(Sprite &sprite);

        mutable bool        mBoundsNeedChecking;
        mutable bool        mInBounds;


        SpriteEngine       &mEngine;
        // The ID must always be assigned through setSpriteId(), which has some
        // behaviour associated with the ID changing.
        ds::sprite_id_t     mId;

        float               mWidth;
        float               mHeight;
        float               mDepth;

        mutable ci::Matrix44f   mTransformation;
        mutable ci::Matrix44f   mInverseTransform;
        mutable bool        mUpdateTransform;

        int                 mSpriteFlags;
        ci::Vec3f           mPosition;
        ci::Vec3f           mCenter;
        ci::Vec3f           mScale;
        ci::Vec3f           mRotation;
        float               mZLevel;
        float               mOpacity;
        ci::Color           mColor;
        int                 mType;
        ci::Rectf           mClippingBounds;
        bool                mClippingBoundsDirty;
        SpriteShader        mSpriteShader;

        mutable ci::Matrix44f   mGlobalTransform;
        mutable ci::Matrix44f   mInverseGlobalTransform;

        Sprite             *mParent;
        std::vector<Sprite *> mChildren; 

        // Class-unique key for this type.  Subclasses can replace.
        char                mBlobType;
    		DirtyState			    mDirty;

        std::function<void (Sprite *, const TouchInfo &)> mProcessTouchInfoCallback;
        std::function<void (Sprite *, const ci::Vec3f &)> mSwipeCallback;
        std::function<bool (Sprite *, const TapInfo &)> mTapInfoCallback;
        std::function<void (Sprite *, const ci::Vec3f &)> mTapCallback;
        std::function<void (Sprite *, const ci::Vec3f &)> mDoubleTapCallback;
        std::function<void (Sprite *, const DragDestinationInfo &)> mDragDestinationCallback;

        bool                mMultiTouchEnabled;
        BitMask             mMultiTouchConstraints;

        // All touch processing happens in the process touch class
        TouchProcess				mTouchProcess;

        bool                mCheckBounds;

        Sprite             *mDragDestination;

        IdleTimer           mIdleTimer;

  private:
        friend class Engine;
        // Internal constructor just for the Engine, used to create the root sprite,
        // which always exists and is identical across all architectures.
        Sprite(SpriteEngine&, const ds::sprite_id_t);

        void                init(const ds::sprite_id_t);
        void                readAttributesFrom(ds::DataBuffer&);

        void                dimensionalStateChanged();
        // Applies to all children, too.
        void                markClippingDirty();

        ci::gl::Texture     mRenderTarget;

        BlendMode           mBlendMode;

        //set by sprite constructors. doesn't need to be passed through.
        bool                mUseShaderTexture; 

        ci::ColorA          mServerColor;
        // This to make onSizeChanged() more efficient -- it can get
        // triggered as a result of position changes, which shouldn't affect it.
        float               mLastWidth, mLastHeight;

    public:
        static void           installAsServer(ds::BlobRegistry&);
        static void           installAsClient(ds::BlobRegistry&);

        template <typename T>
        static void           handleBlobFromServer(ds::BlobReader&);
        static void           handleBlobFromClient(ds::BlobReader&);
};

template <typename T>
static T& Sprite:: make(SpriteEngine& e, Sprite* parent)
{
  T*                    s = new T(e);
  if (!s) throw std::runtime_error("Can't create sprite");
  if (parent) parent->addChild(*s);
  return *s;
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

#endif//DS_OBJECT_INTERFACE_H
