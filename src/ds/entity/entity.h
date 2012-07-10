#pragma once
#ifndef DS_OBJECT_INTERFACE_H
#define DS_OBJECT_INTERFACE_H
#include "cinder/Cinder.h"
#include <list>
#include "../math/glm/glm.hpp"
#include "cinder/Color.h"

namespace ds
{

class UpdateParams;
class DrawParams;
/*!
 * brief Base Class for App Entities
 *
 * basic scene container for app. objects implement a few functions to abstract functionality.
 * Entity will delete children when clearing.
 */
class Entity
{
    public:
        Entity(float width = 0.0f, float height = 0.0f);
        virtual ~Entity();

        void                update(const UpdateParams &updateParams);
        virtual void        draw( const glm::mat4 &trans, const DrawParams &drawParams );

        virtual void        setSize(float width, float height);
        float               getWidth() const;
        float               getHeight() const;

        void                setPosition(float x, float y);
        const glm::vec2    &getPosition() const;

        void                setScale(float x, float y);
        const glm::vec2    &getScale() const;

        void                setCenter(float x, float y);
        const glm::vec2    &getCenter() const;

        void                setRotation(float rotZ);
        float               getRotation() const;

        void                setZLevel( float zlevel );
        float               getZLevel() const;

        // whether to draw be by Entity order or z level.
        // Only works on a per Entity base.
        void                setDrawSorted( bool drawSorted );
        bool                getDrawSorted() const;

        const glm::mat4x4  &getTransform() const;
        const glm::mat4x4  &getGlobalTransform() const;

        void                addChild( Entity *child );

        // removes child from Entity, but does not delete it.
        void                removeChild( Entity *child );
        // calls removeParent then addChild to parent.
        void                setParent( Entity *parent );
        // remove child from parent, does not delete.
        void                removeParent();

        // check to see if Entity contains child
        bool                containsChild( Entity *child ) const;
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

        // removes Entity from parent and deletes all children. Does not delete Entity.
        void                remove();

        // check to see if Entity can be touched
        void                enable(bool flag);
        bool                isEnabled() const;

        Entity             *getParent() const;

        glm::vec2           globalToLocal( const glm::vec2 &globalPoint );
        glm::vec2           localToGlobal( const glm::vec2 &localPoint );

        // check if a point is inside the Entity's bounds.
        bool                contains( const glm::vec2 &point ) const;

        // finds Entity at position
        Entity             *getHit( const glm::vec2 &point );
    protected:
        void                buildTransform() const;
        void                buildGlobalTransform() const;
        virtual void        drawLocal();

        void                setType(int type);

        float               mWidth;
        float               mHeight;

        mutable glm::mat4   mTranformation;
        mutable bool        mUpdateTransform;

        glm::vec2           mPosition;
        glm::vec2           mCenter;
        glm::vec2           mScale;
        float               mRotation;
        float               mZLevel;
        bool                mDrawSorted;
        float               mOpacity;
        ci::Color           mColor;
        bool                mVisible;
        bool                mTransparent;
        int                 mType;
        bool                mEnabled;

        mutable glm::mat4   mGlobalTransform;
        mutable glm::mat4   mInverseGlobalTransform;

        Entity             *mParent;
        std::list<Entity *> mChildren; 
};

}

#endif//DS_OBJECT_INTERFACE_H
