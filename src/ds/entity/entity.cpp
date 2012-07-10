#include "entity.h"
#include "cinder/gl/gl.h"
#include "gl/GL.h"
#include "../math/glm/gtc/matrix_transform.hpp"
#include "../math/glm/gtc/type_ptr.hpp"

using namespace ci;

namespace ds
{

Entity::Entity( float width /*= 0.0f*/, float height /*= 0.0f*/ )
    : mWidth(width)
    , mHeight(height)
    , mCenter(0.0f, 0.0f)
    , mRotation(0.0f)
    , mZLevel(0.0f)
    , mScale(1.0f, 1.0f)
    , mDrawSorted(false)
    , mUpdateTransform(true)
    , mParent(nullptr)
    , mOpacity(1.0f)
    , mColor(Color(1.0f, 1.0f, 1.0f))
    , mVisible(true)
    , mTransparent(true)
    , mEnabled(false)
{

}

Entity::~Entity()
{
    remove();
}

void Entity::update( const UpdateParams &updateParams )
{
    for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
    {
        (*it)->update(updateParams);
    }
}

void Entity::draw( const glm::mat4 &trans, const DrawParams &drawParams )
{
    if ( !mVisible )
        return;

    if ( mUpdateTransform )
        buildTransform();

    glm::mat4 totalTransformation = trans*mTranformation;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(totalTransformation));
    gl::color(mColor.r, mColor.g, mColor.b, mOpacity);

    if ( !mTransparent )
        drawLocal();

    glPopMatrix();

    if ( !mDrawSorted )
    {
        for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
        {
            (*it)->draw(totalTransformation, drawParams);
        }
    }
    else
    {
        std::list<Entity *> mCopy = mChildren;
        mCopy.sort([](Entity *i, Entity *j)
        {
            return i->getZLevel() < j->getZLevel();
        });

        for ( auto it = mCopy.begin(), it2 = mCopy.end(); it != it2; ++it )
        {
            (*it)->draw(totalTransformation, drawParams);
        }
    }
}

void Entity::setPosition( float x, float y )
{
    mPosition = glm::vec2(x, y);
}

const glm::vec2 &Entity::getPosition() const
{
    return mPosition;
}

void Entity::setScale( float x, float y )
{
    mScale = glm::vec2(x, y);
}

const glm::vec2 &Entity::getScale() const
{
    return mScale;
}

void Entity::setCenter( float x, float y )
{
    mCenter = glm::vec2(x, y);
}

const glm::vec2 &Entity::getCenter() const
{
    return mCenter;
}

void Entity::setRotation( float rotZ )
{
    if ( fabs(mRotation-rotZ) < 0.0001f )
        return;

    mRotation = rotZ;
    mUpdateTransform = true;
}

float Entity::getRotation() const
{
    return mRotation;
}

void Entity::setZLevel( float zlevel )
{
    mZLevel = zlevel;
}

float Entity::getZLevel() const
{
    return mZLevel;
}

void Entity::setDrawSorted( bool drawSorted )
{
    mDrawSorted = drawSorted;
}

bool Entity::getDrawSorted() const
{
    return mDrawSorted;
}

const glm::mat4 &Entity::getTransform() const
{
    if ( mUpdateTransform )
        buildTransform();
    return mTranformation;
}

void Entity::addChild( Entity *child )
{
    if ( containsChild(child) )
        return;

    mChildren.push_back(child);
    child->setParent(this);
}

void Entity::removeChild( Entity *child )
{
    if ( !containsChild(child) )
        return;

    mChildren.remove(child);
    child->setParent(nullptr);
}

void Entity::setParent( Entity *parent )
{
    removeParent();
    mParent = parent;
    if ( mParent)
        mParent->addChild(this);
}

void Entity::removeParent()
{
    if ( mParent )
    {
        mParent->removeChild(this);
        mParent = nullptr;
    }
}

bool Entity::containsChild( Entity *child ) const
{
    auto found = std::find(mChildren.begin(), mChildren.end(), child);

    if ( found != mChildren.end() )
        return true;
    return false;
}

void Entity::clearChildren()
{
    auto tempList = mChildren;
    mChildren.clear();

    for ( auto it = tempList.begin(), it2 = tempList.end(); it != it2; ++it )
    {
    	if ( !(*it) )
            continue;
        (*it)->removeParent();
        (*it)->clearChildren();
        delete *it;
    }
}

