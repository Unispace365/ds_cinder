#include "sprite.h"
#include "cinder/gl/gl.h"
#include "gl/GL.h"
#include "../../math/glm/gtc/matrix_transform.hpp"
#include "../../math/glm/gtc/type_ptr.hpp"

using namespace ci;

namespace ds {
namespace ui {

Sprite::Sprite( float width /*= 0.0f*/, float height /*= 0.0f*/ )
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

Sprite::~Sprite()
{
    remove();
}

void Sprite::update( const UpdateParams &updateParams )
{
    for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
    {
        (*it)->update(updateParams);
    }
}

void Sprite::draw( const glm::mat4 &trans, const DrawParams &drawParams )
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
        std::list<Sprite *> mCopy = mChildren;
        mCopy.sort([](Sprite *i, Sprite *j)
        {
            return i->getZLevel() < j->getZLevel();
        });

        for ( auto it = mCopy.begin(), it2 = mCopy.end(); it != it2; ++it )
        {
            (*it)->draw(totalTransformation, drawParams);
        }
    }
}

void Sprite::setPosition( float x, float y )
{
    mPosition = glm::vec2(x, y);
}

const glm::vec2 &Sprite::getPosition() const
{
    return mPosition;
}

void Sprite::setScale( float x, float y )
{
    mScale = glm::vec2(x, y);
}

const glm::vec2 &Sprite::getScale() const
{
    return mScale;
}

void Sprite::setCenter( float x, float y )
{
    mCenter = glm::vec2(x, y);
}

const glm::vec2 &Sprite::getCenter() const
{
    return mCenter;
}

void Sprite::setRotation( float rotZ )
{
    if ( fabs(mRotation-rotZ) < 0.0001f )
        return;

    mRotation = rotZ;
    mUpdateTransform = true;
}

float Sprite::getRotation() const
{
    return mRotation;
}

void Sprite::setZLevel( float zlevel )
{
    mZLevel = zlevel;
}

float Sprite::getZLevel() const
{
    return mZLevel;
}

void Sprite::setDrawSorted( bool drawSorted )
{
    mDrawSorted = drawSorted;
}

bool Sprite::getDrawSorted() const
{
    return mDrawSorted;
}

const glm::mat4 &Sprite::getTransform() const
{
    if ( mUpdateTransform )
        buildTransform();
    return mTranformation;
}

void Sprite::addChild( Sprite *child )
{
    if ( containsChild(child) )
        return;

    mChildren.push_back(child);
    child->setParent(this);
}

void Sprite::removeChild( Sprite *child )
{
    if ( !containsChild(child) )
        return;

    mChildren.remove(child);
    child->setParent(nullptr);
}

void Sprite::setParent( Sprite *parent )
{
    removeParent();
    mParent = parent;
    if ( mParent)
        mParent->addChild(this);
}

void Sprite::removeParent()
{
    if ( mParent )
    {
        mParent->removeChild(this);
        mParent = nullptr;
    }
}

bool Sprite::containsChild( Sprite *child ) const
{
    auto found = std::find(mChildren.begin(), mChildren.end(), child);

    if ( found != mChildren.end() )
        return true;
    return false;
}

void Sprite::clearChildren()
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

void Sprite::buildTransform() const
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

void Sprite::remove()
{
    clearChildren();
    removeParent();
}

void Sprite::setSize( float width, float height )
{
    mWidth = width;
    mHeight = height;
}

void Sprite::setColor( const Color &color )
{
    mColor = color;
}

void Sprite::setColor( float r, float g, float b )
{
    mColor = Color(r, g, b);
}

Color Sprite::getColor() const
{
    return mColor;
}

void Sprite::setOpacity( float opacity )
{
    mOpacity = opacity;
}

float Sprite::getOpacity() const
{
    return mOpacity;
}

void Sprite::drawLocal()
{
    glBegin(GL_QUADS);
    gl::vertex( 0 , 0 );
    gl::vertex( mWidth, 0 );
    gl::vertex( mWidth, mHeight );
    gl::vertex( 0, mHeight );
    glEnd();
}

void Sprite::setTransparent( bool transparent )
{
    mTransparent = transparent;
}

bool Sprite::getTransparent() const
{
    return mTransparent;
}

void Sprite::show()
{
    mVisible = true;
}

void Sprite::hide()
{
    mVisible = false;
}

bool Sprite::visible() const
{
    return mVisible;
}

int Sprite::getType() const
{
    return mType;
}

void Sprite::setType( int type )
{
    mType = type;
}

float Sprite::getWidth() const
{
    return mWidth;
}

float Sprite::getHeight() const
{
    return mHeight;
}

void Sprite::enable( bool flag )
{
    mEnabled = flag;
}

bool Sprite::isEnabled() const
{
    return mEnabled;
}

void Sprite::buildGlobalTransform() const
{
    buildTransform();

    mGlobalTransform = mTranformation;

    for ( Sprite *parent = mParent; parent; parent = parent->getParent() )
    {
        mGlobalTransform = parent->getGlobalTransform() * mGlobalTransform;
    }

    mInverseGlobalTransform = glm::inverse(mGlobalTransform);
}

Sprite * Sprite::getParent() const
{
    return mParent;
}

const glm::mat4x4 & Sprite::getGlobalTransform() const
{
    buildGlobalTransform();

    return mGlobalTransform;
}

glm::vec2 Sprite::globalToLocal( const glm::vec2 &globalPoint )
{
    buildGlobalTransform();

    glm::vec4 point = mInverseGlobalTransform * glm::vec4(globalPoint, 0.0f, 1.0f);
    return glm::vec2(point.x, point.y);
}

glm::vec2 Sprite::localToGlobal( const glm::vec2 &localPoint )
{
    buildGlobalTransform();
    glm::vec4 point = mGlobalTransform * glm::vec4(localPoint, 0.0f, 1.0f);
    return glm::vec2(point.x, point.y);
}

bool Sprite::contains( const glm::vec2 &point ) const
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

Sprite *Sprite::getHit( const glm::vec2 &point )
{
    if ( !mDrawSorted )
    {
        for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
        {
            Sprite *child = *it;
            if ( child->isEnabled() && child->contains(point) )
                return child;
            Sprite *hitChild = child->getHit(point);
            if ( hitChild )
                return hitChild;
        }
    }
    else
    {
        std::list<Sprite *> mCopy = mChildren;
        mCopy.sort([](Sprite *i, Sprite *j)
        {
            return i->getZLevel() < j->getZLevel();
        });

        for ( auto it = mCopy.begin(), it2 = mCopy.end(); it != it2; ++it )
        {
            Sprite *child = *it;
            if ( child->isEnabled() && child->contains(point) )
                return child;
            Sprite *hitChild = child->getHit(point);
            if ( hitChild )
                return hitChild;
        }
    }

    if ( isEnabled() && contains(point) )
        return this;

    return nullptr;
}

} // namespace ui
} // namespace ds
