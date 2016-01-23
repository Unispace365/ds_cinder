#include "sprite.h"
#include <cinder/Camera.h>
#include <cinder/gl/gl.h>
#include "gl/GL.h"
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/app/camera_utils.h"
#include "ds/app/environment.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/math/math_func.h"
#include "ds/math/random.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/tween/tweenline.h"
#include "ds/util/string_util.h"
#include "util/clip_plane.h"
#include "ds/params/draw_params.h"

#include <Poco/Debugger.h>
#include "cinder/ImageIo.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

namespace ds {
namespace ui {

const char          SPRITE_ID_ATTRIBUTE = 1;

namespace {
char                BLOB_TYPE         = 0;

const DirtyState	ID_DIRTY			= newUniqueDirtyState();
const DirtyState	PARENT_DIRTY		= newUniqueDirtyState();
const DirtyState	CHILD_DIRTY			= newUniqueDirtyState();
const DirtyState	FLAGS_DIRTY 	   	= newUniqueDirtyState();
const DirtyState	SIZE_DIRTY 	    	= newUniqueDirtyState();
const DirtyState	POSITION_DIRTY		= newUniqueDirtyState();
const DirtyState	CHECKBOUNDS_DIRTY	= newUniqueDirtyState();
const DirtyState	CENTER_DIRTY		= newUniqueDirtyState();
const DirtyState	SCALE_DIRTY			= newUniqueDirtyState();
const DirtyState	COLOR_DIRTY			= newUniqueDirtyState();
const DirtyState	OPACITY_DIRTY		= newUniqueDirtyState();
const DirtyState	BLEND_MODE			= newUniqueDirtyState();
const DirtyState	CLIPPING_BOUNDS		= newUniqueDirtyState();
const DirtyState	SORTORDER_DIRTY		= newUniqueDirtyState();
const DirtyState	ROTATION_DIRTY		= newUniqueDirtyState();
const DirtyState	CORNER_DIRTY		= newUniqueDirtyState();

const char			PARENT_ATT			= 2;
const char			SIZE_ATT			= 3;
const char			FLAGS_ATT			= 4;
const char			POSITION_ATT		= 5;
const char			CENTER_ATT			= 6;
const char			SCALE_ATT			= 7;
const char			COLOR_ATT			= 8;
const char			OPACITY_ATT			= 9;
const char			BLEND_ATT			= 10;
const char			CLIP_BOUNDS_ATT		= 11;
const char			SORTORDER_ATT		= 12;
const char			ROTATION_ATT		= 13;
const char			CHECKBOUNDS_ATT		= 14;
const char			CORNERRADIUS_ATT	= 15;

// flags
const int           VISIBLE_F			= (1<<0);
const int           TRANSPARENT_F		= (1<<1);
const int           ENABLED_F			= (1<<2);
const int           DRAW_SORTED_F		= (1<<3);
const int           CLIP_F				= (1<<4);
const int           SHADER_CHILDREN_F	= (1<<5);
const int           NO_REPLICATION_F	= (1<<6);
const int           ROTATE_TOUCHES_F	= (1<<7);
const int			DRAW_DEBUG_F		= (1<<8);

const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("sprite");
}

void Sprite::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void Sprite::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Sprite>(r); });
}

void Sprite::handleBlobFromClient(ds::BlobReader& r) {
	ds::DataBuffer&       buf(r.mDataBuffer);
	if(buf.read<char>() != SPRITE_ID_ATTRIBUTE) return;
	ds::sprite_id_t       id = buf.read<ds::sprite_id_t>();
	Sprite*               s = r.mSpriteEngine.findSprite(id);
	if(s) s->readFrom(r);
}

Sprite::Sprite(SpriteEngine& engine, float width /*= 0.0f*/, float height /*= 0.0f*/)
	: SpriteAnimatable(*this, engine)
	, mEngine(engine)
	, mId(ds::EMPTY_SPRITE_ID)
	, mWidth(width)
	, mHeight(height)
	, mTouchProcess(engine, *this)
	, mSpriteShader(Environment::getAppFolder("data/shaders"), "base")
	, mIdleTimer(engine)
	, mLastWidth(width)
	, mLastHeight(height)
	, mPerspective(false)
	, mUseDepthBuffer(false)
{
	init(mEngine.nextSpriteId());
	setSize(width, height);
}

Sprite::Sprite(SpriteEngine& engine, const ds::sprite_id_t id, const bool perspective)
	: SpriteAnimatable(*this, engine)
	, mEngine(engine)
	, mId(ds::EMPTY_SPRITE_ID)
	, mTouchProcess(engine, *this)
	, mSpriteShader(Environment::getAppFolder("data/shaders"), "base")
	, mIdleTimer(engine)
	, mLastWidth(0)
	, mLastHeight(0)
	, mPerspective(perspective)
	, mUseDepthBuffer(false)
{
	init(id);
}

void Sprite::init(const ds::sprite_id_t id) {
	mSpriteFlags = VISIBLE_F | TRANSPARENT_F;
	mWidth = 0;
	mHeight = 0;
	mCenter = ci::Vec3f(0.0f, 0.0f, 0.0f);
	mRotation = ci::Vec3f(0.0f, 0.0f, 0.0f);
	mScale = ci::Vec3f(1.0f, 1.0f, 1.0f);
	mUpdateTransform = true;
	mParent = nullptr;
	mOpacity = 1.0f;
	mColor = ci::Color(1.0f, 1.0f, 1.0f);
	mMultiTouchEnabled = false;
	mCheckBounds = false;
	mBoundsNeedChecking = true;
	mInBounds = true;
	mDepth = 1.0f;
	mDragDestination = nullptr;
	mBlobType = BLOB_TYPE;
	mBlendMode = NORMAL;
	mUseShaderTexture = false;
	mIsInScreenCoordsHack = false;
	mTouchScaleSizeMode = false;
	mCornerRadius = 0.0f;
	mDrawOpacity = 1.0f;
	mDelayedCallCueRef = nullptr;
	mHasDrawLocalClientPost = false;

	mLayoutBPad = 0.0f;
	mLayoutTPad = 0.0f;
	mLayoutRPad = 0.0f;
	mLayoutLPad = 0.0f;
	mLayoutFudge = ci::Vec2f::zero();
	mLayoutSize = ci::Vec2f::zero();
	mLayoutHAlign = 0;
	mLayoutVAlign = 0;
	mLayoutUserType = 0;

	mExportWithXml = true;

	if(mEngine.getRotateTouchesDefault()){
		setRotateTouches(true);
	}

	setSpriteId(id);

	mServerColor = ci::ColorA(static_cast<float>(math::random()*0.5 + 0.5),
							  static_cast<float>(math::random()*0.5 + 0.5),
							  static_cast<float>(math::random()*0.5 + 0.5),
							  0.4f);
	mClippingBounds.set(0.0f, 0.0f, 0.0f, 0.0f);
	mClippingBoundsDirty = false;
	mFrameBuffer[0] = nullptr;
	mFrameBuffer[1] = nullptr;
	mOutputFbo = nullptr;
	mIsRenderFinalToTexture = false;
	mShaderPasses = 0;

	dimensionalStateChanged();
}

Sprite::~Sprite() {
	animStop();
	cancelDelayedCall();

	mEngine.removeFromDragDestinationList(this);

	delete mFrameBuffer[0];
	delete mFrameBuffer[1];
	delete mOutputFbo;

	// We only want to request a delete for the sprite at the head of a tree,
	const sprite_id_t	id = mId;
	setSpriteId(ds::EMPTY_SPRITE_ID);

	for(auto it = mChildren.begin(), end = mChildren.end(); it != end; ++it) {
		(*it)->mParent = nullptr;
		// Make sure the destructor doesn't ask the engine to delete
		(*it)->setSpriteId(ds::EMPTY_SPRITE_ID);
		delete (*it);
	}
	mChildren.clear();

	if(id != ds::EMPTY_SPRITE_ID) {
		if((mSpriteFlags&NO_REPLICATION_F) == 0) {
			mEngine.spriteDeleted(id);
		}
	}
}

void Sprite::updateClient(const UpdateParams &p) {
	mIdleTimer.update();

	if(mCheckBounds) {
		updateCheckBounds();
	}

	for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
		(*it)->updateClient(p);
	}
}



void Sprite::updateServer(const UpdateParams &p) {
	mTouchProcess.update(p);

	mIdleTimer.update();
	mShaderPasses = mSpriteShaders.size() == 0 ? 0 : mSpriteShaders.size() - 1;

	if(mCheckBounds) {
		updateCheckBounds();
	}

	for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
		(*it)->updateServer(p);
	}
}

ci::gl::Texture* Sprite::getShaderOutputTexture()
{
	if (mSpriteShaders.size()>0 && mFrameBuffer[mFboIndex])
		return &mFrameBuffer[mFboIndex]->getTexture();
	else
		return nullptr;
}

