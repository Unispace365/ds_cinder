#pragma once
#ifndef DS_UI_MESHSOURCE_MESHCACHESERVICE_H_
#define DS_UI_MESHSOURCE_MESHCACHESERVICE_H_

#include <unordered_map>
#include <string>
#include <cinder/gl/Vbo.h>
#include <cinder/Thread.h>
#include <cinder/TriMesh.h>
#include <ds/app/engine/engine_service.h>

namespace ds {

extern const std::string&	MESH_CACHE_SERVICE_NAME;

/**
 * \class ds::MeshCacheService
 * \brief Utility to cache mesh geometry.
 */
class MeshCacheService : public ds::EngineService {
public:
	MeshCacheService();

	virtual void			start();

	ci::gl::VboMesh			get(const std::string& key,
								const std::function<ci::TriMesh(void)>& generate_fn);

private:
	std::mutex				mMutex;
	std::unordered_map<std::string, ci::gl::VboMesh>
							mCache;
};

} // namespace ds

#endif
