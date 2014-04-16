#pragma once
#ifndef DS_MESH_H
#define DS_MESH_H

#include <string>
#include <cinder/gl/Vbo.h>
#include <cinder/gl/Light.h>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/util/mesh_file.h"
#include "ds/ui/image_source/image_owner.h"

namespace ds {
namespace ui {

class Mesh : public Sprite
	       , public ImageOwner
{
public:
	static Mesh&				makeMesh(SpriteEngine&, const std::string& filename, Sprite* parent = nullptr);
	static Mesh&				makeMesh(SpriteEngine&, const ds::Resource&, Sprite* parent = nullptr);

	Mesh(SpriteEngine&);
	Mesh(SpriteEngine&, const std::string& filename);
	Mesh(SpriteEngine&, cinder::gl::VboMesh mesh );
	~Mesh();

	void						setMeshFile(const std::string& filename);
	void						Mesh::onMeshChanged();
	virtual void				updateServer(const UpdateParams&);
	virtual void				drawLocalClient();
	bool						isLoaded() const;

	struct Status {
		static const int		STATUS_EMPTY = 0;
		static const int		STATUS_LOADED = 1;
		int						mCode;
	};
	void						setStatusCallback(const std::function<void(const Status&)>&);

protected:
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:
	typedef Sprite				inherited;

	void						setStatus(const int);
	void						init();

	Status						mStatus;
	bool						mStatusDirty;
	std::function<void(const Status&)>
								mStatusFn;

	cinder::gl::VboMesh			mVboMesh;
	ds::ui::util::MeshFile		mMeshFile;

	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif //DS_MESH_H
