// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Material.h"
#include "mesh.h"

#include <memory>

namespace StE {
namespace Graphics {

class Object {
private:
	int material_id;
	std::unique_ptr<mesh_generic> object_mesh;

	glm::mat4 model_mat{ 1.f };
	bool dirty{ false };

public:
	Object(std::unique_ptr<mesh_generic> &&m) : object_mesh(std::move(m)) {}

	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_material_id(int m) {
		material_id = m;
		dirty = true;
	}
	void set_model_transform(const glm::mat4 &m) {
		model_mat = m;
		dirty = true;
	}
	const glm::mat4 &get_model_transform() const { return model_mat; }
	auto get_material_id() const { return material_id; }
	bool is_dirty() const { return dirty; }
	void clear_dirty() { dirty = false; }
};

}
}
