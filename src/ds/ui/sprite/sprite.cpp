#include "sprite.h"
#include "cinder/gl/gl.h"
#include "gl/GL.h"
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "sprite_engine.h"
#include "ds/math/math_func.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

using namespace ci;

namespace ds {
namespace ui {

const char          SPRITE_ID_ATTRIBUTE = 1;

namespace {
char                BLOB_TYPE         = 0;

const DirtyState    ID_DIRTY 			    = newUniqueDirtyState();
const DirtyState    CHILD_DIRTY 			= newUniqueDirtyState();
const DirtyState    POSITION_DIRTY 		= newUniqueDirtyState();

const char          POSITION_ATT      = 2;

const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("sprite");
}

void Sprite::install(ds::BlobRegistry& registry)
{
  BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlob<Sprite>(r);});
}

Sprite::Sprite( SpriteEngine& engine, float width /*= 0.0f*/, float height /*= 0.0f*/ )
    : mEngine(engine)
    , mId(ds::EMPTY_SPRITE_ID)
    , mWidth(width)
    , mHeight(height)
    , mCenter(0.0f, 0.0f, 0.0f)
    , mRotation(0.0f, 0.0f, 0.0f)
    , mZLevel(0.0f)
    , mScale(1.0f, 1.0f, 1.0f)
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
    , mDepth(1.0f)
    , mDragDestination(nullptr)
    , mBlobType(BLOB_TYPE)
    , mBlendMode(NORMAL)
{
  setSpriteId(mEngine.nextSpriteId());
}

Sprite::~Sprite()
{
    remove();
    setSpriteId(0);
}

void Sprite::updateClient( const UpdateParams &updateParams )
{
  if (mCheckBounds) {
    updateCheckBounds();
  }

    for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
    {
        (*it)->updateClient(updateParams);
    }
}

void Sprite::updateServer( const UpdateParams &updateParams )
{
  if (mCheckBounds) {
    updateCheckBounds();
  }

  for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
  {
    (*it)->updateServer(updateParams);
  }
}

void Sprite::drawClient( const Matrix44f &trans, const DrawParams &drawParams )
{
    if ( !mVisible )
        return;

    buildTransform();

    Matrix44f totalTransformation = trans*mTransformation;

    gl::pushModelView();
    gl::multModelView(totalTransformation);
    gl::color(mColor.r, mColor.g, mColor.b, mOpacity);

    if ( !mTransparent )
        drawLocalClient();

    gl::popModelView();

    if ( !mDrawSorted )
    {
        for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
        {
            (*it)->drawClient(totalTransformation, drawParams);
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
            (*it)->drawClient(totalTransformation, drawParams);
        }
    }
}

void Sprite::drawServer( const Matrix44f &trans, const DrawParams &drawParams )
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
    drawLocalServer();

  glPopMatrix();

  if ( !mDrawSorted )
  {
    for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it )
    {
      (*it)->drawServer(totalTransformation, drawParams);
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
      (*it)->drawServer(totalTransformation, drawParams);
    }
  }
}

void Sprite::setPosition( float x, float y, float z )
{
  setPosition(Vec3f(x, y, z));
}

void Sprite::setPosition( const Vec3f &pos )
{
  if (mPosition == pos) return;

  mPosition = pos;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
	markAsDirty(POSITION_DIRTY);
}

const Vec3f &Sprite::getPosition() const
{
    return mPosition;
}