void Sprite::drawClient(const ci::Matrix44f &trans, const DrawParams &drawParams) {
	if ((mSpriteFlags&VISIBLE_F) == 0) {
		return;
	}
	mShaderPass = 0;
	mFboIndex = 0;
	mIsLastPass = mShaderPasses > 0 ? false : true;

	buildTransform();
	ci::Matrix44f totalTransformation = trans*mTransformation;
	ci::gl::pushModelView();
	glLoadIdentity();
	ci::gl::multModelView(totalTransformation);
	bool flipImage = false;

	ci::Area viewport = ci::gl::getViewport();

	while (mShaderPass <= mShaderPasses){
		if (mShaderPasses > 0) {
			//Change viewport for rendering texture to FBO
			ci::gl::setViewport(mFrameBuffer[mFboIndex]->getBounds());

			//Output available on Texture Unit 1
			if (mShaderPasses == mShaderPass){						//last pass
				// render to screen   - may be overridden later to render to texture
				mFrameBuffer[mFboIndex]->unbindFramebuffer();

				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				mFrameBuffer[!mFboIndex]->bindTexture(1);
				ci::gl::popModelView();
				ci::gl::popMatrices();
				ci::gl::setViewport(viewport);

				mFboIndex = !mFboIndex;
				mIsLastPass = true;

				//the 'flipped' flag is ignored by shaders, so we need to manually force the flip

				if (mShaderPasses % 2){
					flipImage = true;
				}
			}
			else if (mShaderPass > 0){ //middle passes
				mFrameBuffer[mFboIndex]->bindFramebuffer();
				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				mFrameBuffer[!mFboIndex]->bindTexture(1);
				mFboIndex = !mFboIndex;
			}
			else { //first pass
				ci::gl::pushModelView();
				ci::gl::pushMatrices();
				ci::gl::setMatricesWindow(mFrameBuffer[mFboIndex]->getSize());
				mFrameBuffer[mFboIndex]->bindFramebuffer();
				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				mFboIndex = !mFboIndex;
			}
		}

		//Need an extra flip if rendering final out to texture
		if (mIsRenderFinalToTexture && mOutputFbo && mIsLastPass) {
			flipImage = !flipImage;
		}

		if (mIsLastPass && mIsRenderFinalToTexture && mOutputFbo){
			ci::gl::pushModelView();
			ci::gl::pushMatrices();
			//Need to set the MVP matrices to match dimensions of sprite object
			ci::gl::setMatricesWindow(ci::Vec2i(static_cast<int>(getWidth()), static_cast<int>(getHeight())));
			mOutputFbo->bindFramebuffer();
		}

		//Need to manual flip image.  The flip() function of the ci::Texture element doesn't work with shaders.
		if (flipImage && mIsLastPass)
		{
			ci::gl::scale(1.0f, -1.0f, 1.0f);							// invert Y axis so increasing Y goes down.
			ci::gl::translate(0.0f, (float)-getHeight(), 0.0f);			// shift origin up to upper-left corner.
		}

		if (!mSpriteShaders.empty()) {
			mSpriteShader = *mSpriteShaders[mShaderPass];
			mSpriteShader.loadShaders();
			mUniform = getShaderUniforms(mSpriteShader.getName());
		}
		else if (!mSpriteShader.isValid()) {
			mSpriteShader.loadShaders();
		}

		if ((mSpriteFlags&TRANSPARENT_F) == 0) {
			ci::gl::enableAlphaBlending();
			applyBlendingMode(mBlendMode);
			ci::gl::GlslProg& shaderBase = mSpriteShader.getShader();
			if (shaderBase) {
				shaderBase.bind();
				shaderBase.uniform("tex0", 0);
				shaderBase.uniform("useTexture", mUseShaderTexture);
				shaderBase.uniform("preMultiply", premultiplyAlpha(mBlendMode));
				mUniform.applyTo(shaderBase);
			}

			//Only set opacity for last pass
			if (mIsLastPass) {
				mDrawOpacity = mOpacity*drawParams.mParentOpacity;
			}
			else {
				mDrawOpacity = 1.0f;
			}

			ci::gl::color(mColor.r, mColor.g, mColor.b, mDrawOpacity);
			if (mUseDepthBuffer) {
				ci::gl::enableDepthRead();
				ci::gl::enableDepthWrite();
			}
			else {
				ci::gl::disableDepthRead();
				ci::gl::disableDepthWrite();
			}

			drawLocalClient();

			if (shaderBase) {
				shaderBase.unbind();
				if (mSpriteShaders.size() > 0){
					mFrameBuffer[mFboIndex]->unbindTexture();

					ci::gl::scale(1.0f, 1.0f, 1.0f);           // invert Y axis so increasing Y goes down.
					ci::gl::translate(0.0f, 0.0f, 0.0f);       // shift origin up to upper-left corner.
				}
			}
		}

		mShaderPass++;
	}
	if (mIsRenderFinalToTexture && mOutputFbo){
		ci::gl::popModelView();
		ci::gl::popMatrices();
		ci::gl::setViewport(viewport);
		mOutputFbo->unbindFramebuffer();
	}

	if((mSpriteFlags&CLIP_F) != 0) {
		const ci::Rectf&      clippingBounds = getClippingBounds();
		enableClipping(clippingBounds.getX1(), clippingBounds.getY1(), clippingBounds.getX2(), clippingBounds.getY2());
	}

	ci::gl::popModelView();

	DrawParams dParams = drawParams;
	dParams.mParentOpacity *= mOpacity;

	if((mSpriteFlags&DRAW_SORTED_F) == 0) {
		for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
			(*it)->drawClient(totalTransformation, dParams);
		}
	} else {
		makeSortedChildren();
		for(auto it = mSortedTmp.begin(), it2 = mSortedTmp.end(); it != it2; ++it) {
			(*it)->drawClient(totalTransformation, dParams);
		}
	}

	/* does multi pass work with post*/

	if((mSpriteFlags&CLIP_F) != 0) {
		disableClipping();
	}

	if(mHasDrawLocalClientPost && ((mSpriteFlags&TRANSPARENT_F) == 0)) {
		ci::gl::pushModelView();
		glLoadIdentity();
		ci::gl::multModelView(totalTransformation);

		ci::gl::enableAlphaBlending();
		applyBlendingMode(mBlendMode);
		ci::gl::GlslProg& shaderBase = mSpriteShader.getShader();
		if(shaderBase) {
			shaderBase.bind();
			shaderBase.uniform("tex0", 0);
			shaderBase.uniform("useTexture", mUseShaderTexture);
			shaderBase.uniform("preMultiply", premultiplyAlpha(mBlendMode));
			mUniform.applyTo(shaderBase);
		}

		ci::gl::color(mColor.r, mColor.g, mColor.b, mDrawOpacity);
		if(mUseDepthBuffer) {
			ci::gl::enableDepthRead();
			ci::gl::enableDepthWrite();
		} else {
			ci::gl::disableDepthRead();
			ci::gl::disableDepthWrite();
		}

		drawLocalClientPost();

		if(shaderBase) {
			shaderBase.unbind();
		}
		ci::gl::popModelView();
	}
}

void Sprite::drawServer(const ci::Matrix44f &trans, const DrawParams &drawParams) {
	if((mSpriteFlags&VISIBLE_F) == 0) {
		return;
	}
	if(mId > 0) {
		glLoadName(mId);
	}

	buildTransform();
	ci::Matrix44f totalTransformation = trans*mTransformation;
	ci::gl::pushModelView();
	glLoadIdentity();
	ci::gl::multModelView(totalTransformation);

	bool debugDraw = getDrawDebug();
	if((mSpriteFlags&TRANSPARENT_F) == 0 && (isEnabled() || debugDraw)) {
		if(debugDraw){
			ci::gl::enableAlphaBlending();
			applyBlendingMode(mBlendMode);

			mDrawOpacity = mOpacity*drawParams.mParentOpacity;
			ci::gl::color(mColor.r, mColor.g, mColor.b, mDrawOpacity);
		} else {
			ci::gl::disableAlphaBlending();
			ci::gl::color(mServerColor);
		}
		if(mUseDepthBuffer) {
			ci::gl::enableDepthRead();
			ci::gl::enableDepthWrite();
		} else {
			ci::gl::disableDepthRead();
			ci::gl::disableDepthWrite();
		}
		drawLocalServer();
	}

	if((mSpriteFlags&CLIP_F) != 0) {
		const ci::Rectf&      clippingBounds = getClippingBounds();
		enableClipping(clippingBounds.getX1(), clippingBounds.getY1(), clippingBounds.getX2(), clippingBounds.getY2());
	}

	ci::gl::popModelView();

	if((mSpriteFlags&DRAW_SORTED_F) == 0) {
		for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
			(*it)->drawServer(totalTransformation, drawParams);
		}
	} else {
		makeSortedChildren();
		for(auto it = mSortedTmp.begin(), it2 = mSortedTmp.end(); it != it2; ++it) {
			(*it)->drawServer(totalTransformation, drawParams);
		}
	}

	if((mSpriteFlags&CLIP_F) != 0) {
		disableClipping();
	}
}

void Sprite::setPosition( float x, float y, float z ) {
	doSetPosition(ci::Vec3f(x, y, z));
}

void Sprite::setPosition(const ci::Vec2f& pos){
	doSetPosition(ci::Vec3f(pos.x, pos.y, mPosition.z));
}

void Sprite::setPosition(const ci::Vec3f &pos) {
	doSetPosition(pos);
}

bool Sprite::getInnerHit(const ci::Vec3f&) const {
	return true;
}

void Sprite::doSetPosition(const ci::Vec3f& pos) {
	if (mPosition == pos) return;

	mPosition = pos;
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	markAsDirty(POSITION_DIRTY);
	dimensionalStateChanged();
	onPositionChanged();
}

void Sprite::doSetScale(const ci::Vec3f& scale) {
	if(mScale == scale) return;

	mScale = scale;
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	markAsDirty(SCALE_DIRTY);
	dimensionalStateChanged();
	onScaleChanged();
}

const ci::Vec3f& Sprite::getPosition() const {
	return mPosition;
}

ci::Vec3f Sprite::getCenterPosition() const {
	return mPosition + getLocalCenterPosition();
}
 
ci::Vec3f Sprite::getLocalCenterPosition() const {
	return ci::Vec3f(floorf(mWidth/2.0f), floorf(mHeight/2.0f), mPosition.z);
}

void Sprite::setScale(float x, float y, float z){
	doSetScale(ci::Vec3f(x, y, z));
}

void Sprite::setScale(const ci::Vec3f& scale){
	doSetScale(scale);
}

void Sprite::setScale(float scale){
	setScale(scale, scale, scale);
}

const ci::Vec3f& Sprite::getScale() const {
	return mScale;
}

void Sprite::setCenter(float x, float y, float z){
	setCenter(ci::Vec3f(x, y, z));
}

