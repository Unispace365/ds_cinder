#include "stdafx.h"

#include "border.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

// TODO: move this to batches

using namespace ci;

namespace ds { namespace ui {

	namespace {
		char BLOB_TYPE = 0;

		const DirtyState& BORDER_WIDTH_DIRTY = INTERNAL_A_DIRTY;

		const char BORDER_WIDTH_ATT = 80;

		const ds::BitMask SPRITE_LOG = ds::Logger::newModule("border sprite");
	} // namespace

	void Border::installAsServer(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
	}

	void Border::installAsClient(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<Border>(r); });
	}

	Border::Border(SpriteEngine& engine)
	  : ds::ui::Sprite(engine)
	  , mBorderWidth(0.0f) {
		mBlobType = BLOB_TYPE;
		setTransparent(false);
		mSpriteShader.setToNoImageShader();
	}

	Border::Border(SpriteEngine& engine, const float width)
	  : ds::ui::Sprite(engine)
	  , mBorderWidth(width) {
		mBlobType = BLOB_TYPE;
		setTransparent(false);
		mNeedsBatchUpdate = true;
		markAsDirty(BORDER_WIDTH_DIRTY);
		mSpriteShader.setToNoImageShader();
	}

	void Border::setBorderWidth(const float borderWidth) {
		mBorderWidth	  = borderWidth;
		mNeedsBatchUpdate = true;
		markAsDirty(BORDER_WIDTH_DIRTY);
	}

	void Border::drawLocalClient() {
		const float renderWidth = mBorderWidth * (mEngine.getDstRect().getWidth() / mEngine.getSrcRect().getWidth());

		if (mCornerRadius < 1.0f && mRenderBatch && mLeftBatch && mRightBatch && mBotBatch) {

			mRenderBatch->draw();
			mLeftBatch->draw();
			mRightBatch->draw();
			mBotBatch->draw();

		} else if (mRenderBatch) {
			ci::gl::lineWidth(renderWidth);
			mRenderBatch->draw();

		} else if (mCornerRadius > 1.0f) {
			ci::gl::lineWidth(renderWidth);
			ci::gl::drawStrokedRoundedRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight), mCornerRadius);
		} else {
			ci::gl::drawStrokedRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight), renderWidth);
		}
	}

	void Border::writeAttributesTo(ds::DataBuffer& buf) {
		ds::ui::Sprite::writeAttributesTo(buf);

		if (mDirty.has(BORDER_WIDTH_DIRTY)) {
			buf.add(BORDER_WIDTH_ATT);
			buf.add(mBorderWidth);
		}
	}

	void Border::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
		if (attributeId == BORDER_WIDTH_ATT) {
			mBorderWidth	  = buf.read<float>();
			mNeedsBatchUpdate = true;
		} else {
			ds::ui::Sprite::readAttributeFrom(attributeId, buf);
		}
	}

	void Border::onBuildRenderBatch() {
		const float renderWidth = mBorderWidth * (mEngine.getDstRect().getWidth() / mEngine.getSrcRect().getWidth());

		const float w	 = getWidth();
		const float h	 = getHeight();
		auto		rect = ci::Rectf(0.0f, 0.0f, w, h);
		if (mCornerRadius > 0.0f) {
			auto theGeom = ci::geom::WireRoundedRect(rect, mCornerRadius);
			if (mRenderBatch) {
				mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
				//	mRenderBatch->getVboMesh().get()->getAvailableAttribs()
			} else {
				mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
			}
		} else {
			/*
			0				 w
			==================
			||              ||
			||              ||
			==================

			Border goes on the inside when there's no corner radius
			*/

			ci::Rectf topRect = ci::Rectf(0.0f, 0.0f, w, renderWidth);
			ci::Rectf botRect = ci::Rectf(0.0f, h - renderWidth, w, h);
			ci::Rectf lefRect = ci::Rectf(0.0f, renderWidth, renderWidth, h - renderWidth);
			ci::Rectf rigRect = ci::Rectf(w - renderWidth, renderWidth, w, h - renderWidth);

			auto topMesh = ci::gl::VboMesh::create(ci::geom::Rect(topRect));
			auto botMesh = ci::gl::VboMesh::create(ci::geom::Rect(botRect));
			auto lefMesh = ci::gl::VboMesh::create(ci::geom::Rect(lefRect));
			auto rigMesh = ci::gl::VboMesh::create(ci::geom::Rect(rigRect));

			if (mRenderBatch)
				mRenderBatch->replaceVboMesh(topMesh);
			else
				mRenderBatch = ci::gl::Batch::create(topMesh, mSpriteShader.getShader());

			if (mBotBatch)
				mBotBatch->replaceVboMesh(botMesh);
			else
				mBotBatch = ci::gl::Batch::create(botMesh, mSpriteShader.getShader());

			if (mLeftBatch)
				mLeftBatch->replaceVboMesh(lefMesh);
			else
				mLeftBatch = ci::gl::Batch::create(lefMesh, mSpriteShader.getShader());

			if (mRightBatch)
				mRightBatch->replaceVboMesh(rigMesh);
			else
				mRightBatch = ci::gl::Batch::create(rigMesh, mSpriteShader.getShader());
		}
	}


}} // namespace ds::ui
