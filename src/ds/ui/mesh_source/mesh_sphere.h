#pragma once
#ifndef DS_UI_MESHSOURCE_MESHSPHERE_H_
#define DS_UI_MESHSOURCE_MESHSPHERE_H_

#include "mesh_source.h"

namespace ds {
namespace ui {

/**
* \class ds::ui::MeshSphere
* \brief The public API for creating mesh geometry.
*/
class MeshSphere : public MeshSource {
public:
	MeshSphere(const float radius, const int x_res = 36, const int y_res = 12);

private:
	MeshSphere();
	class SphereData;
};

} // namespace ui
} // namespace ds

#endif