void Sprite::setCenter(const ci::Vec3f& center){
	if(mCenter == center) return;

	mCenter = center;
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	markAsDirty(CENTER_DIRTY);
	dimensionalStateChanged();
	onCenterChanged();
}

const ci::Vec3f& Sprite::getCenter() const {
	return mCenter;
}

void Sprite::setRotation(float rotZ) {
	doSetRotation(ci::Vec3f(mRotation.x, mRotation.y, rotZ) );
}

void Sprite::setRotation(const float xRot, const float yRot, const float zRot) {
	doSetRotation(ci::Vec3f(xRot, yRot, zRot));
}

void Sprite::setRotation(const ci::Vec3f& rot) {
	doSetRotation(rot);
}

void Sprite::doSetRotation(const ci::Vec3f& rot) {
	if(math::isEqual(mRotation.x, rot.x) && math::isEqual(mRotation.y, rot.y) && math::isEqual(mRotation.z, rot.z))
		return;

	mRotation = rot;
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	markAsDirty(ROTATION_DIRTY);
	dimensionalStateChanged();
	onRotationChanged();
}

ci::Vec3f Sprite::getRotation() const
{
	return mRotation;
}

namespace {
	inline float	min(const float a, const float b) { return a <= b ? a : b; }
	inline float	max(const float a, const float b) { return a >= b ? a : b; }
}

ci::Rectf Sprite::getBoundingBox() const {
	const ci::Matrix44f&	t = getTransform();

	ci::Vec3f				ul = t * ci::Vec3f(0.0f, 0.0f, 0.0f);
	ci::Vec3f				ll = t * ci::Vec3f(0.0f, getHeight(), 0.0f);
	ci::Vec3f				lr = t * ci::Vec3f(getWidth(), getHeight(), 0.0f);
	ci::Vec3f				ur = t * ci::Vec3f(getWidth(), 0.0f, 0.0f);

	const float				left = min(min(min(ul.x, ll.x), lr.x), ur.x);
	const float				right = max(max(max(ul.x, ll.x), lr.x), ur.x);
	const float				top = min(min(min(ul.y, ll.y), lr.y), ur.y);
	const float				bottom = max(max(max(ul.y, ll.y), lr.y), ur.y);
	return ci::Rectf(left, top, right, bottom);
}

ci::Rectf Sprite::getChildBoundingBox() const {
	if(mChildren.empty()) return getBoundingBox();

	auto it = mChildren.begin();
	// initialize the box to the first child and expand from there
	ci::Rectf result = (*it)->getBoundingBox();
	for(auto end = mChildren.end(); it != end; ++it) {
		ci::Rectf curBounds = (*it)->getBoundingBox();
		result.include(curBounds);
	}
	return result;
}

void Sprite::setDrawSorted(bool drawSorted){
	setFlag(DRAW_SORTED_F, drawSorted, FLAGS_DIRTY, mSpriteFlags);
}

bool Sprite::getDrawSorted() const {
  return getFlag(DRAW_SORTED_F, mSpriteFlags);
}

const ci::Matrix44f &Sprite::getTransform() const {
	buildTransform();
	return mTransformation;
}

void Sprite::addChild(Sprite &child){
	if(this == &child) {
		throw std::runtime_error("Trying to add a Sprite to itself.");
	}

	if(containsChild(&child))
		return;

	if(getFlag(SHADER_CHILDREN_F, mSpriteFlags)) {
		child.setBaseShader(mSpriteShader.getLocation(), mSpriteShader.getName(), true);
	}

	mChildren.push_back(&child);
	child.setParent(this);
	child.setPerspective(mPerspective);
	child.setDrawSorted(getDrawSorted());
	child.setUseDepthBuffer(mUseDepthBuffer);

	onChildAdded(child);
}

void Sprite::removeChild(Sprite &child){
	if(!containsChild(&child))
		return;

	onChildRemoved(child);

	auto found = std::find(mChildren.begin(), mChildren.end(), &child);
	if(found != mChildren.end()) mChildren.erase(found);
	if(child.getParent() == this) {
		child.setParent(nullptr);
		child.setPerspective(false);
	}
}

void Sprite::setParent(Sprite *parent) {
	if(containsChild(parent)){
		removeChild(*parent);
	}
	removeParent();
	mParent = parent;
	if(mParent)
		mParent->addChild(*this);
	markAsDirty(PARENT_DIRTY);
}

void Sprite::removeParent() {
	if (mParent) {
		mParent->removeChild(*this);
		mParent = nullptr;
		markAsDirty(PARENT_DIRTY);
	}
}

void Sprite::remove() {
	clearChildren();
	removeParent();
}

void Sprite::release() {
	removeParent();
	delete this;
}

bool Sprite::containsChild(Sprite *child) const {
	auto found = std::find(mChildren.begin(), mChildren.end(), child);

	if(found != mChildren.end()) {
		return true;
	}
	return false;
}

void Sprite::clearChildren(){
	if(mChildren.empty()) return;
	auto tempList = mChildren;
	mChildren.clear();

	for(auto it = tempList.begin(), it2 = tempList.end(); it != it2; ++it){
		if(!(*it) || (*it)->getParent() != this)
			continue;
		(*it)->removeParent();
		(*it)->setParent(nullptr);
		delete *it;
	}
}

void Sprite::forEachChild(const std::function<void(Sprite&)>& fn, const bool recurse) {
	if(!fn) return;

	for(auto it = mChildren.begin(), end = mChildren.end(); it != end; ++it) {
		Sprite*		s(*it);
		if(s) {
			fn(*s);
			if(recurse) s->forEachChild(fn, recurse);
		}
	}
}

void Sprite::buildTransform() const{
	if(!mUpdateTransform)
		return;

	mUpdateTransform = false;

	mTransformation = ci::Matrix44f::identity();

	mTransformation.setToIdentity();
	mTransformation.translate(ci::Vec3f(mPosition.x, mPosition.y, mPosition.z));
	mTransformation.rotate(ci::Vec3f(1.0f, 0.0f, 0.0f), mRotation.x * math::DEGREE2RADIAN);
	mTransformation.rotate(ci::Vec3f(0.0f, 1.0f, 0.0f), mRotation.y * math::DEGREE2RADIAN);
	mTransformation.rotate(ci::Vec3f(0.0f, 0.0f, 1.0f), mRotation.z * math::DEGREE2RADIAN);
	mTransformation.scale(ci::Vec3f(mScale.x, mScale.y, mScale.z));
	mTransformation.translate(ci::Vec3f(-mCenter.x*mWidth, -mCenter.y*mHeight, -mCenter.z*mDepth));
	//mTransformation.setToIdentity();
	//mTransformation.translate(Vec3f(-mCenter.x*mWidth, -mCenter.y*mHeight, -mCenter.z*mDepth));
	//mTransformation.scale(Vec3f(mScale.x, mScale.y, mScale.z));
	//mTransformation.rotate(Vec3f(0.0f, 0.0f, 1.0f), mRotation.z * math::DEGREE2RADIAN);
	//mTransformation.rotate(Vec3f(0.0f, 1.0f, 0.0f), mRotation.y * math::DEGREE2RADIAN);
	//mTransformation.rotate(Vec3f(1.0f, 0.0f, 0.0f), mRotation.x * math::DEGREE2RADIAN);
	//mTransformation.translate(Vec3f(mPosition.x, mPosition.y, 1.0f));

	mInverseTransform = mTransformation.inverted();
}

const ci::Vec3f Sprite::getSize()const{
	return ci::Vec3f(mWidth, mHeight, mDepth);
}

void Sprite::setSizeAll(float width, float height, float depth){
	if(mWidth == width && mHeight == height && mDepth == depth) return;

	mWidth = width;
	mHeight = height;
	mDepth = depth;
	mUpdateTransform = true;
	markAsDirty(SIZE_DIRTY);
	dimensionalStateChanged();
}

void Sprite::setSizeAll(const ci::Vec3f& size3d){
	setSizeAll(size3d.x, size3d.y, size3d.z);
}

void Sprite::setSize(const ci::Vec2f& size2d) {
	setSize(size2d.x, size2d.y);
}

void Sprite::setSize(float width, float height) {
	setSizeAll(width, height, mDepth);
}

void Sprite::sizeToChildBounds(){
	ci::Rectf childBounds = getChildBoundingBox();

	move(childBounds.x1, childBounds.y1);
	setSize(childBounds.getWidth(), childBounds.getHeight());

	// move the children to compensate
	for (auto it = mChildren.begin(), end = mChildren.end(); it != end; ++it){
		(*it)->move(-childBounds.x1, -childBounds.y1);
	}	
}

ci::Vec3f Sprite::getPreferredSize() const {
	return ci::Vec3f(0.0f, 0.0f, 0.0f);
}

void Sprite::setColor(const ci::Color &color){
	if(mColor == color) return;

	mColor = color;
	markAsDirty(COLOR_DIRTY);
}

void Sprite::setColor(float r, float g, float b){
	setColor(ci::Color(r, g, b));
}

void Sprite::setColorA(const ci::ColorA& color){
	setColor(ci::Color(color.r, color.g, color.b));
	setOpacity(color.a);
}

ci::Color Sprite::getColor() const {
	return mColor;
}

ci::ColorA Sprite::getColorA() const {
	return ci::ColorA(mColor.r, mColor.g, mColor.b, mOpacity);
}

void Sprite::setOpacity(float opacity){
	if(mOpacity == opacity) return;

	mOpacity = opacity;
	markAsDirty(OPACITY_DIRTY);
}

float Sprite::getOpacity() const {
	return mOpacity;
}

float Sprite::getDrawOpacity() const
{
	return mDrawOpacity;
}

