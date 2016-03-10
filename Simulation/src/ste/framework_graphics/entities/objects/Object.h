// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "Material.h"
#include "mesh.h"
#include "entity_signalling.h"

#include <memory>

namespace StE {
namespace Graphics {

class Object : public entity_signalling<Object*> {	
protected:
	int material_id;
	std::unique_ptr<mesh_generic> object_mesh;

public:
	Object(std::unique_ptr<mesh_generic> &&m) : entity_signalling(this), object_mesh(std::move(m)) {}

	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_material_id(int m) { material_id = m; }
	auto get_material_id() const { return material_id; }
};

}
}
