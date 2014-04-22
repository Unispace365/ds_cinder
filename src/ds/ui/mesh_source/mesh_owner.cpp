#include "mesh_owner.h"

#include "mesh_file.h"
#include "mesh_sphere.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::MeshOwner
 */
MeshOwner::MeshOwner(SpriteEngine& e)
		: mEngineForMesh(e) {
}

void MeshOwner::setMesh(const MeshSource& src) {
	if (mMeshSource == src) return;

	mMeshSource.setEngine(nullptr);
	mMeshSource = src;
	mMeshSource.setEngine(&mEngineForMesh);
	onMeshChanged();
}

void MeshOwner::clearMesh() {
	setMesh(MeshSource());
}

const ci::gl::VboMesh* MeshOwner::getMesh() {
	return mMeshSource.getMesh();
}

void MeshOwner::setSphereMesh(const float radius, const int x_res, const int y_res) {
	setMesh(MeshSphere(radius, x_res, y_res));
}

void MeshOwner::setFileMesh(const std::string& filename) {
	setMesh(MeshFile(filename));
}

void MeshOwner::onMeshChanged() {
}

} // namespace ui
} // namespace ds