void Sprite::drawLocalClient(){
	if(mCornerRadius > 0.0f){
		ci::gl::drawSolidRoundedRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight), mCornerRadius);
	} else {
		// do this ourselves since Cinder is only willing to send vertices and texture coordinates
		glEnableClientState( GL_VERTEX_ARRAY );
		GLfloat verts[8];
		glVertexPointer( 2, GL_FLOAT, 0, verts );
		
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		GLfloat texCoords[8];
		glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );

		verts[0*2+0] = mWidth;
		verts[0*2+1] = 0.0f;
		verts[1*2+0] = 0.0f;
		verts[1*2+1] = 0.0f;
		verts[2*2+0] = mWidth;
		verts[2*2+1] = mHeight;
		verts[3*2+0] = 0.0f;
		verts[3*2+1] = mHeight;

		texCoords[0*2+0] = 1.0f;
		texCoords[0*2+1] = 0.0f;
		texCoords[1*2+0] = 0.0f;
		texCoords[1*2+1] = 0.0f;
		texCoords[2*2+0] = 1.0f;
		texCoords[2*2+1] = 1.0f;
		texCoords[3*2+0] = 0.0f;
		texCoords[3*2+1] = 1.0f;

		bool usingExtent = false;
		GLint extentLocation;
		GLfloat extent[8];
		ci::gl::GlslProg& shaderBase = mSpriteShader.getShader();
		if(shaderBase) {
			extentLocation = shaderBase.getAttribLocation("extent");
			if((extentLocation != GL_INVALID_OPERATION) && (extentLocation != -1)) {
				usingExtent = true;
				glEnableVertexAttribArray(extentLocation);
				glVertexAttribPointer( extentLocation, 2, GL_FLOAT, GL_FALSE, 0, extent );
				for(int i = 0; i < 4; i++) {
					extent[i*2+0] = mWidth;
					extent[i*2+1] = mHeight;
				}
			}
		}

		bool usingExtra = false;
		GLint extraLocation;
		GLfloat extra[16];
		if(shaderBase) {
			extraLocation = shaderBase.getAttribLocation("extra");
			if((extraLocation != GL_INVALID_OPERATION) && (extraLocation != -1)) {
				usingExtra = true;
				glEnableVertexAttribArray(extraLocation);
				glVertexAttribPointer( extraLocation, 4, GL_FLOAT, GL_FALSE, 0, extra );
				for(int i = 0; i < 4; i++) {
					extra[i*4+0] = mShaderExtraData.x;
					extra[i*4+1] = mShaderExtraData.y;
					extra[i*4+2] = mShaderExtraData.z;
					extra[i*4+3] = mShaderExtraData.w;
				}
			}
		}

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );

		if(usingExtent) {
			glDisableVertexAttribArray(extentLocation);
		}

		if(usingExtra) {
			glDisableVertexAttribArray(extraLocation);
		}
	}
}

void Sprite::drawLocalServer(){
	Sprite::drawLocalClient();
}

void Sprite::setTransparent(bool transparent){
	setFlag(TRANSPARENT_F, transparent, FLAGS_DIRTY, mSpriteFlags);
}

bool Sprite::getTransparent() const {
	return getFlag(TRANSPARENT_F, mSpriteFlags);
}

void Sprite::show(){
	const auto before = visible();
	setFlag(VISIBLE_F, true, FLAGS_DIRTY, mSpriteFlags);
	const auto now = visible();
	if(before != now)
	{
		onAppearanceChanged(now);
	}
	doPropagateVisibilityChange(before, now);
}

void Sprite::hide(){
	const auto before = visible();
	setFlag(VISIBLE_F, false, FLAGS_DIRTY, mSpriteFlags);
	const auto now = visible();
	if(before != now)
	{
		onAppearanceChanged(now);
	}
	doPropagateVisibilityChange(before, now);
}

bool Sprite::visible() const {
	return getFlag(VISIBLE_F, mSpriteFlags);
}

float Sprite::getWidth() const {
	return mWidth;
}

float Sprite::getHeight() const {
	return mHeight;
}

void Sprite::enable(bool flag) {
	mTouchProcess.clearTouches();
	setFlag(ENABLED_F, flag, FLAGS_DIRTY, mSpriteFlags);
}

bool Sprite::isEnabled() const {
	return getFlag(ENABLED_F, mSpriteFlags);
}

void Sprite::buildGlobalTransform() const {
	buildTransform();

	mGlobalTransform = mTransformation;

	for(Sprite *parent = mParent; parent; parent = parent->getParent())	{
		parent->buildTransform();
		mGlobalTransform = parent->mTransformation * mGlobalTransform;
	}

	mInverseGlobalTransform = mGlobalTransform.inverted();
}

void Sprite::parentEventReceived(const ds::Event &e) {
	Sprite*		p = mParent;
	while (p) {
		p->eventReceived(e);
		p = p->mParent;
	}
}

Sprite *Sprite::getParent() const {
	return mParent;
}

const ci::Matrix44f &Sprite::getGlobalTransform() const {
	buildGlobalTransform();

	return mGlobalTransform;
}

ci::Vec3f Sprite::globalToLocal(const ci::Vec3f &globalPoint){
	buildGlobalTransform();

	ci::Vec4f point = mInverseGlobalTransform * ci::Vec4f(globalPoint.x, globalPoint.y, globalPoint.z, 1.0f);
	return ci::Vec3f(point.x, point.y, point.z);
}

ci::Vec3f Sprite::localToGlobal(const ci::Vec3f &localPoint){
	buildGlobalTransform();
	ci::Vec4f point = mGlobalTransform * ci::Vec4f(localPoint.x, localPoint.y, localPoint.z, 1.0f);
	return ci::Vec3f(point.x, point.y, point.z);
}

