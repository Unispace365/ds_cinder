#include "stdafx.h"

#include "image.h"

#include <map>

#include <cinder/ImageIo.h>

#include "ds/debug/logger.h"
#include "ds/app/blob_reader.h"
#include <ds/app/environment.h>
#include "ds/data/data_buffer.h"
#include "ds/app/blob_registry.h"
#include "ds/util/image_meta_data.h"
#include "ds/ui/sprite/sprite_engine.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE			= 0;

const DirtyState&	IMG_SRC_DIRTY		= INTERNAL_A_DIRTY;
const DirtyState&	IMG_CROP_DIRTY		= INTERNAL_B_DIRTY;

const char			IMG_SRC_ATT			= 80;
const char			IMG_CROP_ATT		= 81;
}

void Image::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Image::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Image>(r);});
}

Image& Image::makeImage(SpriteEngine& e, const std::string& fn, Sprite* parent) {
	return makeAlloc<ds::ui::Image>([&e, &fn]()->ds::ui::Image*{ return new ds::ui::Image(e, fn); }, parent);
}

Image& Image::makeImage(SpriteEngine& e, const ds::Resource& r, Sprite* parent) {
	return makeImage(e, r.getPortableFilePath(), parent);
}

Image::Image(SpriteEngine& engine)
	: inherited(engine)
	, ImageOwner(engine)
	, mStatusFn(nullptr)
	, mCircleCropped(false)
{
	mStatus.mCode = Status::STATUS_EMPTY;
	mDrawRect.mOrthoRect = ci::Rectf::zero();
	mDrawRect.mPerspRect = ci::Rectf::zero();
	mBlobType = BLOB_TYPE;

	setTransparent(false);
	setUseShaderTexture(true);

	markAsDirty(IMG_SRC_DIRTY);
	markAsDirty(IMG_CROP_DIRTY);
	
	mLayoutFixedAspect = true;
}

