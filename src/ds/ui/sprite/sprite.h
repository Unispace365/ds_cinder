#pragma once
#ifndef DS_OBJECT_INTERFACE_H
#define DS_OBJECT_INTERFACE_H
#include "cinder/Cinder.h"
#include <list>
#include "cinder/Color.h"
#include "cinder/Matrix22.h"
#include "cinder/Matrix33.h"
#include "cinder/MatrixAffine2.h"
#include "cinder/Matrix44.h"
#include "cinder/Vector.h"

using namespace ci;

namespace ds {

class UpdateParams;
class DrawParams;

namespace ui {

struct TouchInfo;
class SpriteEngine;

/*!
 * brief Base Class for App Entities
 *
 * basic scene container for app. objects implement a few functions to abstract functionality.
 * Sprite will delete children when clearing.
 */
class Sprite
{
    public:
        Sprite(SpriteEngine&, float width = 0.0f, float height = 0.0f);
        virtual ~Sprite();

        void                update(const UpdateParams &updateParams);
        virtual void        draw( const Matrix44f &trans, const DrawParams &drawParams );

        virtual void        setSize(float width, float height);
        float               getWidth() const;
        float               getHeight() const;

        void                setPosition(const Vec2f &pos);
        void                setPosition(float x, float y);
        const Vec2f        &getPosition() const;

        void                move(const Vec2f &delta);
        void                move(float deltaX, float deltaY);

        void                setScale(const Vec2f &scale);
        void                setScale(float x, float y);
        const Vec2f        &getScale() const;

        // center of the Sprite. Where its positioned at and rotated at.
        void                setCenter(const Vec2f &center);
        void                setCenter(float x, float y);
        const Vec2f        &getCenter() const;

        void                setRotation(float rotZ);
        float               getRotation() const;

        void                setZLevel( float zlevel );
        float               getZLevel() const;

        // whether to draw be by Sprite order or z level.
        // Only works on a per Sprite base.
        void                setDrawSorted( bool drawSorted );
        bool                getDrawSorted() const;

        const Matrix44f    &getTransform() const;
        const Matrix44f    &getGlobalTransform() const;

        void                addChild( Sprite *child );

        // removes child from Sprite, but does not delete it.
        void                removeChild( Sprite *child );
        // calls removeParent then addChild to parent.
        void                setParent( Sprite *parent );
        // remove child from parent, does not delete.
        void                removeParent();

        // check to see if Sprite contains child
        bool                containsChild( Sprite *child ) const;
        // removes and deletes all children
        void                clearChildren();

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

        Vec2f               globalToLocal( const Vec2f &globalPoint );
        Vec2f               localToGlobal( const Vec2f &localPoint );

        // check if a point is inside the Sprite's bounds.
        bool                contains( const Vec2f &point ) const;

        // finds Sprite at position
        Sprite             *getHit( const Vec2f &point );

        void                setProcessTouchCallback( const std::function<void (Sprite *, const TouchInfo &)> &func );
    protected:
        friend class        TouchManager;
        void                processTouchInfo( const TouchInfo &touchInfo );
        void                buildTransform() const;
        void                buildGlobalTransform() const;
        virtual void        drawLocal();

        void                setType(int type);

        SpriteEngine       &mEngine;

        float               mWidth;
        float               mHeight;

        mutable Matrix44f   mTransformation;
        mutable bool        mUpdateTransform;

        Vec2f               mPosition;
        Vec2f               mCenter;
        Vec2f               mScale;
        float               mRotation;
        float               mZLevel;
        bool                mDrawSorted;
        float               mOpacity;
        ci::Color           mColor;
        bool                mVisible;
        bool                mTransparent;
        int                 mType;
        bool                mEnabled;

        mutable Matrix44f   mGlobalTransform;
        mutable Matrix44f   mInverseGlobalTransform;

        Sprite             *mParent;
        std::list<Sprite *> mChildren; 

        std::function<void (Sprite *, const TouchInfo &)> mProcessTouchInfoCallback;
};

} // namespace ui
} // namespace ds

#endif//DS_OBJECT_INTERFACE_H