void Entity::buildTransform() const
{
    mUpdateTransform = false;

    mTranformation = glm::mat4(1.0f);
    //mTranformation = glm::scale(mTranformation, glm::vec3(mScale, 1.0f)) *
    //                 glm::rotate(mTranformation, mRotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
    //                 glm::translate(mTranformation, glm::vec3(mPosition, 0.0f)) *
    //                 glm::translate(mTranformation, glm::vec3(-mCenter.x*mWidth, -mCenter.y*mHeight, 0.0f));
    
    mTranformation = glm::translate(mTranformation, glm::vec3(mPosition, 0.0f)) *
                     glm::scale(mTranformation, glm::vec3(mScale, 1.0f)) *
                     glm::rotate(mTranformation, mRotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
                     glm::translate(mTranformation, glm::vec3(-mCenter.x*mWidth, -mCenter.y*mHeight, 0.0f));
}

void Entity::remove()
{
    clearChildren();
    removeParent();
}

void Entity::setSize( float width, float height )
{
    mWidth = width;
    mHeight = height;
}

void Entity::setColor( const Color &color )
{
    mColor = color;
}

void Entity::setColor( float r, float g, float b )
{
    mColor = Color(r, g, b);
}

Color Entity::getColor() const
{
    return mColor;
}

void Entity::setOpacity( float opacity )
{
    mOpacity = opacity;
}

float Entity::getOpacity() const
{
    return mOpacity;
}

void Entity::drawLocal()
{
    glBegin(GL_QUADS);
    gl::vertex( 0 , 0 );
    gl::vertex( mWidth, 0 );
    gl::vertex( mWidth, mHeight );
    gl::vertex( 0, mHeight );
    glEnd();
}

void Entity::setTransparent( bool transparent )
{
    mTransparent = transparent;
}

bool Entity::getTransparent() const
{
    return mTransparent;
}

void Entity::show()
{
    mVisible = true;
}

void Entity::hide()
{
    mVisible = false;
}

bool Entity::visible() const
{
    return mVisible;
}

int Entity::getType() const
{
    return mType;
}

void Entity::setType( int type )
{
    mType = type;
}

float Entity::getWidth() const
{
    return mWidth;
}

float Entity::getHeight() const
{
    return mHeight;
}

void Entity::enable( bool flag )
{
    mEnabled = flag;
}

bool Entity::isEnabled() const
{
    return mEnabled;
}

void Entity::buildGlobalTransform() const
{
    buildTransform();

    mGlobalTransform = mTranformation;

    for ( Entity *parent = mParent; parent; parent = parent->getParent() )
    {
        mGlobalTransform = parent->getGlobalTransform() * mGlobalTransform;
    }

    mInverseGlobalTransform = glm::inverse(mGlobalTransform);
}

Entity * Entity::getParent() const
{
    return mParent;
}

const glm::mat4x4 & Entity::getGlobalTransform() const
{
    buildGlobalTransform();

    return mGlobalTransform;
}

glm::vec2 Entity::globalToLocal( const glm::vec2 &globalPoint )
{
    buildGlobalTransform();

    glm::vec4 point = mInverseGlobalTransform * glm::vec4(globalPoint, 0.0f, 1.0f);
    return glm::vec2(point.x, point.y);
}

glm::vec2 Entity::localToGlobal( const glm::vec2 &localPoint )
{
    buildGlobalTransform();
    glm::vec4 point = mGlobalTransform * glm::vec4(localPoint, 0.0f, 1.0f);
    return glm::vec2(point.x, point.y);
}

bool Entity::contains( const glm::vec2 &point ) const
{
    buildGlobalTransform();

    glm::vec4 pR = glm::vec4(point, 0.0f, 1.0f);

    glm::vec4 cA = mGlobalTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 cB = mGlobalTransform * glm::vec4(mWidth, 0.0f, 0.0f, 1.0f);
    glm::vec4 cC = mGlobalTransform * glm::vec4(mWidth, mHeight, 0.0f, 1.0f);
    
    glm::vec4 v1 = cA - cB;
    glm::vec4 v2 = cC - cB;
    glm::vec4 v = pR - cB;

    float dot1 = glm::dot(v, v1);
    float dot2 = glm::dot(v, v2);
    float dot3 = glm::dot(v1, v1);
    float dot4 = glm::dot(v2, v2);

	return (
        dot1 >= 0 &&
        dot2 >= 0 &&
        dot1 <= dot3 &&
        dot2 <= dot4
	);
}

Entity *Entity::getHit( const glm::vec2 &point )
{
    if ( !mDrawSorted )
    {
        for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
        {
            Entity *child = *it;
            if ( child->isEnabled() && child->contains(point) )
                return child;
            Entity *hitChild = child->getHit(point);
            if ( hitChild )
                return hitChild;
        }
    }
    else
    {
        std::list<Entity *> mCopy = mChildren;
        mCopy.sort([](Entity *i, Entity *j)
        {
            return i->getZLevel() < j->getZLevel();
        });

        for ( auto it = mCopy.begin(), it2 = mCopy.end(); it != it2; ++it )
        {
            Entity *child = *it;
            if ( child->isEnabled() && child->contains(point) )
                return child;
            Entity *hitChild = child->getHit(point);
            if ( hitChild )
                return hitChild;
        }
    }

    if ( isEnabled() && contains(point) )
        return this;

    return nullptr;
}

}