Image::Image(SpriteEngine& engine, const std::string& filename, const int flags)
	: Image(engine)
{
	setImageFile(filename, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags)
	: Image(engine)
{
	setImageResource(resourceId, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource& resource, const int flags)
	: Image(engine)
{
	setImageResource(resource, flags);
}

Image::~Image() { /* no-op */ }

void Image::updateServer(const UpdateParams& up)
{
	inherited::updateServer(up);
	checkStatus();
}

void Image::updateClient(const UpdateParams& up)
{
	inherited::updateClient(up);
	checkStatus();
}

void Image::drawLocalClient()
{
	if (!inBounds() || !isLoaded()) return;

	if (auto tex = mImageSource.getImage())
	{
		const ci::Rectf& useRect = (getPerspective() ? mDrawRect.mPerspRect : mDrawRect.mOrthoRect);

		tex->bind();
		if(mRenderBatch){
			mRenderBatch->draw();
		} else {
			ci::gl::drawSolidRect(useRect);
		}

		tex->unbind();

		// TODO
		/*
		
		// we're gonna do this ourselves so we can pass additional attributes to the shader
		ci::gl::SaveTextureBindState saveBindState( tex->getTarget() );
		ci::gl::BoolState saveEnabledState( tex->getTarget() );
		ci::gl::ClientBoolState vertexArrayState( GL_VERTEX_ARRAY );
		ci::gl::ClientBoolState texCoordArrayState( GL_TEXTURE_COORD_ARRAY );	
		tex->enableAndBind();

		glEnableClientState( GL_VERTEX_ARRAY );
		GLfloat verts[8];
		glVertexPointer( 2, GL_FLOAT, 0, verts );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		GLfloat texCoords[8];
		glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );

		verts[0*2+0] = useRect.getX2(); verts[0*2+1] = useRect.getY1();	
		verts[1*2+0] = useRect.getX1(); verts[1*2+1] = useRect.getY1();	
		verts[2*2+0] = useRect.getX2(); verts[2*2+1] = useRect.getY2();	
		verts[3*2+0] = useRect.getX1(); verts[3*2+1] = useRect.getY2();	

		const Rectf srcCoords = tex->getAreaTexCoords( tex->getCleanBounds() );
		texCoords[0*2+0] = srcCoords.getX2(); texCoords[0*2+1] = srcCoords.getY1();	
		texCoords[1*2+0] = srcCoords.getX1(); texCoords[1*2+1] = srcCoords.getY1();	
		texCoords[2*2+0] = srcCoords.getX2(); texCoords[2*2+1] = srcCoords.getY2();	
		texCoords[3*2+0] = srcCoords.getX1(); texCoords[3*2+1] = srcCoords.getY2();	

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

		if(usingExtent) {
			glDisableVertexAttribArray(extentLocation);
		}

		if(usingExtra) {
			glDisableVertexAttribArray(extraLocation);
		}
		*/
	}
}

void Image::setSizeAll( float width, float height, float depth )
{
	setScale( width / getWidth(), height / getHeight() );
}

bool Image::isLoaded() const
{
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Image::setCircleCrop(bool circleCrop)
{
	mCircleCropped = circleCrop;
	if(circleCrop){
		// switch to crop shader
		mSpriteShader.setShaders(Environment::getAppFolder("data/shaders"), "circle_crop");
	} else {
		// go back to base shader
		mSpriteShader.setShaders(Environment::getAppFolder("data/shaders"), "base");
	}
}

void Image::setCircleCropRect(const ci::Rectf& rect)
{
	markAsDirty(IMG_CROP_DIRTY);
	mShaderExtraData.x = rect.x1;
	mShaderExtraData.y = rect.y1;
	mShaderExtraData.z = rect.x2;
	mShaderExtraData.w = rect.y2;
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn)
{
	if(mEngine.getMode() != mEngine.STANDALONE_MODE){
		//DS_LOG_WARNING("Currently only works in Standalone mode, fill in the UDP callbacks if you want to use this otherwise");
		// TODO: fill in some callbacks? This actually kinda works. This will only not work in server-only mode. Everything else is fine
	}
	mStatusFn = fn;
}

bool Image::isLoadedPrimary() const
{
	return isLoaded();
}

void Image::onImageChanged()
{
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);
	doOnImageUnloaded();

	// Make my size match
	ImageMetaData		d;
	if (mImageSource.getMetaData(d) && !d.empty()) {
		Sprite::setSizeAll(d.mSize.x, d.mSize.y, mDepth);
	}
	else {
		// Metadata not found, reset all internal states
		ds::ui::Sprite::setSizeAll(0, 0, 1.0f);
		ds::ui::Sprite::setScale(1.0f, 1.0f, 1.0f);
		mDrawRect.mOrthoRect = ci::Rectf::zero();
		mDrawRect.mPerspRect = ci::Rectf::zero();
	}
}

void Image::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(IMG_SRC_DIRTY)) {
		buf.add(IMG_SRC_ATT);
		mImageSource.writeTo(buf);
	}

	if (mDirty.has(IMG_CROP_DIRTY)) {
		buf.add(IMG_CROP_ATT);
		buf.add(mShaderExtraData.x);
		buf.add(mShaderExtraData.y);
		buf.add(mShaderExtraData.z);
		buf.add(mShaderExtraData.w);
	}
}

void Image::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == IMG_SRC_ATT) {
		mImageSource.readFrom(buf);
		setStatus(Status::STATUS_EMPTY);
	} else if (attributeId == IMG_CROP_ATT) {
		mShaderExtraData.x = buf.read<float>();
		mShaderExtraData.y = buf.read<float>();
		mShaderExtraData.z = buf.read<float>();
		mShaderExtraData.w = buf.read<float>();
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void Image::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	if (mStatusFn) mStatusFn(mStatus);
}

void Image::checkStatus()
{
	if (mImageSource.getImage() && !isLoadedPrimary())
	{
		if (mEngine.getMode() == mEngine.CLIENT_MODE)
		{
			setStatus(Status::STATUS_LOADED);
			doOnImageLoaded();
		}
		else
		{
			auto tex = mImageSource.getImage();
			setStatus(Status::STATUS_LOADED);
			doOnImageLoaded();
			const float prevRealW = getWidth(), prevRealH = getHeight();
			if (prevRealW <= 0 || prevRealH <= 0) {
				Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
			}
			else {
				float prevWidth = prevRealW * getScale().x;
				float prevHeight = prevRealH * getScale().y;
				Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
				setSize(prevWidth, prevHeight);
			}
		}
	}
}

void Image::doOnImageLoaded()
{
	if (auto tex = mImageSource.getImage())
	{
		mDrawRect.mPerspRect = ci::Rectf(0.0f, static_cast<float>(tex->getHeight()), static_cast<float>(tex->getWidth()), 0.0f);
		mDrawRect.mOrthoRect = ci::Rectf(0.0f, 0.0f, static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()));
	}

	onImageLoaded();
}

void Image::doOnImageUnloaded()
{
	onImageUnloaded();
}

void Image::setSize( float width, float height ) {
	setSizeAll(width, height, mDepth);
}

} // namespace ui
} // namespace ds
