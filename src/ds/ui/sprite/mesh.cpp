#include "stdafx.h"

#include "mesh.h"

#include <map>

#include <cinder/gl/gl.h>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

#include <gl/GL.h>

using namespace ci;

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE = 0;

const DirtyState&	IMG_SRC_DIRTY = INTERNAL_A_DIRTY;
const DirtyState&	MESH_SRC_DIRTY = INTERNAL_B_DIRTY;

const char			IMG_SRC_ATT = 80;
const char			MESH_SRC_ATT = 81;

const ds::BitMask   SPRITE_LOG = ds::Logger::newModule("mesh sprite");
}

void Mesh::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void Mesh::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Mesh>(r); });
}

Mesh& Mesh::makeMesh(SpriteEngine& e, const std::string& fn, Sprite* parent) {
	return makeAlloc<ds::ui::Mesh>([&e, &fn]()->ds::ui::Mesh*{ return new ds::ui::Mesh(e, fn); }, parent);
}

Mesh& Mesh::makeMesh(SpriteEngine& e, const ds::Resource& r, Sprite* parent) {
	return makeMesh(e, r.getAbsoluteFilePath(), parent);
}

Mesh::Mesh(SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, ImageOwner(engine)
	, MeshOwner(engine) {
	init();
}

Mesh::Mesh(SpriteEngine& engine, const std::string& filename)
	: ds::ui::Sprite(engine)
	, ImageOwner(engine)
	, MeshOwner(engine) {
	init();
	setFileMesh(filename);
}

Mesh::~Mesh() {
}

void Mesh::onUpdateServer(const UpdateParams& up) {
	if(mStatusDirty) {
		mStatusDirty = false;
		if(mStatusFn) mStatusFn(mStatus);
	}
}

void Mesh::drawLocalClient() {
	if(!inBounds()) return;

	const ci::gl::TextureRef		tex = mImageSource.getImage();
	if(!tex) return;

	if(!mVboMesh) {
		const ci::gl::VboMeshRef	mesh = getMesh();
		if(!mesh) return;
		mVboMesh = mesh;
		if(!mVboMesh) return;
	}

	// Save the depth-read state...
	GLboolean oldDepthWriteMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthWriteMask);

	// Depth read/write is enabled in the root, but unfortunately necessary
	// right now because every sprite sets (or unsets) the depth buffer in its
	// draw, which I don't think is how that should be done (but maybe, what do
	// I know).  At least now we are safely restoring this state to whatever it
	// was before this draw call.
	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();

	tex->bind();
	ci::gl::draw(mVboMesh);
	tex->unbind();

	// Restore depth-write state.  GL_DEPTH_TEST (depth-read) is automagically
	// restored via Cinder's BoolState destructor
	glDepthMask(oldDepthWriteMask);
}

void Mesh::drawLocalServer() {
	drawLocalClient();
}

bool Mesh::isLoaded() const {
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Mesh::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.STANDALONE_MODE,
				  "Currently only works in Standalone mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void Mesh::onMeshChanged() {
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(MESH_SRC_DIRTY);
}

void Mesh::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if(mDirty.has(IMG_SRC_DIRTY)) {
		buf.add(IMG_SRC_ATT);
		mImageSource.writeTo(buf);
	}
	// TODO: Eric, want to implement some kind of MeshSource?
	//else if (mDirty.has(MESH_SRC_DIRTY)) {
	//buf.add(MESH_SRC_ATT);
	//mMeshSource.writeTo(buf);
	//}
}

void Mesh::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == IMG_SRC_ATT) {
		mImageSource.readFrom(buf);
		// TODO: Eric, want to implement some kind of MeshSource?
		//} else if (attributeId == MESH_SRC_ATT) {
		//mMeshSource.readFrom(buf);
	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

void Mesh::setStatus(const int code) {
	if(code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusDirty = true;
}

void Mesh::init() {
	mStatus.mCode = Status::STATUS_EMPTY;
	mStatusDirty = false;
	mStatusFn = nullptr;

	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTexture(true);
	markAsDirty(MESH_SRC_DIRTY);
}

} // namespace ui
} // namespace ds
