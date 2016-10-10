#include "mesh_sphere.h"

#include <sstream>

#include <ds/ui/sprite/sprite_engine.h>
#include "mesh_cache_service.h"

namespace ds {
namespace ui {

namespace {

ci::TriMesh createTriMesh( 
		std::vector<unsigned> &indices,
		const std::vector<cinder::vec3> &positions, 
		const std::vector<cinder::vec3> &normals,
		const std::vector<cinder::vec2> &texCoords )
{
	cinder::TriMesh mesh;
	if ( indices.size() > 0 ) {
		mesh.appendIndices( &indices[ 0 ], indices.size() );
	}
	if ( normals.size() > 0 ) {
		for ( auto iter = normals.begin(); iter != normals.end(); ++iter ) {
			mesh.appendNormal( *iter );
		}
	}
	if ( positions.size() > 0 ) {
		mesh.appendVertices( &positions[ 0 ], positions.size() );
	}
	if ( texCoords.size() > 0 ) {
		for ( auto iter = texCoords.begin(); iter != texCoords.end(); ++iter ) {
			mesh.appendTexCoord( *iter );
		}
	}

	return mesh;
}

ci::TriMesh createSphere(float radius, int x_res, int y_res) {
	std::vector<unsigned> indices;
	std::vector<cinder::vec3> normals;
	std::vector<cinder::vec3> positions;
	std::vector<cinder::vec2> texCoords;

	float step_phi = (float)M_PI / (float)y_res;
	float step_theta = ((float)M_PI * 2.0f) / (float)x_res;

	int p = 0;
	for ( float phi=0.0f; p<=y_res; p++, phi+=step_phi ) {
		int t = 0;

		unsigned a = (unsigned)( ( p + 0 ) * (x_res+1) );
		unsigned b = (unsigned)( ( p + 1 ) * (x_res+1) );

		for ( float theta=step_theta; t<=x_res; t++, theta+=step_theta ) {
			float sinPhi = cinder::math<float>::sin( phi );
			float x = radius * sinPhi * cinder::math<float>::cos( theta );
			float z = -radius * sinPhi * cinder::math<float>::sin( theta );
			float y = radius * cinder::math<float>::cos( phi );
			cinder::vec3 position( x, y, z );
			cinder::vec3 normal = position.normalized();
			cinder::vec2 texCoord = cinder::vec2( (float)t/(float)(x_res), (float)p/(float)y_res );

			normals.push_back( normal );
			positions.push_back( position );
			texCoords.push_back( texCoord ); 

			if ( t < x_res+1 ) {
				unsigned n = t + 1;
				indices.push_back( a + t );
				indices.push_back( b + t );
				indices.push_back( a + n );
				indices.push_back( a + n );
				indices.push_back( b + t );
				indices.push_back( b + n );
			}
		}
	}

	for ( auto iter=indices.begin(); iter!=indices.end(); ) {
		if ( *iter < positions.size() ) {
			++iter;
		} else {
			iter = indices.erase( iter );
		}
	}

	return createTriMesh(indices, positions, normals, texCoords);
}

} // anonymous namespace

class MeshSphere::SphereData : public MeshSource::Data {
public:
	SphereData(const float radius, const int x_res, const int y_res)
			: mEngine(nullptr)
			, mRadius(radius)
			, mXRes(x_res)
			, mYRes(y_res)
			, mMeshBuilt(false) {
	}

	virtual bool					isEqual(const Data& o) const {
		const SphereData*	ds = dynamic_cast<const SphereData*>(&o);
		if (!ds) return false;
		return mRadius == ds->mRadius && mXRes == ds->mXRes && mYRes == ds->mYRes;
	}

	virtual const ci::gl::VboMesh*	getMesh() {
		if (!mMeshBuilt) buildMesh();
		if (mMesh.getNumIndices() < 1) return nullptr;
		return &mMesh;
	}

	virtual void					setEngine(SpriteEngine* e) {
		mEngine = e;
	}

private:
	void							buildMesh() {
		mMeshBuilt = true;
		cacheMesh();
		if (!mMesh) {
			ci::TriMesh			mesh;
			mesh = createSphere(mRadius, mXRes, mYRes);
			mMesh = ci::gl::VboMesh(mesh);
		}
	}

	void							cacheMesh() {
		if (!mEngine) return;

		MeshCacheService&			s(mEngine->getService<ds::MeshCacheService>(ds::MESH_CACHE_SERVICE_NAME));
		std::stringstream			buf;
		// Currently getting a couple duplicates, should clip radius to some int value.
		buf << "sphere;r="<< mRadius << ";x=" << mXRes << ";y=" << mYRes;
		mMesh = s.get(buf.str(), [this]()->ci::TriMesh { ci::TriMesh t; t = createSphere(mRadius, mXRes, mYRes); return t;});
	}


	ds::ui::SpriteEngine*	mEngine;
	float					mRadius;
	int						mXRes, mYRes;
	// Building
	ci::gl::VboMesh			mMesh;
	bool					mMeshBuilt;
};

/**
 * \class ds::ui::MeshSphere
 */
MeshSphere::MeshSphere(const float radius, const int x_res, const int y_res)
		: MeshSource(std::shared_ptr<Data>(new SphereData(radius, x_res, y_res))) {
}

} // namespace ui
} // namespace ds