bool Sprite::contains(const ci::Vec3f& point, const float pad) const {
	// If I don't check this, then sprites with no size are always picked.
	// Someone who knows the math can probably address the root issue.
	if(mWidth < 0.001f || mHeight < 0.001f) return false;
	// Same deal as above.
	if(mScale.x <= 0.0f || mScale.y <= 0.0f) return false;

	buildGlobalTransform();

	ci::Vec4f pR = ci::Vec4f(point.x, point.y, point.z, 1.0f);

	ci::Vec4f cA = mGlobalTransform * ci::Vec4f(-pad, -pad, 0.0f, 1.0f);
	ci::Vec4f cB = mGlobalTransform * ci::Vec4f(mWidth + pad, -pad, 0.0f, 1.0f);
	ci::Vec4f cC = mGlobalTransform * ci::Vec4f(mWidth + pad, mHeight + pad, 0.0f, 1.0f);

	ci::Vec4f v1 = cA - cB;
	ci::Vec4f v2 = cC - cB;
	ci::Vec4f v = pR - cB;

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

Sprite* Sprite::getHit(const ci::Vec3f &point) {
	// EH:  Not sure what bigworld was doing, but I don't see why we'd want to
	// select children of an invisible sprite.
	if(!visible()) {
		return nullptr;
	}
	// EH: Fix a bug where scales of 0,0,0 result in the sprite ALWAYS getting picked
	if(mScale.x <= 0.0f || mScale.y <= 0.0f || mScale.z <= 0.0f) {
		return nullptr;
	}
	if(getClipping()) {
		if(!contains(point))
			return nullptr;
	}

	if(!getFlag(DRAW_SORTED_F, mSpriteFlags))
	{
		for(auto it = mChildren.rbegin(), it2 = mChildren.rend(); it != it2; ++it)
		{
			Sprite *child = *it;
			Sprite *hitChild = child->getHit(point);
			if(hitChild)
				return hitChild;
			if(child->visible() && child->isEnabled() && child->contains(point) && child->getInnerHit(point))
				return child;
		}
	} else {
		makeSortedChildren();
		// picks happen in reverse sorted order (front to back)
		for(auto it = mSortedTmp.rbegin(), it2 = mSortedTmp.rend(); it != it2; ++it)
		{
			Sprite *child = *it;
			
			if(child->visible() && child->isEnabled() && child->contains(point) && child->getInnerHit(point))
				return child;
			Sprite *hitChild = child->getHit(point);
			if(hitChild)
				return hitChild;
		}
	}

	if(isEnabled() && contains(point) && getInnerHit(point))
		return this;

	return nullptr;
}

Sprite* Sprite::getPerspectiveHit(CameraPick& pick){
	if(!visible())
		return nullptr;

	makeSortedChildren();

	std::vector<ds::ui::Sprite*> candidates;

	for(auto it = mSortedTmp.rbegin(), it2 = mSortedTmp.rend(); it != it2; ++it) {
		Sprite*		hit = (*it)->getPerspectiveHit(pick);
		if(hit) {
			candidates.push_back(hit);
			//	std::cout << "Found pick candidate: " << hit->getParent()->localToGlobal(hit->getPosition()).z << " " << hit->getId() << std::endl;
			//	return hit;
		}
	}

	if(!candidates.empty()){
		if(candidates.size() == 1){
			return candidates.front();
		}


		float closestZ = -10000000.0f;
		if(candidates.front()->getParent()){
			closestZ = candidates.front()->getParent()->localToGlobal(candidates.front()->getPosition()).z;
		}
		ds::ui::Sprite* hit = candidates.front();
		for(auto it = candidates.begin() + 1; it < candidates.end(); ++it){
			if(!(*it)->getParent()) continue;
			float newZ = (*it)->getParent()->localToGlobal((*it)->getPosition()).z;
			if(newZ > closestZ){
				hit = (*it);
				closestZ = newZ;
			}
		}

		return hit;
	}

	if(isEnabled()) {
		const float	w = getScaleWidth(),
			h = getScaleHeight();

		if(w <= 0.0f || h <= 0.0f)
			return nullptr;

		ci::Vec3f ptR = pick.getScreenPt();
		ci::Vec3f a = getPosition();
		ci::Vec3f ptA = a;
		ci::Vec3f ptB = a;
		ci::Vec3f ptC = a;
		ci::Vec3f ptD = a;

		ci::Vec2f ptA_s;
		ci::Vec2f ptB_s;
		ci::Vec2f ptC_s;
		ci::Vec2f ptD_s;


		ptA.x -= mCenter.x*w;
		ptA.y += (1 - mCenter.y)*h;
		ptA_s = pick.worldToScreen(getParent()->localToGlobal(ci::Vec3f(ptA)));

		ptB.x += (1 - mCenter.x)*w;
		ptB.y += (1 - mCenter.y)*h;
		ptB_s = pick.worldToScreen(getParent()->localToGlobal(ci::Vec3f(ptB)));

		ptC.x += (1 - mCenter.x)*w;
		ptC.y -= mCenter.y*h;
		ptC_s = pick.worldToScreen(getParent()->localToGlobal(ci::Vec3f(ptC)));

		ci::Vec2f v1 = ptA_s - ptB_s;
		ci::Vec2f v2 = ptC_s - ptB_s;
		ci::Vec2f v;

		v.x = ptR.x - ptB_s.x;
		v.y = ptR.y - ptB_s.y;

		float dot1 = v.dot(v1);
		float dot2 = v.dot(v2);
		if(dot1 >= 0 &&
		   dot2 >= 0 &&
		   dot1 <= v1.dot(v1) &&
		   dot2 <= v2.dot(v2)
		   ) return this;
	}

	return nullptr;
}

void Sprite::setProcessTouchCallback(const std::function<void(Sprite *, const TouchInfo &)> &func){
	mProcessTouchInfoCallback = func;
}

void Sprite::processTouchInfo(const TouchInfo &touchInfo) {
	mTouchProcess.processTouchInfo(touchInfo);
}

void Sprite::move(const ci::Vec3f &delta) {
	mPosition += delta;
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	// XXX This REALLY should be going through doSetPosition().
	// Don't know what the original thought was, but now I'm
	// nrevous to hook that up.
	markAsDirty(POSITION_DIRTY);
	dimensionalStateChanged();
}

void Sprite::move( float deltaX, float deltaY, float deltaZ ) {
	mPosition += ci::Vec3f(deltaX, deltaY, deltaZ);
	mUpdateTransform = true;
	mBoundsNeedChecking = true;
	// XXX This REALLY should be going through doSetPosition().
	// Don't know what the original thought was, but now I'm
	// nrevous to hook that up.
	markAsDirty(POSITION_DIRTY);
	dimensionalStateChanged();
}

bool Sprite::multiTouchEnabled() const {
	return mMultiTouchEnabled;
}

const ci::Matrix44f& Sprite::getInverseGlobalTransform() const {
	return mInverseGlobalTransform;
}

const ci::Matrix44f& Sprite::getInverseTransform() const {
	buildTransform();
	return mInverseTransform;
}

ds::UserData& Sprite::getUserData() {
	return mUserData;
}

const ds::UserData& Sprite::getUserData() const {
	return mUserData;
}

bool Sprite::hasMultiTouchConstraint( const BitMask &constraint ) const {
	return mMultiTouchConstraints & constraint;
}

void Sprite::swipe(const ci::Vec3f &swipeVector){
	if(mSwipeCallback)
		mSwipeCallback(this, swipeVector);
}

bool Sprite::hasDoubleTap() const{
	if(mDoubleTapCallback){
		return true;
	}
	return false;
}

bool Sprite::tapInfo(const TapInfo& ti){
	if(mTapInfoCallback){
		return mTapInfoCallback(this, ti);
	}
	return false;
}

void Sprite::tap(const ci::Vec3f &tapPos){
	if(mTapCallback){
		mTapCallback(this, tapPos);
	}
}

void Sprite::doubleTap(const ci::Vec3f &tapPos){
	if(mDoubleTapCallback)
		mDoubleTapCallback(this, tapPos);
}

bool Sprite::hasTap() const {
	if(mTapCallback){
		return true;
	}
	return false;
}

bool Sprite::hasTapInfo() const {
	return mTapInfoCallback != nullptr;
}

void Sprite::processTouchInfoCallback(const TouchInfo &touchInfo){
	if(mProcessTouchInfoCallback)
		mProcessTouchInfoCallback(this, touchInfo);
}

void Sprite::setTapInfoCallback(const std::function<bool(Sprite *, const TapInfo &)> &func){
	mTapInfoCallback = func;
}

void Sprite::setTapCallback(const std::function<void(Sprite *, const ci::Vec3f &)> &func){
	mTapCallback = func;
}

void Sprite::setDoubleTapCallback(const std::function<void(Sprite *, const ci::Vec3f &)> &func){
	mDoubleTapCallback = func;
}

void Sprite::enableMultiTouch(const BitMask &constraints){
	mMultiTouchEnabled = true;
	mMultiTouchConstraints.clear();
	mMultiTouchConstraints |= constraints;
}

void Sprite::disableMultiTouch() {
	mMultiTouchEnabled = false;
	mMultiTouchConstraints.clear();
}

void Sprite::callAfterDelay(const std::function<void(void)>& fn, const float delay_in_seconds) {
	if (!fn) return;
	cancelDelayedCall();
	ci::Timeline&		t = mEngine.getTweenline().getTimeline();
	mDelayedCallCueRef = t.add(fn, t.getCurrentTime() + delay_in_seconds);
}

void Sprite::cancelDelayedCall(){
	if(mDelayedCallCueRef){
		mDelayedCallCueRef->removeSelf();
		mDelayedCallCueRef = nullptr;
	}
}

bool Sprite::checkBounds() const {
	if(!mCheckBounds)
		return true;

	mBoundsNeedChecking = false;
	mInBounds = false;

	const ci::Rectf&		screenRect(mEngine.getSrcRect());
	//  ci::Rectf screenRect = mEngine.getScreenRect();

	float screenMinX = screenRect.getX1();
	float screenMaxX = screenRect.getX2();
	float screenMinY = screenRect.getY1();
	float screenMaxY = screenRect.getY2();

	float spriteMinX = 0.0f;
	float spriteMinY = 0.0f;
	float spriteMaxX = mWidth - 1.0f;
	float spriteMaxY = mHeight - 1.0f;

	ci::Vec3f positions[4];

	buildGlobalTransform();

	positions[0] = (mGlobalTransform * ci::Vec4f(spriteMinX, spriteMinY, 0.0f, 1.0f)).xyz();
	positions[1] = (mGlobalTransform * ci::Vec4f(spriteMaxX, spriteMinY, 0.0f, 1.0f)).xyz();
	positions[2] = (mGlobalTransform * ci::Vec4f(spriteMinX, spriteMaxY, 0.0f, 1.0f)).xyz();
	positions[3] = (mGlobalTransform * ci::Vec4f(spriteMaxX, spriteMaxY, 0.0f, 1.0f)).xyz();


	spriteMinX = spriteMaxX = positions[0].x;
	spriteMinY = spriteMaxY = positions[0].y;

	for(int i = 1; i < 4; ++i) {
		if(positions[i].x < spriteMinX)
			spriteMinX = positions[i].x;
		if(positions[i].y < spriteMinY)
			spriteMinY = positions[i].y;
		if(positions[i].x > spriteMaxX)
			spriteMaxX = positions[i].x;
		if(positions[i].y > spriteMaxY)
			spriteMaxY = positions[i].y;
	}

	if(spriteMinX == spriteMaxX || spriteMinY == spriteMaxY) {
		return false;
	}

	if(spriteMinX > screenMaxX)
		return false;
	if(spriteMaxX < screenMinX)
		return false;
	if(spriteMinY > screenMaxY)
		return false;
	if(spriteMaxY < screenMinY)
		return false;

	for(int i = 0; i < 4; ++i) {
		if(positions[i].x >= screenMinX && positions[i].x <= screenMaxX && positions[i].y >= screenMinY && positions[i].y <= screenMaxY) {
			mInBounds = true;
			return true;
		}
	}

	ci::Vec3f screenpos[4];

	screenpos[0] = ci::Vec3f(screenMinX, screenMinY, 0.0f);
	screenpos[1] = ci::Vec3f(screenMaxX, screenMinY, 0.0f);
	screenpos[2] = ci::Vec3f(screenMinX, screenMaxY, 0.0f);
	screenpos[3] = ci::Vec3f(screenMaxX, screenMaxY, 0.0f);

	for(int i = 0; i < 4; ++i) {
		if(screenpos[i].x >= spriteMinX && screenpos[i].x <= spriteMaxX && screenpos[i].y >= spriteMinY && screenpos[i].y <= spriteMaxY) {
			mInBounds = true;
			return true;
		}
	}


	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 4; ++j) {
			if(math::intersect2D(screenpos[i % 4], screenpos[(i + 1) % 4], positions[i % 4], positions[(i + 1) % 4])) {
				mInBounds = true;
				return true;
			}
		}
	}

	mInBounds = true;
	return true;
}

void Sprite::setCheckBounds(bool checkBounds) {
	mCheckBounds = checkBounds;
	mInBounds = !mCheckBounds;
	mBoundsNeedChecking = checkBounds;
	markAsDirty(CHECKBOUNDS_DIRTY);
}

bool Sprite::getCheckBounds() const {
	return mCheckBounds;
}

void Sprite::updateCheckBounds() const
{
	if(mBoundsNeedChecking)
		checkBounds();
}

bool Sprite::inBounds() const {
	updateCheckBounds();
	return mInBounds;
}

bool Sprite::isLoaded() const {
	return true;
}

float Sprite::getDepth() const {
	return mDepth;
}

void Sprite::setDragDestination(Sprite *dragDestination){
	mDragDestination = dragDestination;
}

Sprite *Sprite::getDragDestination() const{
	return mDragDestination;
}

void Sprite::setDragDestinationCallback(const std::function<void(Sprite *, const DragDestinationInfo &)> &func){
	mDragDestinationCallback = func;
}

void Sprite::dragDestination(Sprite *sprite, const DragDestinationInfo &dragInfo) {
	if(mDragDestinationCallback)
		mDragDestinationCallback(sprite, dragInfo);
}

