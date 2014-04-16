#pragma once
#ifndef DS_UI_MESHSOURCE_MESHFILE_H_
#define DS_UI_MESHSOURCE_MESHFILE_H_

#include "mesh_source.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::MeshFile
 * \brief Load a mesh from a file.
 */
class MeshFile : public MeshSource {
public:
	MeshFile(const std::string& filename);

private:
	MeshFile();
	class FileData;
};

} // namespace ui
} // namespace ds

#endif
