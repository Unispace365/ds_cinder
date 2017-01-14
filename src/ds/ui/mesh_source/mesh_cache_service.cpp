#include "stdafx.h"

#include "mesh_cache_service.h"

namespace ds {

namespace {
const std::string	_MESH_CACHE_SERVICE_NAME("ds:meshcache");
}

const std::string&	MESH_CACHE_SERVICE_NAME(_MESH_CACHE_SERVICE_NAME);

/**
* \class ds::MeshCacheService
*/
MeshCacheService::MeshCacheService() {
}

void MeshCacheService::start() {
}

ci::gl::VboMeshRef MeshCacheService::get(const std::string& key,
										 const std::function<ci::TriMesh(void)>& generate_fn) {
	std::unique_lock<std::mutex>	lock(mMutex);
	if(!mCache.empty()) {
		auto f = mCache.find(key);
		if(f != mCache.end()) return f->second;
	}
	if(!generate_fn) return nullptr;
	ci::gl::VboMeshRef		vbo = ci::gl::VboMesh::create(generate_fn());
	mCache[key] = vbo;
	return vbo;
}

} // namespace ds
