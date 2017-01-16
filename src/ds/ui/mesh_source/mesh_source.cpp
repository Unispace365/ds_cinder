#include "stdafx.h"

#include "mesh_source.h"

namespace ds {
namespace ui {

/**
* \class ds::ui::MeshSource
*/
MeshSource::MeshSource() {
}

MeshSource::MeshSource(const std::shared_ptr<Data>& d)
	: mData(d) {
}

MeshSource::~MeshSource() {
}

bool MeshSource::operator==(const MeshSource& o) const {
	if(!mData && !o.mData) return true;
	if(!mData) return false;
	if(!o.mData) return false;
	return mData->isEqual(*(o.mData.get()));
}

bool MeshSource::empty() const {
	return !mData;
}

const ci::gl::VboMeshRef MeshSource::getMesh() {
	if(!mData) return nullptr;
	return mData->getMesh();
}

void MeshSource::setEngine(SpriteEngine* e) {
	if(mData) mData->setEngine(e);
}

/**
* \class ds::ui::MeshSource::Data
*/
MeshSource::Data::Data() {
}

MeshSource::Data::~Data() {
}

void MeshSource::Data::setEngine(SpriteEngine*) {
}

} // namespace ui
} // namespace ds