bool Sprite::isDirty() const {
	return !mDirty.isEmpty();
}

void Sprite::writeTo(ds::DataBuffer& buf) {
	if ((mSpriteFlags&NO_REPLICATION_F) != 0) return;
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
	// Terminate the sprite and attribute list
	buf.add(ds::TERMINATOR_CHAR);
	// If I wrote any attributes then make sure to terminate the block
	mDirty.clear();

	for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
		(*it)->writeTo(buf);
	}
}

void Sprite::writeClientTo(ds::DataBuffer &buf) const {
	writeClientAttributesTo(buf);
	for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
		(*it)->writeClientTo(buf);
	}
}

void Sprite::writeAttributesTo(ds::DataBuffer &buf) {
	if(mDirty.has(PARENT_DIRTY)) {
		if(mParent){
			buf.add(PARENT_ATT);
			buf.add(mParent->getId());
		} 

		// Why would you send an empty sprite id?
		//buf.add(ds::EMPTY_SPRITE_ID);
	}
	if (mDirty.has(SIZE_DIRTY)) {
		buf.add(SIZE_ATT);
		buf.add(mWidth);
		buf.add(mHeight);
		buf.add(mDepth);
	}
	if (mDirty.has(FLAGS_DIRTY)) {
		buf.add(FLAGS_ATT);
		buf.add(mSpriteFlags);
		// This is being sent here because I do not want to introduce a
		// new dirty state and the previous code already sets flag to false.
		buf.add(mSpriteShader.getLocation());
		buf.add(mSpriteShader.getName());
	}
	if (mDirty.has(POSITION_DIRTY)) {
		buf.add(POSITION_ATT);
		buf.add(mPosition.x);
		buf.add(mPosition.y);
		buf.add(mPosition.z);
	}
	if (mDirty.has(CHECKBOUNDS_DIRTY)) {
		buf.add(CHECKBOUNDS_ATT);
		buf.add(mCheckBounds);
	}
	if (mDirty.has(CENTER_DIRTY)) {
		buf.add(CENTER_ATT);
		buf.add(mCenter.x);
		buf.add(mCenter.y);
		buf.add(mCenter.z);
	}
	if (mDirty.has(ROTATION_DIRTY)) {
		buf.add(ROTATION_ATT);
		buf.add(mRotation.x);
		buf.add(mRotation.y);
		buf.add(mRotation.z);
	}
	if (mDirty.has(SCALE_DIRTY)) {
		buf.add(SCALE_ATT);
		buf.add(mScale.x);
		buf.add(mScale.y);
		buf.add(mScale.z);
	}
	if (mDirty.has(COLOR_DIRTY)) {
		buf.add(COLOR_ATT);
		buf.add(mColor.r);
		buf.add(mColor.g);
		buf.add(mColor.b);
	}
	if (mDirty.has(OPACITY_DIRTY)) {
		buf.add(OPACITY_ATT);
		buf.add(mOpacity);
	}
	if (mDirty.has(BLEND_MODE)) {
		buf.add(BLEND_ATT);
		buf.add(mBlendMode);
	}
	if (mDirty.has(CLIPPING_BOUNDS)) {
		buf.add(CLIP_BOUNDS_ATT);
		buf.add(mClippingBounds.getX1());
		buf.add(mClippingBounds.getY1());
		buf.add(mClippingBounds.getX2());
		buf.add(mClippingBounds.getY2());
	}
	if (mDirty.has(CORNER_DIRTY)){
		buf.add(CORNERRADIUS_ATT);
		buf.add(mCornerRadius);
	}
	if (mDirty.has(SORTORDER_DIRTY)) {
		// A flat list of ints, the first value is the number of ints
		buf.add(SORTORDER_ATT);
		buf.add<int32_t>(mChildren.size());
		for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
			buf.add<sprite_id_t>((*it) ? (*it)->getId() : 0);
		}
	}
}

void Sprite::readFrom(ds::BlobReader& blob) {
	ds::DataBuffer&       buf(blob.mDataBuffer);
	readAttributesFrom(buf);
}

void Sprite::readAttributesFrom(ds::DataBuffer& buf) {
	char          id;
	bool          transformChanged = false;
	while (buf.canRead<char>() && (id=buf.read<char>()) != ds::TERMINATOR_CHAR) {
		if (id == PARENT_ATT) {
			const sprite_id_t     parentId = buf.read<sprite_id_t>();
			Sprite*               parent = mEngine.findSprite(parentId);
			if (parent) parent->addChild(*this);
		} else if (id == SIZE_ATT) {
			mWidth = buf.read<float>();
			mHeight = buf.read<float>();
			mDepth = buf.read<float>();
			transformChanged = true;
		} else if (id == ROTATION_ATT) {
			mRotation.x = buf.read<float>();
			mRotation.y = buf.read<float>();
			mRotation.z = buf.read<float>();
			transformChanged = true;
		} else if (id == FLAGS_ATT) {
			mSpriteFlags = buf.read<int>();
			// This is being read here because I do not want to introduce a
			// new dirty state and the previous code already sets flag to false.
			// This is a no-op if it's the same shader.
			// NOTE: in a __thiscall function, usually order of argument execution
			// is from right to left. but I am just gonna play safe and copy the
			// strings once.
			auto loc = buf.read<std::string>();
			auto name = buf.read<std::string>();
			mSpriteShader.setShaders(loc, name);
		} else if (id == POSITION_ATT) {
			mPosition.x = buf.read<float>();
			mPosition.y = buf.read<float>();
			mPosition.z = buf.read<float>();
			transformChanged = true;
		} else if (id == CHECKBOUNDS_ATT) {
			bool checkBounds = buf.read<bool>();
			setCheckBounds(checkBounds);
		} else if (id == CENTER_ATT) {
			mCenter.x = buf.read<float>();
			mCenter.y = buf.read<float>();
			mCenter.z = buf.read<float>();
			transformChanged = true;
		} else if (id == SCALE_ATT) {
			mScale.x = buf.read<float>();
			mScale.y = buf.read<float>();
			mScale.z = buf.read<float>();
			transformChanged = true;
		} else if (id == COLOR_ATT) {
			mColor.r = buf.read<float>();
			mColor.g = buf.read<float>();
			mColor.b = buf.read<float>();
		} else if (id == OPACITY_ATT) {
			mOpacity = buf.read<float>();
		} else if (id == BLEND_ATT) {
			mBlendMode = buf.read<BlendMode>();
		} else if(id == CLIP_BOUNDS_ATT) {
			float x1 = buf.read<float>();
			float y1 = buf.read<float>();
			float x2 = buf.read<float>();
			float y2 = buf.read<float>();
			mClippingBounds.set(x1, y1, x2, y2);
			markClippingDirty();
		} else if(id == CORNERRADIUS_ATT){ 
			float cornerRad = buf.read<float>();
			mCornerRadius = cornerRad;
		} else if (id == SORTORDER_ATT) {
			int32_t						size = buf.read<int32_t>();
			// I'll assume anything beyond a certain size is a broken packet.
			if (size > 0 && size < 100000) {
				try {
					std::vector<sprite_id_t>	order;
					for (int32_t k=0; k<size; ++k) {
						order.push_back(buf.read<sprite_id_t>());
					}
					setSpriteOrder(order);
				} catch (std::exception const&) {
				}
			} 
		} else {
			readAttributeFrom(id, buf);
		}
	}

	if (transformChanged) {
		mUpdateTransform = true;
		mBoundsNeedChecking = true;
		dimensionalStateChanged();
	}
}

void Sprite::setSpriteId(const ds::sprite_id_t& id) {
	if (mId == id) return;

	if (mId != ds::EMPTY_SPRITE_ID) mEngine.unregisterSprite(*this);
	mId = id;
	if (mId != ds::EMPTY_SPRITE_ID) mEngine.registerSprite(*this);
	markAsDirty(ID_DIRTY);
}

void Sprite::setFlag(const int newBit, const bool on, const DirtyState& dirty, int& oldFlags) {
	int             newFlags = oldFlags;
	if(on) newFlags |= newBit;
	else newFlags &= ~newBit;
	if(newFlags == oldFlags) return;

	oldFlags = newFlags;
	markAsDirty(dirty);
}

bool Sprite::getFlag(const int bit, const int flags) const {
	return (flags&bit) != 0;
}

void Sprite::markAsDirty(const DirtyState& dirty){
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

void Sprite::markChildrenAsDirty(const DirtyState& dirty){
	mDirty |= dirty;
	for (auto it=mChildren.begin(), end=mChildren.end(); it != end; ++it) {
		(*it)->markChildrenAsDirty(dirty);
	}
}

void Sprite::setBlendMode(const BlendMode &blendMode){
	if(mBlendMode == blendMode)
		return;

	mBlendMode = blendMode;
	markAsDirty(BLEND_MODE);
}

BlendMode Sprite::getBlendMode() const {
	return mBlendMode;
}

void Sprite::setBaseShader(const std::string &location, const std::string &shadername, bool applyToChildren){
	mSpriteShader.setShaders(location, shadername);
	setFlag(SHADER_CHILDREN_F, applyToChildren, FLAGS_DIRTY, mSpriteFlags);

	if(applyToChildren) {
		for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
			(*it)->setBaseShader(location, shadername, applyToChildren);
		}
	}
}


//Setup for multi-pass shaders
void Sprite::setShaderList(const std::vector<std::pair<std::string, std::string>> shaderPair, bool applyToChildren /*= false*/)
{
	removeShaders();

	for (auto it = shaderPair.begin(); it != shaderPair.end(); ++it) {
		addNewShader(*it, false, applyToChildren);
	}

	setupIntermediateFrameBuffers();
}

void Sprite::removeShaders(){
	for (auto it = mSpriteShaders.begin(); it != mSpriteShaders.end(); ++it) {
		ds::ui::SpriteShader* ss = *it;
		delete ss;
	}
	mSpriteShaders.clear();
}

