#include "sprite.h"
#include "cinder/gl/gl.h"
#include "gl/GL.h"
#include "ds/math/math_defs.h"
#include "sprite_engine.h"
#include "ds/math/math_func.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

using namespace ci;

namespace ds {
namespace ui {

Sprite::Sprite( SpriteEngine& engine, float width /*= 0.0f*/, float height /*= 0.0f*/ )
    : mEngine(engine)
    , mWidth(width)
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
    , mMultiTouchEnabled(false)
    , mTouchProcess(engine, *this)
    , mCheckBounds(false)
    , mBoundsNeedChecking(true)
    , mInBounds(true)
{

}

Sprite::~Sprite()
{
    remove();
}

void Sprite::update( const UpdateParams &updateParams )
{
  if (mCheckBounds) {
    updateCheckBounds();
  }

    for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
    {
        (*it)->update(updateParams);
    }
}

void Sprite::draw( const Matrix44f &trans, const DrawParams &drawParams )
{
    if ( !mVisible )
        return;

    buildTransform();

    Matrix44f totalTransformation = trans*mTransformation;

    glPushMatrix();
    //gl::multModelView(totalTransformation);
    gl::multModelView(totalTransformation);
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
  mPosition = Vec2f(x, y);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::setPosition( const Vec2f &pos )
{
  mPosition = pos;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

const Vec2f &Sprite::getPosition() const
{
    return mPosition;
}

void Sprite::setScale( float x, float y )
{
  mScale = Vec2f(x, y);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::setScale( const Vec2f &scale )
{
  mScale = scale;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

const Vec2f &Sprite::getScale() const
{
  return mScale;
}

void Sprite::setCenter( float x, float y )
{
  mCenter = Vec2f(x, y);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::setCenter( const Vec2f &center )
{
  mCenter = center;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

const Vec2f &Sprite::getCenter() const
{
    return mCenter;
}

void Sprite::setRotation( float rotZ )
{
    if ( fabs(mRotation-rotZ) < 0.0001f )
        return;

    mRotation = rotZ;
    mUpdateTransform = true;
    mBoundsNeedChecking = true;
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

const Matrix44f &Sprite::getTransform() const
{
    buildTransform();
    return mTransformation;
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
  if (!mUpdateTransform)
    return;

    mUpdateTransform = false;

    mTransformation = Matrix44f::identity();
    //mTransformation = glm::scale(mTransformation, glm::vec3(mScale, 1.0f)) *
    //                 glm::rotate(mTransformation, mRotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
    //                 glm::translate(mTransformation, glm::vec3(mPosition, 0.0f)) *
    //                 glm::translate(mTransformation, glm::vec3(-mCenter.x*mWidth, -mCenter.y*mHeight, 0.0f));
    
    //mTransformation = glm::translate(mTransformation, glm::vec3(mPosition, 0.0f)) *
    //                 glm::scale(mTransformation, glm::vec3(mScale, 1.0f)) *
    //                 glm::rotate(mTransformation, mRotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
    //                 glm::translate(mTransformation, glm::vec3(-mCenter.x*mWidth, -mCenter.y*mHeight, 0.0f));

    mTransformation.setToIdentity();
    mTransformation.translate(Vec3f(mPosition.x, mPosition.y, 0.0f));
    mTransformation.rotate(Vec3f(0.0f, 0.0f, 1.0f), mRotation * math::DEGREE2RADIAN);
    mTransformation.scale(Vec3f(mScale.x, mScale.y, 1.0f));
    mTransformation.translate(Vec3f(-mCenter.x*mWidth, -mCenter.y*mHeight, 0.0f));

    mInverseTransform = mTransformation.inverted();
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

    mGlobalTransform = mTransformation;

    for ( Sprite *parent = mParent; parent; parent = parent->getParent() )
    {
        mGlobalTransform = parent->getGlobalTransform() * mGlobalTransform;
    }

    mInverseGlobalTransform = mGlobalTransform.inverted();
}

Sprite *Sprite::getParent() const
{
    return mParent;
}

const Matrix44f &Sprite::getGlobalTransform() const
{
    buildGlobalTransform();

    return mGlobalTransform;
}

Vec2f Sprite::globalToLocal( const Vec2f &globalPoint )
{
    buildGlobalTransform();

    Vec4f point = mInverseGlobalTransform * Vec4f(globalPoint.x, globalPoint.y, 0.0f, 1.0f);
    return Vec2f(point.x, point.y);
}

Vec2f Sprite::localToGlobal( const Vec2f &localPoint )
{
    buildGlobalTransform();
    Vec4f point = mGlobalTransform * Vec4f(localPoint.x, localPoint.y, 0.0f, 1.0f);
    return Vec2f(point.x, point.y);
}

bool Sprite::contains( const Vec2f &point ) const
{
    buildGlobalTransform();

    Vec4f pR = Vec4f(point.x, point.y, 0.0f, 1.0f);

    Vec4f cA = mGlobalTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    Vec4f cB = mGlobalTransform * Vec4f(mWidth, 0.0f, 0.0f, 1.0f);
    Vec4f cC = mGlobalTransform * Vec4f(mWidth, mHeight, 0.0f, 1.0f);
    
    Vec4f v1 = cA - cB;
    Vec4f v2 = cC - cB;
    Vec4f v = pR - cB;

    float dot1 = v.dot(v1);
    float dot2 = v.dot(v2);
    float dot3 = v1.dot(v1);
    float dot4 = v2.dot(v2);

	return (
        dot1 >= 0 &&
        dot2 >= 0 &&
        dot1 <= dot3 &&
        dot2 <= dot4
	);
}

Sprite *Sprite::getHit( const Vec2f &point )
{
    if ( !mDrawSorted )
    {
        for ( auto it = mChildren.rbegin(), it2 = mChildren.rend(); it != it2; ++it )
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

void Sprite::setProcessTouchCallback( const std::function<void (Sprite *, const TouchInfo &)> &func )
{
  mProcessTouchInfoCallback = func;
}

void Sprite::processTouchInfo( const TouchInfo &touchInfo )
{
  mTouchProcess.processTouchInfo(touchInfo);
}

void Sprite::move( const Vec2f &delta )
{
  mPosition += delta;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::move( float deltaX, float deltaY )
{
  mPosition += Vec2f(deltaX, deltaY);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

bool Sprite::multiTouchEnabled() const
{
  return mMultiTouchEnabled;
}

const Matrix44f &Sprite::getInverseGlobalTransform() const
{
  return mInverseGlobalTransform;
}

const Matrix44f    &Sprite::getInverseTransform() const
{
  buildTransform();
  return mInverseTransform;
}

bool Sprite::hasMultiTouchConstraint( const BitMask &constraint ) const
{
  return mMultiTouchConstraints & constraint;
}

bool Sprite::multiTouchConstraintNotZero() const
{
  return mMultiTouchConstraints.getFirstIndex() >= 0;
}

void Sprite::swipe( const Vec2f &swipeVector )
{
  if (mSwipeCallback)
    mSwipeCallback(this, swipeVector);
}

bool Sprite::hasDoubleTap() const
{
  if (mDoubleTapCallback)
    return true;
  return false;
}

void Sprite::tap( const Vec2f &tapPos )
{
  if (mTapCallback)
    mTapCallback(this, tapPos);
}

void Sprite::doubleTap( const Vec2f &tapPos )
{
  if (mDoubleTapCallback)
    mDoubleTapCallback(this, tapPos);
}

bool Sprite::hasTap() const
{
  if (mTapCallback)
    return true;
  return false;
}

void Sprite::processTouchInfoCallback( const TouchInfo &touchInfo )
{
  if (mProcessTouchInfoCallback)
    mProcessTouchInfoCallback(this, touchInfo);
}

void Sprite::setTapCallback( const std::function<void (Sprite *, const Vec2f &)> &func )
{
  mTapCallback = func;
}

void Sprite::setDoubleTapCallback( const std::function<void (Sprite *, const Vec2f &)> &func )
{
  mDoubleTapCallback = func;
}

void Sprite::enableMultiTouch( const BitMask &constraints )
{
  mMultiTouchEnabled = true;
  mMultiTouchConstraints.clear();
  mMultiTouchConstraints |= constraints;
}

void Sprite::disableMultiTouch()
{
  mMultiTouchEnabled = false;
  mMultiTouchConstraints.clear();
}

bool Sprite::checkBounds() const
{
  if (!mCheckBounds)
    return true;

  mBoundsNeedChecking = false;
  mInBounds = false;

  Rectf screenRect = mEngine.getScreenRect();

  float screenMinX = screenRect.getX1();
  float screenMaxX = screenRect.getX2();
  float screenMinY = screenRect.getY1();
  float screenMaxY = screenRect.getY2();

  float spriteMinX = 0.0f;
  float spriteMinY = 0.0f;
  float spriteMaxX = mWidth-1.0f;
  float spriteMaxY = mHeight-1.0f;

  Vec2f positions[4];

  buildGlobalTransform();

  positions[0] = (mGlobalTransform * Vec4f(spriteMinX, spriteMinY, 0.0f, 1.0f)).xy();
  positions[1] = (mGlobalTransform * Vec4f(spriteMaxX, spriteMinY, 0.0f, 1.0f)).xy();
  positions[2] = (mGlobalTransform * Vec4f(spriteMinX, spriteMaxY, 0.0f, 1.0f)).xy();
  positions[3] = (mGlobalTransform * Vec4f(spriteMaxX, spriteMaxY, 0.0f, 1.0f)).xy();


  spriteMinX = spriteMaxX = positions[0].x;
  spriteMinY = spriteMaxY = positions[0].y;

  for ( int i = 1; i < 4; ++i ) {
    if ( positions[i].x < spriteMinX )
      spriteMinX = positions[i].x;
    if ( positions[i].y < spriteMinY )
      spriteMinY = positions[i].y;
    if ( positions[i].x > spriteMaxX )
      spriteMaxX = positions[i].x;
    if ( positions[i].y > spriteMaxY )
      spriteMaxY = positions[i].y;
  }

  if ( spriteMinX == spriteMaxX || spriteMinY == spriteMaxY ) {
    return false;
  }

  if (spriteMinX > screenMaxX)
    return false;
  if (spriteMaxX < screenMinX)
    return false;
  if (spriteMinY > screenMaxY)
    return false;
  if (spriteMaxY < screenMinY)
    return false;

  for ( int i = 0; i < 4; ++i ) {
    if ( positions[i].x >= screenMinX && positions[i].x <= screenMaxX && positions[i].y >= screenMinY && positions[i].y <= screenMaxY ) {
      mInBounds = true;
      return true;
    }
  }

  Vec2f screenpos[4];

  screenpos[0] = Vec2f(screenMinX, screenMinY);
  screenpos[1] = Vec2f(screenMaxX, screenMinY);
  screenpos[2] = Vec2f(screenMinX, screenMaxY);
  screenpos[3] = Vec2f(screenMaxX, screenMaxY);

  for ( int i = 0; i < 4; ++i ) {
    if ( screenpos[i].x >= spriteMinX && screenpos[i].x <= spriteMaxX && screenpos[i].y >= spriteMinY && screenpos[i].y <= spriteMaxY ) {
      mInBounds = true;
      return true;
    }
  }


  for ( int i = 0; i < 4; ++i ) {
    for ( int j = 0; j < 4; ++j ) {
      if ( math::intersect2D( screenpos[i%4], screenpos[(i+1)%4], positions[i%4], positions[(i+1)%4] ) ) {
        mInBounds = true;
        return true;
      }
    }
  }

  return true;
}

void Sprite::setCheckBounds( bool checkBounds )
{
  mCheckBounds = checkBounds;
  mInBounds = !mCheckBounds;
  mBoundsNeedChecking = checkBounds;
}

bool Sprite::getCheckBounds() const
{
  return mCheckBounds;
}

void Sprite::updateCheckBounds() const
{
  if (mBoundsNeedChecking)
    checkBounds();
}

bool Sprite::inBounds() const
{
  updateCheckBounds();
  return mInBounds;
}

bool Sprite::isLoaded() const
{
  return true;
}

} // namespace ui
} // namespace ds
