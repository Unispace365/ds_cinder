#pragma once
#ifndef DS_UI_MESHSOURCE_MESHSOURCE_H_
#define DS_UI_MESHSOURCE_MESHSOURCE_H_

#include <memory>
#include <cinder/TriMesh.h>

namespace ds {
class DataBuffer;

namespace ui {

/**
 * \class ds::ui::MeshSource
 * \brief The public API for creating mesh geometry.
 */
class MeshSource {
public:
	MeshSource();
	virtual ~MeshSource();

	bool							operator==(const MeshSource&) const;
	bool							empty() const;

	const ci::TriMesh*				getMesh();

protected:
	class Data {
	public:
		Data();
		virtual ~Data();
		virtual bool				isEqual(const Data&) const = 0;
		virtual const ci::TriMesh*	getMesh() = 0;

		// Network replication
#if 0
		virtual char				getBlobType() const = 0;
		virtual void				writeTo(DataBuffer&) const = 0;
		virtual bool				readFrom(DataBuffer&) = 0;
#endif
	};
	MeshSource(const std::shared_ptr<Data>&);
	std::shared_ptr<Data>			mData;
};

} // namespace ui
} // namespace ds

#endif