void Sprite::addNewShader(const std::string location, const std::string shaderName, bool addToFront /*= false*/, bool applyToChildren /*= false*/)
{
	std::pair<std::string, std::string> newShader;
	newShader.first = location;
	newShader.second = shaderName;
	addNewShader(newShader, addToFront, applyToChildren);
}

void Sprite::addNewShader(const std::pair<std::string, std::string> shaderPair, bool addToFront /*= false*/, bool applyToChildren /*= false*/)
{
	std::string location = shaderPair.first;
	std::string shadername = shaderPair.second;

	ds::ui::SpriteShader* spriteShader = new ds::ui::SpriteShader(location, shadername);
	if (!addToFront){
		mSpriteShaders.push_back(spriteShader);
	}
	else {
		mSpriteShaders.insert(mSpriteShaders.begin(), spriteShader);
	}

	setFlag(SHADER_CHILDREN_F, applyToChildren, FLAGS_DIRTY, mSpriteFlags);
	if (applyToChildren) {
		for (auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
			(*it)->addNewShader(shaderPair, applyToChildren);
		}
	}
}



//For GLSL programs in memory
void Sprite::addNewMemoryShader(const std::string& vert, const std::string& frag , std::string shaderName, bool addToFront /*= false*/, bool applyToChildren /*= false*/)
{
	
	ds::ui::SpriteShader* spriteShader = new ds::ui::SpriteShader(vert, frag, shaderName);
	if (!addToFront){
		mSpriteShaders.push_back(spriteShader);
	}
	else { 
		mSpriteShaders.insert(mSpriteShaders.begin(), spriteShader);
	}

	setFlag(SHADER_CHILDREN_F, applyToChildren, FLAGS_DIRTY, mSpriteFlags);
	if (applyToChildren) {
		for (auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
			(*it)->addNewMemoryShader(vert, frag, shaderName, addToFront, applyToChildren);
		}
	}
}



bool Sprite::removeShader(std::string shaderName)
{
	for (auto it = mSpriteShaders.begin(); it != mSpriteShaders.end(); ++it) {
		ds::ui::SpriteShader* ss = *it;
		if (!ss->getName().compare(shaderName.c_str())){
			mSpriteShaders.erase(std::remove(mSpriteShaders.begin(), mSpriteShaders.end(), ss), mSpriteShaders.end());
			return true;
		}
	}
	return false;
}

void Sprite::setShadersUniforms(std::string shaderName, ds::gl::Uniform uniforms){
	mUniforms[shaderName] = uniforms;
}


ds::gl::Uniform Sprite::getShaderUniforms(std::string shaderName) {
		std::map<std::string, ds::gl::Uniform>::iterator it;
		it = mUniforms.find(shaderName);
	if (it != mUniforms.end()){
		return it->second;
	}
	return ds::gl::Uniform();
}

void Sprite::setShaderExtraData(const ci::Vec4f& data){
	mShaderExtraData.set(data);
}

void Sprite::setFinalRenderToTexture(bool render_to_texture)
{
	if (render_to_texture == mIsRenderFinalToTexture) return;
	mIsRenderFinalToTexture = render_to_texture;

	setupFinalRenderBuffer();

}

bool Sprite::isFinalRenderToTexture()
{
	return mIsRenderFinalToTexture;
}

ci::gl::Texture* Sprite::getFinalOutTexture()
{
	if (mOutputFbo){
		//ci::Surface tmp(mOutputFbo->getTexture());
		//ci::writeImage("c:/videos/myTex.jpg", tmp); // "jpg");
		return &(mOutputFbo->getTexture());
	}
	else return nullptr;
}

bool Sprite::isShaderName(std::string name) const{
	if (mSpriteShaders[mShaderPass]->getName().compare(name) == 0){
		return true;
	}
	return false;
}

int Sprite::getShaderNumber(std::string name) const {
	int num = -1;
	int val = 0;
	if (!mSpriteShaders.empty()){
		for (auto it = mSpriteShaders.begin(); it != mSpriteShaders.end(); ++it) {
			bool match = (*it)->getName().compare(name) == 0;
			if (match) {
				num = val;
				break;
			}
			val++;
		}
	}
	return num;
}



ds::ui::SpriteShader* Sprite::getShaderFromListName(std::string name) const {
	ds::ui::SpriteShader* spriteShader = nullptr;

	if (!mSpriteShaders.empty()){
		for (auto it = mSpriteShaders.begin(); it != mSpriteShaders.end(); ++it) {
			bool match = (*it)->getName().compare(name) ==0;
			if (match) {
				spriteShader = *it;
				break;
			}
		}
	}
	return spriteShader;
}

std::string Sprite::getBaseShaderName() const {
	return mSpriteShader.getName();
}

bool Sprite::getUseShaderTexture() const {
	return mUseShaderTexture;
}

void Sprite::setUseShaderTexture( bool flag ) {
	mUseShaderTexture = flag;
}

void Sprite::setClipping( bool flag ) {
	setFlag(CLIP_F, flag, FLAGS_DIRTY, mSpriteFlags);
	markClippingDirty();
}

bool Sprite::getClipping() const {
	return getFlag(CLIP_F, mSpriteFlags);
}

const ci::Rectf& Sprite::getClippingBounds()
{
	if(mClippingBoundsDirty) {
		mClippingBoundsDirty = false;
		computeClippingBounds();
	}
	return mClippingBounds;
}

void Sprite::computeClippingBounds(){
	if(getClipping()) {
		float l = 0.0f, t = 0.0f;
		float r = mWidth;
		float b = mHeight;


		// first find the outermost clipped window and use it as our reference
		Sprite *outerClippedSprite = nullptr;
		Sprite *curSprite = this;
		while(curSprite) {
			if(curSprite->getClipping())
				outerClippedSprite = curSprite;
			curSprite = curSprite->mParent;
		}

		float old_l = mClippingBounds.getX1();
		float old_r = mClippingBounds.getX2();
		float old_t = mClippingBounds.getY1();
		float old_b = mClippingBounds.getY2();

		if(outerClippedSprite) {
			curSprite = mParent;
			while(curSprite) {
				if(curSprite->getClipping()) {
					float ww = curSprite->getWidth();
					float wh = curSprite->getHeight();

					ci::Vec3f tl, br;
					tl = globalToLocal(curSprite->localToGlobal(ci::Vec3f(0, 0, 0)));
					br = globalToLocal(curSprite->localToGlobal(ci::Vec3f(ww, wh, 0)));

					float wl = tl.x;
					float wt = tl.y;
					float wr = br.x;
					float wb = br.y;

					if(wl > l) l = wl;
					if(wr < r) r = wr;
					if(wt > t) t = wt;
					if(wb < b) b = wb;

					if(wl > r) r = wl + 1;
					if(wr < l) l = wr - 1;
					if(wt > b) b = wt + 1;
					if(wb < t) t = wb - 1;
				}
				curSprite = curSprite->mParent;
			}

			if(l == r) r += 1;
			if(t == b) b += 1;
		}

		if(!math::isEqual(old_l, l) || !math::isEqual(old_r, r) || !math::isEqual(old_t, t) || !math::isEqual(old_b, b)) {
			mClippingBounds.set(l, t, r, b);
			markAsDirty(CLIPPING_BOUNDS);
		}
	}
}

void Sprite::dimensionalStateChanged(){
	markClippingDirty();
	if (mLastWidth != mWidth || mLastHeight != mHeight) {
		mLastWidth = mWidth;
		mLastHeight = mHeight;
		onSizeChanged();
	}

	setupFinalRenderBuffer();
	setupIntermediateFrameBuffers();
}

void Sprite::setupIntermediateFrameBuffers(){
	if (mSpriteShaders.size() > 0){
		ci::gl::Fbo::Format format;
		format.setColorInternalFormat(GL_RGBA);
		format.enableColorBuffer(true, 1);
		format.enableDepthBuffer(false);

		if (getWidth() > 1.0f) {
			if (mFrameBuffer[0])
				delete mFrameBuffer[0];
			if (mFrameBuffer[1])
				delete mFrameBuffer[1];
			mFrameBuffer[0] = new ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), format);
			mFrameBuffer[1] = new ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), format);

		}
	}
}


void Sprite::setupFinalRenderBuffer(){
	if (mOutputFbo)
		delete mOutputFbo;

	if (mIsRenderFinalToTexture &&
		getWidth() > 1.0f &&
		getHeight() > 1.0f){
		ci::gl::Fbo::Format  format;
		mOutputFbo = new ci::gl::Fbo(static_cast<int>(mEngine.getSrcRect().getWidth()), static_cast<int>(mEngine.getSrcRect().getHeight()), format);
	}
	else mOutputFbo = nullptr;
}


void Sprite::markClippingDirty(){
	mClippingBoundsDirty = true;
	for(auto it = mChildren.begin(), end = mChildren.end(); it != end; ++it) {
		Sprite*     s = *it;
		if(s) s->markClippingDirty();
	}
}

void Sprite::makeSortedChildren() {
	mSortedTmp = mChildren;
	std::sort( mSortedTmp.begin(), mSortedTmp.end(), [](Sprite *i, Sprite *j) {
		return i->getPosition().z < j->getPosition().z;
	});
}

void Sprite::setSecondBeforeIdle( const double idleTime ) {
	mIdleTimer.setSecondBeforeIdle(idleTime);
}

double Sprite::secondsToIdle() const {
	return mIdleTimer.secondsToIdle();
}

bool Sprite::isIdling() const {
	return mIdleTimer.isIdling();
}

void Sprite::startIdling() {
	mIdleTimer.startIdling();
}

void Sprite::resetIdleTimer() {
	mIdleTimer.resetIdleTimer();
}

void Sprite::clearIdleTimer() {
	mIdleTimer.clear();
}