void Sprite::setScale( float x, float y, float z )
{
  mScale = Vec3f(x, y, z);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::setScale( const Vec3f &scale )
{
  mScale = scale;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

const Vec3f &Sprite::getScale() const
{
  return mScale;
}

void Sprite::setCenter( float x, float y, float z )
{
  mCenter = Vec3f(x, y, z);
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::setCenter( const Vec3f &center )
{
  mCenter = center;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

const Vec3f &Sprite::getCenter() const
{
    return mCenter;
}

void Sprite::setRotation( float rotZ )
{
    if ( math::isEqual(mRotation.z, rotZ) )
        return;

    mRotation.z = rotZ;
    mUpdateTransform = true;
    mBoundsNeedChecking = true;
}

void Sprite::setRotation( const Vec3f &rot )
{
  if ( math::isEqual(mRotation.x, rot.x) && math::isEqual(mRotation.y, rot.y) && math::isEqual(mRotation.z, rot.z) )
    return;

  mRotation = rot;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

Vec3f Sprite::getRotation() const
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

void Sprite::addChild( Sprite &child )
{
    if ( containsChild(&child) )
        return;

    mChildren.push_back(&child);
    child.setParent(this);
}

void Sprite::removeChild( Sprite &child )
{
    if ( !containsChild(&child) )
        return;

    mChildren.remove(&child);
    child.setParent(nullptr);
}

void Sprite::setParent( Sprite *parent )
{
    removeParent();
    mParent = parent;
    if (mParent)
        mParent->addChild(*this);
}

void Sprite::removeParent()
{
    if ( mParent )
    {
        mParent->removeChild(*this);
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

    mTransformation.setToIdentity();
    mTransformation.translate(Vec3f(mPosition.x, mPosition.y, mPosition.z));
    mTransformation.rotate(Vec3f(1.0f, 0.0f, 0.0f), mRotation.x * math::DEGREE2RADIAN);
    mTransformation.rotate(Vec3f(0.0f, 1.0f, 0.0f), mRotation.y * math::DEGREE2RADIAN);
    mTransformation.rotate(Vec3f(0.0f, 0.0f, 1.0f), mRotation.z * math::DEGREE2RADIAN);
    mTransformation.scale(Vec3f(mScale.x, mScale.y, mScale.z));
    mTransformation.translate(Vec3f(-mCenter.x*mWidth, -mCenter.y*mHeight, -mCenter.z*mDepth));
    //mTransformation.setToIdentity();
    //mTransformation.translate(Vec3f(-mCenter.x*mWidth, -mCenter.y*mHeight, -mCenter.z*mDepth));
    //mTransformation.scale(Vec3f(mScale.x, mScale.y, mScale.z));
    //mTransformation.rotate(Vec3f(0.0f, 0.0f, 1.0f), mRotation.z * math::DEGREE2RADIAN);
    //mTransformation.rotate(Vec3f(0.0f, 1.0f, 0.0f), mRotation.y * math::DEGREE2RADIAN);
    //mTransformation.rotate(Vec3f(1.0f, 0.0f, 0.0f), mRotation.x * math::DEGREE2RADIAN);
    //mTransformation.translate(Vec3f(mPosition.x, mPosition.y, 1.0f));

    mInverseTransform = mTransformation.inverted();
}

void Sprite::remove()
{
    clearChildren();
    removeParent();
}

void Sprite::setSize( float width, float height, float depth )
{
    mWidth = width;
    mHeight = height;
    mDepth = depth;
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

void Sprite::drawLocalClient()
{
    //glBegin(GL_QUADS);
    //gl::vertex( 0 , 0 );
    //gl::vertex( mWidth, 0 );
    //gl::vertex( mWidth, mHeight );
    //gl::vertex( 0, mHeight );
    //glEnd();
  gl::drawSolidRect(Rectf(0.0f, 0.0f, mWidth, mHeight));
}

void Sprite::drawLocalServer()
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

Vec3f Sprite::globalToLocal( const Vec3f &globalPoint )
{
    buildGlobalTransform();

    Vec4f point = mInverseGlobalTransform * Vec4f(globalPoint.x, globalPoint.y, globalPoint.z, 1.0f);
    return Vec3f(point.x, point.y, point.z);
}

Vec3f Sprite::localToGlobal( const Vec3f &localPoint )
{
    buildGlobalTransform();
    Vec4f point = mGlobalTransform * Vec4f(localPoint.x, localPoint.y, localPoint.z, 1.0f);
    return Vec3f(point.x, point.y, point.z);
}

bool Sprite::contains( const Vec3f &point ) const
{
    buildGlobalTransform();

    Vec4f pR = Vec4f(point.x, point.y, point.z, 1.0f);

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

Sprite *Sprite::getHit( const Vec3f &point )
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

void Sprite::move( const Vec3f &delta )
{
  mPosition += delta;
  mUpdateTransform = true;
  mBoundsNeedChecking = true;
}

void Sprite::move( float deltaX, float deltaY, float deltaZ )
{
  mPosition += Vec3f(deltaX, deltaY, deltaZ);
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

void Sprite::swipe( const Vec3f &swipeVector )
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

void Sprite::tap( const Vec3f &tapPos )
{
  if (mTapCallback)
    mTapCallback(this, tapPos);
}

void Sprite::doubleTap( const Vec3f &tapPos )
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

void Sprite::setTapCallback( const std::function<void (Sprite *, const Vec3f &)> &func )
{
  mTapCallback = func;
}

void Sprite::setDoubleTapCallback( const std::function<void (Sprite *, const Vec3f &)> &func )
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

  Vec3f positions[4];

  buildGlobalTransform();

  positions[0] = (mGlobalTransform * Vec4f(spriteMinX, spriteMinY, 0.0f, 1.0f)).xyz();
  positions[1] = (mGlobalTransform * Vec4f(spriteMaxX, spriteMinY, 0.0f, 1.0f)).xyz();
  positions[2] = (mGlobalTransform * Vec4f(spriteMinX, spriteMaxY, 0.0f, 1.0f)).xyz();
  positions[3] = (mGlobalTransform * Vec4f(spriteMaxX, spriteMaxY, 0.0f, 1.0f)).xyz();


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

  Vec3f screenpos[4];

  screenpos[0] = Vec3f(screenMinX, screenMinY, 0.0f);
  screenpos[1] = Vec3f(screenMaxX, screenMinY, 0.0f);
  screenpos[2] = Vec3f(screenMinX, screenMaxY, 0.0f);
  screenpos[3] = Vec3f(screenMaxX, screenMaxY, 0.0f);

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
  
  mInBounds = true;
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

float Sprite::getDepth() const
{
  return mDepth;
}

void Sprite::setDragDestiantion( Sprite *dragDestination )
{
  mDragDestination = dragDestination;
}

Sprite *Sprite::getDragDestination() const
{
  return mDragDestination;
}

void Sprite::setDragDestinationCallback( const std::function<void (Sprite *, const DragDestinationInfo &)> &func )
{
  mDragDestinationCallback = func;
}

void Sprite::dragDestination( Sprite *sprite, const DragDestinationInfo &dragInfo )
{
  if (mDragDestinationCallback)
    mDragDestinationCallback(sprite, dragInfo);
}

bool Sprite::isDirty() const
{
  return !mDirty.isEmpty();
}

void Sprite::writeTo(ds::DataBuffer& buf)
{
  if (mDirty.isEmpty()) return;
  if (mId == ds::EMPTY_SPRITE_ID) {
    // This shouldn't be possible
    DS_LOG_WARNING_M("Sprite::writeTo() on empty sprite ID", SPRITE_LOG);
    return;
  }

  buf.add(mBlobType);
  buf.add(SPRITE_ID_ATTRIBUTE);
  buf.add(mId);

  writeAttributesTo(buf);
  mDirty.clear();

  for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
    (*it)->writeTo(buf);
  }
}

void Sprite::writeAttributesTo(ds::DataBuffer& buf)
{
		if (mDirty.has(POSITION_DIRTY)) {
      buf.add(POSITION_ATT);
      buf.add(mPosition.x);
      buf.add(mPosition.y);
      buf.add(mPosition.z);
    }
}

void Sprite::readFrom(ds::BlobReader& blob)
{
  ds::DataBuffer&       buf(blob.mDataBuffer);
  readAttributesFrom(buf);
}

void Sprite::readAttributesFrom(ds::DataBuffer& buf)
{
  char          id;
  while ((id=buf.read<char>()) != 0) {
    if (id == POSITION_ATT) {
      mPosition.x = buf.read<float>();
      mPosition.y = buf.read<float>();
      mPosition.z = buf.read<float>();
    } else {
      readAttributeFrom(id, buf);
    }
  }
}

void Sprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
}

void Sprite::setSpriteId(const ds::sprite_id_t& id)
{
  if (mId == id) return;

  if (mId != ds::EMPTY_SPRITE_ID) mEngine.unregisterSprite(*this);
  mId = id;
  if (mId != ds::EMPTY_SPRITE_ID) mEngine.registerSprite(*this);
  markAsDirty(ID_DIRTY);
}

void Sprite::markAsDirty(const DirtyState& dirty)
{
	mDirty |= dirty;
	Sprite*		      p = mParent;
	while (p) {
		if ((p->mDirty&CHILD_DIRTY) == true) break;

		p->mDirty |= CHILD_DIRTY;
		p = p->mParent;
	}

	// Opacity is a special case, since it's composed of myself and all parent values.
	// So make sure any children are notified that my opacity changed.
// This was true in BigWorld, probably still but not sure yet.
//	if (dirty.has(ds::OPACITY_DIRTY)) {
//		markChildrenAsDirty(ds::OPACITY_DIRTY);
//	}
}

void Sprite::markChildrenAsDirty(const DirtyState& dirty)
{
	mDirty |= dirty;
	for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
		(*it)->markChildrenAsDirty(dirty);
	}
}

void Sprite::setBlendMode( const BlendMode &blendMode )
{

}

Sprite::BlendMode Sprite::getBlendMode() const
{
  return mBlendMode;
}

void Sprite::setShader( const std::string &shaderName )
{

}

std::string Sprite::getShaderName() const
{
  return mShaderName;
}

} // namespace ui
} // namespace ds
