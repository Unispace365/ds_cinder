#pragma once
#ifndef DS_UI_MESHSOURCE_MESHSOURCE_H_
#define DS_UI_MESHSOURCE_MESHSOURCE_H_

#include <memory>
#include <cinder/gl/Vbo.h>
#include <cinder/gl/VboMesh.h>

namespace ds {
class DataBuffer;

namespace ui {
class SpriteEngine;

/**
* \class ds::ui::MeshSource
* \brief The public API for creating mesh geometry.
*/
class MeshSource {
public:
	MeshSource();
	virtual ~MeshSource();

	bool								operator==(const MeshSource&) const;
	bool								empty() const;

	const ci::gl::VboMeshRef			getMesh();

protected:
	// Internal maintenance
	void								setEngine(SpriteEngine*);

	friend class MeshOwner;
	class Data {
	public:
		Data();
		virtual ~Data();
		virtual bool					isEqual(const Data&) const = 0;
		virtual const ci::gl::VboMeshRef	getMesh() = 0;
		// If a subclass needs the engine, then handle this.
		virtual void					setEngine(SpriteEngine*);
		// Network replication
#if 0
		virtual char					getBlobType() const = 0;
		virtual void					writeTo(DataBuffer&) const = 0;
		virtual bool					readFrom(DataBuffer&) = 0;
#endif
	};
	MeshSource(const std::shared_ptr<Data>&);
	std::shared_ptr<Data>				mData;
};

} // namespace ui
} // namespace ds

#endif