void Sprite::setNoReplicationOptimization(const bool on) {
	// This doesn't need to be replicated. Obviously.
	if (on) mSpriteFlags |= NO_REPLICATION_F;
	else mSpriteFlags &= ~NO_REPLICATION_F;
}

void Sprite::markTreeAsDirty() {
	markAsDirty(ds::BitMask::newFilled());
	markChildrenAsDirty(ds::BitMask::newFilled());
}

void Sprite::setRotateTouches(const bool on) {
	// This doesn't need to be replicated. Obviously.
	if (on) mSpriteFlags |= ROTATE_TOUCHES_F;
	else mSpriteFlags &= ~ROTATE_TOUCHES_F;
}

bool Sprite::isRotateTouches() const {
	return ((mSpriteFlags&ROTATE_TOUCHES_F) != 0);
}

void Sprite::userInputReceived() {
	if (mParent) {
		mParent->userInputReceived();
	}
	resetIdleTimer();
}

void Sprite::sendSpriteToFront(Sprite &sprite) {
	auto found = std::find(mChildren.begin(), mChildren.end(), &sprite);
	if (found == mChildren.end()) return;
	if (*found == mChildren.back()) return;

	mChildren.erase(found);
	mChildren.push_back(&sprite);

	markAsDirty(SORTORDER_DIRTY);
}

void Sprite::sendSpriteToBack(Sprite &sprite) {
	auto found = std::find(mChildren.begin(), mChildren.end(), &sprite);
	if (found == mChildren.end()) return;
	if (*found == mChildren.front()) return;

	mChildren.erase(found);
	mChildren.insert(mChildren.begin(), &sprite);

	markAsDirty(SORTORDER_DIRTY);
}

Sprite* Sprite::getFirstDescendantWithName(const std::wstring& name) {
	Sprite* output = nullptr;
	for(auto it = mChildren.begin(); it != mChildren.end(); it++) {
		if((*it)->getSpriteName() == name) {
			output = (*it);
			break;
		} else {
			output = (*it)->getFirstDescendantWithName(name);
			if(output != nullptr) {
				break;
			}
		}
	}
	return output;
}

void Sprite::sendToFront() {
	if (mParent) {
		mParent->sendSpriteToFront(*this);
	}
}

void Sprite::sendToBack() {
	if (mParent) {
		mParent->sendSpriteToBack(*this);
	}
}

void Sprite::setSpriteOrder(const std::vector<sprite_id_t> &order) {
	for (auto it=order.begin(), end=order.end(); it!=end; ++it) {
		const sprite_id_t	id(*it);
		auto found = std::find_if(mChildren.begin(), mChildren.end(), [id](Sprite *s)->bool { return s && s->getId() == id; });
		if (found != mChildren.end()) {
			Sprite*			s(*found);
			mChildren.erase(found);
			mChildren.push_back(s);
		}
	}
}

ds::ui::SpriteShader &Sprite::getBaseShader() {
	return mSpriteShader;
}

float Sprite::getScaleWidth() const {
	return mScale.x * getWidth();
}

float Sprite::getScaleHeight() const {
	return mScale.y * getHeight();
}

float Sprite::getScaleDepth() const {
	return mScale.z * mDepth;
}

void Sprite::setSwipeCallback( const std::function<void (Sprite *, const ci::Vec3f &)> &func ) {
	mSwipeCallback = func;
}

bool Sprite::hasTouches() const {
	return mTouchProcess.hasTouches();
}

void Sprite::passTouchToSprite( Sprite *destinationSprite, const TouchInfo &touchInfo ) {
	if (!destinationSprite || this == destinationSprite) return;

	// tell our current sprite we're through.
	TouchInfo newTouchInfo = touchInfo;
	newTouchInfo.mCurrentGlobalPoint = localToGlobal(ci::Vec3f(-10000.0f,-10000.0f, 0.0f));	// make sure we touch up outside the sprite area, so buttons don't think they're hit
	newTouchInfo.mPhase = TouchInfo::Removed;
	newTouchInfo.mPassedTouch = true;

	processTouchInfo(newTouchInfo);
	// switch to the new sprite
	mEngine.setSpriteForFinger(touchInfo.mFingerId, destinationSprite);
	newTouchInfo = touchInfo;
	newTouchInfo.mPhase = TouchInfo::Added; 
	destinationSprite->processTouchInfo(newTouchInfo);
}

void Sprite::postAppSetup() {
	mBlobType = BLOB_TYPE;
}

bool Sprite::getPerspective() const{
  return mPerspective;
}

void Sprite::setPerspective( const bool perspective ){
  mPerspective = perspective;

  for (auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
	(*it)->setPerspective(perspective);
  }
}

void Sprite::setIsInScreenCoordsHack(const bool b){
	mIsInScreenCoordsHack = b;
}

void Sprite::setUseDepthBuffer(bool useDepth)
{
	mUseDepthBuffer = useDepth;

	for(auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it) {
		(*it)->setUseDepthBuffer(mUseDepthBuffer);
	}
}

bool Sprite::getUseDepthBuffer() const{
	return mUseDepthBuffer;
}

void Sprite::setCornerRadius( const float newRadius ){
	mCornerRadius = newRadius;
	markAsDirty(CORNER_DIRTY);
}

float Sprite::getCornerRadius() const{
	return mCornerRadius;
}

void Sprite::setTouchScaleMode(bool doSizeScale){
	mTouchScaleSizeMode = doSizeScale;
}

void Sprite::readClientFrom(ds::DataBuffer& buf){
	while (buf.canRead<char>())
	{
		char cmd = buf.read<char>();
		if (cmd != TERMINATOR_CHAR) {
			readClientAttributeFrom(cmd, buf);
		}
		else
		{
			return;
		}
	}
}


ds::gl::Uniform& Sprite::getUniform(){
	return mUniform;
}

#ifdef _DEBUG
void Sprite::write(std::ostream &s, const size_t tab) const {
	writeState(s, tab);
	for (auto it=mChildren.begin(), end=mChildren.end(); it!=end; ++it) {
		Sprite*		child(*it);
		if (child) child->write(s, tab + 1);
	}
}

namespace {

void			write_matrix44f(const ci::Matrix44f &m, std::ostream &s) {
	for (int k=0; k<4; ++k) {
		auto row = m.getRow(k);
		if (k > 0) s << ", ";
		s << row;
	}
}

}

void Sprite::writeState(std::ostream &s, const size_t tab) const {
	for (size_t k=0; k<tab; ++k) s << "\t";
	s << "CLASS NAME=" << typeid(this).name() << "ID=" << mId << " flags=" << mSpriteFlags << " pos=" << mPosition << " size=[" << mWidth << "x" << mHeight << "x" << mDepth << "] scale=" << mScale << " cen=" << mCenter << " rot=" << mRotation << " clip=" << mClippingBounds << std::endl;
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE opacity=" << mOpacity << " use_shader=" << mUseShaderTexture << " use_depthbuffer=" << mUseDepthBuffer << " last_w=" << mLastWidth << " last_h=" << mLastHeight << std::endl;
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE need_bounds_check=" << mBoundsNeedChecking << " in_bounds=" << mInBounds << " check_bounds=" << mCheckBounds << " clip_dirty=" << mClippingBoundsDirty << " update_transform=" << mUpdateTransform << std::endl;
	// Transform
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE transform=";
	write_matrix44f(mTransformation, s);
	s << std::endl;
	// Inv transform
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE inv_tform=";
	write_matrix44f(mInverseTransform, s);
	s << std::endl;
	// Global transform
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE global_tx=";
	write_matrix44f(mGlobalTransform, s);
	s << std::endl;
	// Global inverse transform
	for (size_t k=0; k<tab+2; ++k) s << "\t";
	s << "STATE gl_inv_tx=";
	write_matrix44f(mInverseGlobalTransform, s);
	s << std::endl;
}

#endif

void Sprite::doPropagateVisibilityChange(bool before, bool after){
	if (before == after) return;

	for (auto it = mChildren.cbegin(), it2 = mChildren.cend(); it != it2; ++it)
	{
		if ((*it)->visible()) {
			// if we were visible, and now we're hidden and our child is visible...
			// our child will vanish.
			if (before && !after) {
				(*it)->onAppearanceChanged(false);
			}

			// if we were hidden, and now we're visible and our child is visible...
			// our child will show up.
			else if (!before && after) {
				(*it)->onAppearanceChanged(true);
			}

			// continue propagating
			(*it)->doPropagateVisibilityChange(before, after);
		}

		// DO NOT propagate this change to hidden children!
		// visibility change of a parent has no effect on hidden children.
	}
}

void Sprite::setDrawDebug(const bool doDebug){
	if(doDebug) mSpriteFlags |= DRAW_DEBUG_F;
	else mSpriteFlags &= ~DRAW_DEBUG_F;
}

bool Sprite::getDrawDebug(){
	return getFlag(DRAW_DEBUG_F, mSpriteFlags);
}


void Sprite::setSpriteName(const std::wstring& name){
	mSpriteName = name;
}

const std::wstring Sprite::getSpriteName(const bool useDefault) const {
	if(mSpriteName.empty() && useDefault){
		std::wstringstream wss;
		wss << getId();
		auto spriteName = wss.str();
		return spriteName;
	} else {
		return mSpriteName;
	}
}

/*
 * --------------------LockScale
 */
Sprite::LockScale::LockScale(Sprite& s, const ci::Vec3f& temporaryScale)
		: mSprite(s)
		, mScale(s.mScale) {
	mSprite.mScale = temporaryScale;

	mSprite.mUpdateTransform = true;
	mSprite.buildTransform();
	mSprite.computeClippingBounds();
}

Sprite::LockScale::~LockScale() {
	mSprite.mScale = mScale;

	mSprite.mUpdateTransform = true;
	mSprite.buildTransform();
	mSprite.computeClippingBounds();
}

} // namespace ui
} // namespace ds
